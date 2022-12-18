use std::sync::Arc;

use eframe::egui::plot::{Plot, Points};
use glam::*;
use parking_lot::RwLock;
use rand::Rng;
use rayon::prelude::*;

mod benchmark;

const G: SimFloat = 1.0; //6.674e-11;
const N: usize = 10000;
const TIMESTEP: SimFloat = 1e-7;
const SOFT: SimFloat = 1e-1;
const N_PER_GROUP: u32 = 64;

type SimFloat = f32;
type SimVec = Vec2;

struct SimParams {
    softening: SimFloat,
    timestep: SimFloat,
}

impl SimParams {
    fn as_array(&self) -> Vec<SimFloat> {
        vec![self.softening, self.timestep]
    }
}
struct Demo {
    points: Arc<RwLock<Vec<SimVec>>>,
    sim_params: Arc<RwLock<SimParams>>,
}

fn init_multi_threaded(
    sim_params: Arc<RwLock<SimParams>>,
    points: Arc<RwLock<Vec<SimVec>>>,
) -> std::thread::JoinHandle<()> {
    std::thread::spawn(move || {
        let mut rng = rand::thread_rng();
        let mut positions = Vec::new();
        let mut velocities = Vec::new();
        let mut masses = Vec::new();
        for _ in 0..N {
            masses.push(rng.gen_range::<SimFloat, _>(1e1..5e1));
            positions.push(2.0 * rng.gen::<SimVec>() - 1.0);
            velocities.push(2.0 * rng.gen::<SimVec>() - 1.0);
        }
        loop {
            let time_start = std::time::Instant::now();
            let SimParams {
                softening,
                timestep,
            } = *sim_params.read();

            let forces: Vec<SimVec> = positions
                .par_iter()
                .map(|&p1| {
                    positions
                        .iter()
                        .zip(&masses[..])
                        .map(|(&p2, &m2)| {
                            let r = p2 - p1;
                            m2 * r * (r.length() + softening).powi(-3)
                        })
                        .sum::<SimVec>()
                })
                .collect();

            positions
                .iter_mut()
                .zip(&mut velocities[..])
                .zip(&forces[..])
                .for_each(|((p, v), f)| {
                    *v += *f * G * timestep;
                    *p += *v * timestep;
                });

            let time_took = (std::time::Instant::now() - time_start).as_millis();
            if time_took > 10 {
                println!("Iteration took: {} ms", time_took);
            }
            *points.write() = positions.to_vec();
        }
    })
}

use std::{borrow::Cow, mem};
use wgpu::util::DeviceExt;

fn init_compute(
    sim_params: Arc<RwLock<SimParams>>,
    points: Arc<RwLock<Vec<SimVec>>>,
) -> std::thread::JoinHandle<()> {
    std::thread::spawn(move || {
        // WGPU init
        let backends = wgpu::Backends::all();
        let instance = wgpu::Instance::new(backends);
        let adapter = pollster::block_on(wgpu::util::initialize_adapter_from_env_or_default(
            &instance, backends, None,
        ))
        .unwrap();
        let (device, queue) = pollster::block_on(adapter.request_device(
            &wgpu::DeviceDescriptor {
                label: None,
                features: wgpu::Features::default() | wgpu::Features::MAPPABLE_PRIMARY_BUFFERS,
                limits: wgpu::Limits::default(),
            },
            None,
        ))
        .unwrap();

        // Compute pipeline init
        let compute_shader = device.create_shader_module(wgpu::ShaderModuleDescriptor {
            label: None,
            source: wgpu::ShaderSource::Wgsl(Cow::Borrowed(include_str!("compute.wgsl"))),
        });
        let sim_data = sim_params.read().as_array();
        let sim_param_buffer = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Simulation Parameter Buffer"),
            contents: bytemuck::cast_slice(&sim_data),
            usage: wgpu::BufferUsages::UNIFORM | wgpu::BufferUsages::COPY_DST,
        });
        let compute_bind_group_layout =
            device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
                entries: &[
                    wgpu::BindGroupLayoutEntry {
                        binding: 0,
                        visibility: wgpu::ShaderStages::COMPUTE,
                        ty: wgpu::BindingType::Buffer {
                            ty: wgpu::BufferBindingType::Uniform,
                            has_dynamic_offset: false,
                            min_binding_size: wgpu::BufferSize::new(
                                (sim_data.len() * mem::size_of::<SimFloat>()) as _,
                            ),
                        },
                        count: None,
                    },
                    wgpu::BindGroupLayoutEntry {
                        binding: 1,
                        visibility: wgpu::ShaderStages::COMPUTE,
                        ty: wgpu::BindingType::Buffer {
                            ty: wgpu::BufferBindingType::Storage { read_only: true },
                            has_dynamic_offset: false,
                            min_binding_size: wgpu::BufferSize::new(
                                (N * (std::mem::size_of::<SimFloat>() * 6)) as _,
                            ),
                        },
                        count: None,
                    },
                    wgpu::BindGroupLayoutEntry {
                        binding: 2,
                        visibility: wgpu::ShaderStages::COMPUTE,
                        ty: wgpu::BindingType::Buffer {
                            ty: wgpu::BufferBindingType::Storage { read_only: false },
                            has_dynamic_offset: false,
                            min_binding_size: wgpu::BufferSize::new(
                                (N * (std::mem::size_of::<SimFloat>() * 6)) as _,
                            ),
                        },
                        count: None,
                    },
                ],
                label: None,
            });
        let compute_pipeline_layout =
            device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
                label: Some("compute layout"),
                bind_group_layouts: &[&compute_bind_group_layout],
                push_constant_ranges: &[],
            });
        let compute_pipeline = device.create_compute_pipeline(&wgpu::ComputePipelineDescriptor {
            label: Some("Compute pipeline"),
            layout: Some(&compute_pipeline_layout),
            module: &compute_shader,
            entry_point: "main",
        });

        let mut rng = rand::thread_rng();
        let mut particles = Vec::new(); // Layout: [(mass, vel_x, vel_y, pos_x, pos_y), ..]
        for _ in 0..N {
            // mass
            particles.push(rng.gen_range::<SimFloat, _>(1.0..25.0));
            // padding
            particles.push(0.0);
            // vel
            particles.push(rng.gen_range::<SimFloat, _>(-1.0..1.0));
            particles.push(rng.gen_range::<SimFloat, _>(-1.0..1.0));
            // pos
            particles.push(rng.gen_range::<SimFloat, _>(-1.0..1.0));
            particles.push(rng.gen_range::<SimFloat, _>(-1.0..1.0));
        }

        let mut particle_buffers = Vec::<wgpu::Buffer>::new();
        let mut particle_bind_groups = Vec::<wgpu::BindGroup>::new();
        for i in 0..2 {
            particle_buffers.push(
                device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
                    label: Some(&format!("Particle Buffer {}", i)),
                    contents: bytemuck::cast_slice(&particles),
                    usage: wgpu::BufferUsages::MAP_READ
                        | wgpu::BufferUsages::STORAGE
                        | wgpu::BufferUsages::COPY_DST,
                }),
            );
        }

        for i in 0..2 {
            particle_bind_groups.push(device.create_bind_group(&wgpu::BindGroupDescriptor {
                layout: &compute_bind_group_layout,
                entries: &[
                    wgpu::BindGroupEntry {
                        binding: 0,
                        resource: sim_param_buffer.as_entire_binding(),
                    },
                    wgpu::BindGroupEntry {
                        binding: 1,
                        resource: particle_buffers[i].as_entire_binding(),
                    },
                    wgpu::BindGroupEntry {
                        binding: 2,
                        resource: particle_buffers[(i + 1) % 2].as_entire_binding(), // bind to opposite buffer
                    },
                ],
                label: None,
            }));
        }

        // Main compute loop
        let mut frame_num = 0;
        let work_group_count = (N as f64 / N_PER_GROUP as f64).ceil() as u32;
        loop {
            let time_start = std::time::Instant::now();
            let mut command_encoder =
                device.create_command_encoder(&wgpu::CommandEncoderDescriptor { label: None });
            {
                let mut cpass = command_encoder
                    .begin_compute_pass(&wgpu::ComputePassDescriptor { label: None });
                cpass.set_pipeline(&compute_pipeline);
                cpass.set_bind_group(0, &particle_bind_groups[frame_num % 2], &[]);
                cpass.dispatch_workgroups(work_group_count, 1, 1);
            }
            queue.write_buffer(
                &sim_param_buffer,
                0,
                bytemuck::cast_slice(&sim_params.read().as_array()),
            );
            queue.submit(Some(command_encoder.finish()));

            let data_buffer = &particle_buffers[(frame_num + 1) % 2];
            frame_num = frame_num.wrapping_add(1);

            data_buffer
                .slice(..)
                .map_async(wgpu::MapMode::Read, move |_| {});
            device.poll(wgpu::Maintain::Wait);
            let time_took = (std::time::Instant::now() - time_start).as_millis();
            if time_took > 10 {
                println!("Iteration took: {} ms", time_took);
            }

            {
                let buffer_view = data_buffer.slice(..).get_mapped_range();
                let _particles: &[SimFloat] = bytemuck::cast_slice(&buffer_view);
                let mut guard = points.write();
                guard.clear();
                guard.extend(
                    _particles
                        .chunks_exact(6)
                        .map(|chunk| SimVec::new(chunk[4], chunk[5])),
                );
            }
            data_buffer.unmap();
        }
    })
}

impl Default for Demo {
    fn default() -> Self {
        let points = Arc::new(RwLock::new(Vec::new()));
        let sim_params = Arc::new(RwLock::new(SimParams {
            softening: SOFT,
            timestep: TIMESTEP,
        }));
        // init_multi_threaded(sim_params.clone(), points.clone());
        init_compute(sim_params.clone(), points.clone());
        Self { points, sim_params }
    }
}
impl eframe::App for Demo {
    fn update(&mut self, ctx: &eframe::egui::Context, _frame: &mut eframe::Frame) {
        eframe::egui::TopBottomPanel::top("bar").show(ctx, |ui| {
            ui.horizontal(|ui| {
                ui.add(
                    eframe::egui::Slider::new(
                        &mut self.sim_params.write().softening,
                        SOFT * 0.1..=SOFT * 10.0,
                    )
                    .logarithmic(true)
                    .text("Softening"),
                );
                ui.add(
                    eframe::egui::Slider::new(
                        &mut self.sim_params.write().timestep,
                        TIMESTEP * 0.01..=TIMESTEP * 1000.0,
                    )
                    .logarithmic(true)
                    .text("Timestep"),
                );
            })
        });
        eframe::egui::CentralPanel::default().show(ctx, |ui| {
            let markers_plot = Plot::new("demo").data_aspect(1.0);
            markers_plot.show(ui, |plot_ui| {
                let points = self
                    .points
                    .read()
                    .iter()
                    .map(|p| [p.x as f64, p.y as f64])
                    .collect::<Vec<_>>();
                plot_ui.points(Points::new(points).radius(5.0));
            });
        });
        ctx.request_repaint();
    }
}

fn main() {
    eframe::run_native(
        "nbody",
        Default::default(),
        Box::new(|_| Box::new(Demo::default())),
    );
    // benchmark::bench()
}

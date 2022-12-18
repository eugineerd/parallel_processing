struct Particle {
  mass: f32,
  vel: vec2<f32>,
  pos: vec2<f32>,
};

struct SimParams {
    softening: f32,
    timestep: f32,
};

@group(0) @binding(0) var<uniform> params : SimParams;
@group(0) @binding(1) var<storage, read> particlesSrc : array<Particle>;
@group(0) @binding(2) var<storage, read_write> particlesDst : array<Particle>;

let G: f32 = 1.0;

@compute
@workgroup_size(64)
fn main(@builtin(global_invocation_id) global_invocation_id: vec3<u32>) {
    let total = arrayLength(&particlesSrc);
    let index = global_invocation_id.x;
    if index >= total {
        return;
    }

    var mass1: f32 = particlesSrc[index].mass;
    var pos1: vec2<f32> = particlesSrc[index].pos;
    var vel1: vec2<f32> = particlesSrc[index].vel;

    var acc = vec2(0.0, 0.0);
    for (var j: u32 = 0u; j < total; j += 1u) {
        var r = particlesSrc[j].pos - pos1;
        var r_len = length(r);
        var mass2 = particlesSrc[j].mass;
        acc += mass2 * r * pow(r_len * r_len + params.softening * params.softening, -1.5);
    }

    vel1 += G * acc * params.timestep;
    pos1 += vel1 * params.timestep;
    
    // Write back
    particlesDst[index] = Particle(mass1, vel1, pos1);
}
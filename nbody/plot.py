from subprocess import run, PIPE
import tqdm
import seaborn as sns
import matplotlib.pyplot as plt
import re

times = []
plt.rcParams["figure.figsize"] = (24,12)

fig, ax = plt.subplots(2, 3)
# proc = run([f"target/release/nbody"], stdout=PIPE)
# result = proc.stdout.decode("utf-8")
# lines = result.split("\n")
with open("results.txt") as f:
    lines=f.readlines()
N = []
single_times = []
multi_times = []
multi_speed = []
compute_times = []
compute_speed = []
for i in range(0, 9):
    N.append(int(re.search("\d+", lines[i*6+0]).group(0)))
    single_times.append(float(re.search("\d+(\.\d+)?", lines[i*6+1]).group(0)))
    multi_times.append(float(re.search("\d+(\.\d+)?", lines[i*6+2]).group(0)))
    multi_speed.append(float(re.search("\d+(\.\d+)?", lines[i*6+3]).group(0)))
    compute_times.append(float(re.search("\d+(\.\d+)?", lines[i*6+4]).group(0)))
    compute_speed.append(float(re.search("\d+(\.\d+)?", lines[i*6+5]).group(0)))

    # efficiency_ax = ax[2][size_n] 
    # efficiency_ax.set_title(f"Efficiency, {alg}, {m}x{m} matrix")
    # efficiency_ax.set_xlabel("num threads")
    # efficiency_ax.set_ylabel("times")
    # sns.lineplot(y=[x/y for x, y in zip(speedups, threads)], x=_threads, ax=efficiency_ax)

times_ax = ax[0][0] 
times_ax.set_title(f"Time, singlethreaded")
times_ax.set_xlabel("N")
times_ax.set_yscale("log")
times_ax.set_xscale("log")
times_ax.set_ylabel("time, ns")
sns.lineplot(y=single_times, x=N, ax=times_ax)

times_ax = ax[0][1] 
times_ax.set_title(f"Time, multithreaded")
times_ax.set_xlabel("N")
times_ax.set_yscale("log")
times_ax.set_xscale("log")
times_ax.set_ylabel("time, ns")
sns.lineplot(y=multi_times, x=N, ax=times_ax)

times_ax = ax[0][2] 
times_ax.set_title(f"Time, compute")
times_ax.set_xlabel("N")
times_ax.set_yscale("log")
times_ax.set_xscale("log")
times_ax.set_ylabel("time, ns")
sns.lineplot(y=compute_times, x=N, ax=times_ax)

times_ax = ax[1][1] 
times_ax.set_title(f"Speedup, multithreaded")
times_ax.set_xlabel("N")
times_ax.set_xscale("log")
times_ax.set_ylabel("times")
sns.lineplot(y=multi_speed, x=N, ax=times_ax)

times_ax = ax[1][2] 
times_ax.set_title(f"Speedup, compute")
times_ax.set_xlabel("N")
times_ax.set_xscale("log")
times_ax.set_ylabel("times")
sns.lineplot(y=compute_speed, x=N, ax=times_ax)

fig.savefig("results.png", bbox_inches='tight')
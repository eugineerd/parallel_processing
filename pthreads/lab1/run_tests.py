from subprocess import run, PIPE
import tqdm
import seaborn as sns
import matplotlib.pyplot as plt

times = []
sizes =[(4, 200), (10, 200), (20, 200), (50, 200), (100, 200), (250, 20), (1000, 3)]
threads = [1, 2, 4, 8, 16, 32]
plt.rcParams["figure.figsize"] = (40,24)

for l, alg in enumerate(["qr_decomposition", "row_mult", "col_mult", "block_mult"]):
    fig, ax = plt.subplots(3, len(sizes))
    for size_n, (m, r) in enumerate(sizes):
        times = []
        _threads = threads
        if alg == "block_mult":
            _threads = [1, 2, 4]
        for i, t in enumerate(_threads):
            s = 0
            out = f"{alg} {m}x{m} {t}"
            for j in tqdm.tqdm(range(r), desc=out):
                proc = run([f"lab1/{alg}", f"../lab1/data/{m}x{m}.in", "/dev/null", f"{t}"], stdout=PIPE)
                try:
                    result = proc.stdout.decode("utf-8")
                    s += int(result)
                except:
                    s += s/(j+1)
            time = s / r / 1000
            out += f": {time:.3f}ms\n"
            with open("results_.txt", "a+") as ff:
                ff.write(out)
            print(out, end="")
            times.append(time)
            
        times_ax = ax[0][size_n] 
        times_ax.set_title(f"Time, {alg}, {m}x{m} matrix")
        times_ax.set_xlabel("num threads")
        times_ax.set_yscale("log")
        times_ax.set_ylabel("time, ms")
        sns.lineplot(y=times, x=_threads, ax=times_ax)

        speedup_ax = ax[1][size_n] 
        speedup_ax.set_title(f"Speedup, {alg}, {m}x{m} matrix")
        speedup_ax.set_xlabel("num threads")
        speedup_ax.set_ylabel("times")
        speedups = [times[0]/x for x in times]
        sns.lineplot(y=speedups, x=_threads, ax=speedup_ax)

        efficiency_ax = ax[2][size_n] 
        efficiency_ax.set_title(f"Efficiency, {alg}, {m}x{m} matrix")
        efficiency_ax.set_xlabel("num threads")
        efficiency_ax.set_ylabel("times")
        sns.lineplot(y=[x/y for x, y in zip(speedups, threads)], x=_threads, ax=efficiency_ax)
    fig.savefig(f"{alg}.png", bbox_inches='tight')
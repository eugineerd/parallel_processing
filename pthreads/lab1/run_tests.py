from subprocess import run, PIPE
import tqdm
import seaborn as sns
import matplotlib.pyplot as plt

times = []
sizes =[(4, 200), (10, 200), (20, 200), (50, 200), (100, 200), (250, 20), (1000, 3)] 
threads = [1, 2, 4, 8, 16, 32]
plt.rcParams["figure.figsize"] = (30,50)
fig, ax = plt.subplots(3, len(sizes))

for l, alg in enumerate(["col_mult", "row_mult"]):
    for size_n, (m, r) in enumerate(sizes):
        times = []
        for i, t in enumerate(threads):
            s = 0
            out = f"{alg} {m}x{m} {t}"
            for j in tqdm.tqdm(range(r), desc=out):
                proc = run([f"lab1/{alg}", f"../lab1/data/{m}x{m}.in", "/dev/null", f"{t}"], stdout=PIPE)
                s += int(proc.stdout.decode("utf-8"))
            time = s / r / 1000
            out += f": {time:.3f}ms\n"
            with open("results_.txt", "a+") as ff:
                ff.write(out)
            print(out, end="")
            times.append(time)
        ax[l][size_n].set_title(f"{alg}, {m}x{m} matrix")
        ax[l][size_n].set_xlabel("num threads")
        ax[l][size_n].set_yscale("log")
        ax[l][size_n].set_ylabel("time, ms")
        sns.lineplot(y=times, x=[f"{x} threads" for x in threads], ax=ax[l][size_n])
fig.savefig("out.png", bbox_inches='tight')
from subprocess import run, PIPE
import tqdm
import seaborn as sns
import matplotlib.pyplot as plt

times = []
sizes =[(4, 1000), (10, 1000), (20, 1000), (50, 1000), (100, 500), (250,100), (1000, 25)] 
plt.rcParams["figure.figsize"] = (25,7)
fig, ax = plt.subplots(1, 4)

for i, t in enumerate([1, 2, 4, 8]):
    times = []
    for m, r in sizes:
        s = 0
        for j in tqdm.tqdm(range(r)):
            proc = run(["lab1/row_mult", f"../lab1/data/{m}x{m}.in", "/dev/null", f"{t}"], stdout=PIPE)
            s += int(proc.stdout.decode("utf-8"))
        time = s / r
        out = f"{m}x{m} {t}: {time}us\n"
        with open("results_.txt", "a+") as ff:
            ff.write(out)
        print(out, end="")
        times.append(time)
    ax[i].set_title(f"row_mult, {t} threads")
    ax[i].set_ylabel("time, us")
    ax[i].set_xlabel("matrix size")
    sns.lineplot(y=times, x=[f"{x[0]}x{x[0]}" for x in sizes], ax=ax[i])
fig.savefig("out.png")
from subprocess import run, PIPE
import tqdm
import seaborn as sns
import matplotlib.pyplot as plt

times = []
sizes =[(4, 1000), (10, 1000), (20, 1000), (50, 1000), (100, 500), (250,100), (1000, 10)] 
plt.rcParams["figure.figsize"] = (30,30)
fig, ax = plt.subplots(3, 4)

l = 0
alg = "row_mult"
for i, t in enumerate([1, 2, 4, 8]):
    times = []
    for m, r in sizes:
        s = 0
        for j in tqdm.tqdm(range(r)):
            proc = run([f"lab1/{alg}", f"../lab1/data/{m}x{m}.in", "/dev/null", f"{t}"], stdout=PIPE)
            s += int(proc.stdout.decode("utf-8"))
        time = s / r / 1000
        out = f"{alg} {m}x{m} {t}: {time:.3f}ms\n"
        with open("results_.txt", "a+") as ff:
            ff.write(out)
        print(out, end="")
        times.append(time)
    ax[l][i].set_title(f"{alg}, {t} threads")
    ax[l][i].set_xlabel("matrix size")
    ax[l][i].set_yscale("log")
    ax[l][i].set_ylabel("time, ms")
    sns.lineplot(y=times, x=[f"{x[0]}x{x[0]}" for x in sizes], ax=ax[l][i])

col_sizes = sizes[:5]
times = []
for m, r in col_sizes:
    s = 0
    f = open(f"../lab1/data/{m}x{m}.in")
    for j in tqdm.tqdm(range(r)):
        proc = run([f"lab1/col_mult"], stdout=PIPE, stdin=f)
        s += int(proc.stdout.decode("utf-8"))
    f.close()
    time = s / r / 1000
    print(f"col_mult: {time:.3f}ms")
    times.append(time)
ax[1][0].set_title(f"col_mult")
ax[1][0].set_xlabel("matrix size")
ax[1][0].set_yscale("log")
ax[1][0].set_ylabel("time, ms")
sns.lineplot(y=times, x=[f"{x[0]}x{x[0]}" for x in col_sizes], ax=ax[1][0])

fig.savefig("out.png", bbox_inches='tight')
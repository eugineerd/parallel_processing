import pathlib
import numpy as np

if not pathlib.Path(".").resolve().name == "build":
    print("must be run from build dir")
    exit(0)
    
data_dir = pathlib.Path("../lab1/data")
data_dir.mkdir(parents=True, exist_ok=True)

for i in [4, 10, 20, 50, 100, 250, 1000]:
    mat_a = np.random.rand(i, i)*np.random.randint(1, 10)
    mat_b = np.random.rand(i, i)*np.random.randint(1, 10)
    
    name = f"{i}x{i}"
    in_path = data_dir.joinpath(f"{name}.in")
    in_text = (
        f"{i} {i}\n" + "\n".join([" ".join(map(str,x)) for x in mat_a])+
        f"\n\n{i} {i}\n" + "\n".join([" ".join(map(str,x)) for x in mat_b])
    )
    in_path.write_text(in_text)

    out_path = data_dir.joinpath(f"{name}.out")
    out_text = "\n".join([" ".join(map(str,x)) for x in np.matmul(mat_a, mat_b)])
    out_path.write_text(out_text)

import math
import subprocess
from pathlib import Path

tests = [
    "tsp_51_1",
    "tsp_100_3",
    "tsp_200_2",
    "tsp_574_1",
    "tsp_1889_1",
    "tsp_33810_1",
]

root = Path(__file__).resolve().parent
failed = False

for name in tests:
    input_data = (root / "data" / name).read_text()
    data = input_data.split()
    n = int(data[0])
    points = list(zip(map(float, data[1::2]), map(float, data[2::2])))
    result = subprocess.run(
        [root / "a.out"],
        input=input_data,
        text=True,
        capture_output=True,
        timeout=60,
        check=True,
    )
    answer = result.stdout.split()
    if len(answer) != n + 1:
        print(f"{name}: FAILED")
        failed = True
        continue

    claimed_length = float(answer[0])
    tour = list(map(int, answer[1:]))
    if sorted(tour) != list(range(n)):
        print(f"{name}: FAILED")
        failed = True
        continue

    length = sum(math.dist(points[tour[i]], points[tour[(i + 1) % n]]) for i in range(n))
    correct = math.isfinite(claimed_length) and math.isclose(length, claimed_length, rel_tol=1e-9, abs_tol=1e-4)
    failed |= not correct
    status = "OK" if correct else "FAILED"
    print(f"{name}: {status} length={length:.6f}")

raise SystemExit(1 if failed else 0)

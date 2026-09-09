import subprocess
from pathlib import Path

tests = [
    "gc_50_3",
    "gc_70_7",
    "gc_100_5",
    "gc_250_9",
    "gc_500_1",
    "gc_1000_5",
]

root = Path(__file__).resolve().parent
failed = False

for name in tests:
    input_data = (root / "data" / name).read_text()
    data = list(map(int, input_data.split()))
    n, m = data[:2]
    edges = list(zip(data[2::2], data[3::2]))
    result = subprocess.run(
        [root / "a.out"],
        input=input_data,
        text=True,
        capture_output=True,
        timeout=60,
        check=True,
    )
    answer = list(map(int, result.stdout.split()))
    if len(answer) != n + 1:
        print(f"{name}: FAILED")
        failed = True
        continue

    count = answer[0]
    color = answer[1:]
    correct = all(0 <= c < count for c in color)
    correct &= len(set(color)) == count
    correct &= all(color[u] != color[v] for u, v in edges)
    failed |= not correct
    status = "OK" if correct else "FAILED"
    print(f"{name}: {status} colors={count}")

raise SystemExit(1 if failed else 0)

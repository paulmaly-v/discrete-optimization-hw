import subprocess
from pathlib import Path


tests = [
    "sc_157_0",
    "sc_330_0",
    "sc_1000_11",
    "sc_5000_1",
    "sc_10000_5",
    "sc_10000_2",
]

root = Path(__file__).resolve().parent

for name in tests:
    test = root / "data" / name
    input_data = test.read_text()
    lines = input_data.splitlines()
    n, m = map(int, lines[0].split())
    sets = [list(map(int, line.split())) for line in lines[1:]]

    result = subprocess.run(
        [root / "a.out"],
        input=input_data,
        text=True,
        capture_output=True,
        timeout=60,
        check=True,
    )
    answer = list(map(int, result.stdout.split()))

    if len(answer) != m + 1:
        print(f"{name}: FAILED")
        continue

    claimed_cost = answer[0]
    chosen = answer[1:]
    cost = sum(data[0] * take for data, take in zip(sets, chosen))
    covered = [False] * n
    for data, take in zip(sets, chosen):
        if take:
            for element in data[1:]:
                if element >= 0:
                    covered[element] = True

    correct = all(take in (0, 1) for take in chosen)
    correct &= all(covered) and cost == claimed_cost
    status = "OK" if correct else "FAILED"
    print(f"{name}: {status} cost={cost}")

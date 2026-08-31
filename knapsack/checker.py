import subprocess
from pathlib import Path


tests = [
    "ks_30_0",
    "ks_50_0",
    "ks_200_0",
    "ks_400_0",
    "ks_1000_0",
    "ks_10000_0",
]

root = Path(__file__).resolve().parent

for name in tests:
    test = root / "data" / name
    input_data = test.read_text()
    data = list(map(int, input_data.split()))
    n, capacity = data[:2]
    items = list(zip(data[2::2], data[3::2]))

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
        continue

    claimed_value = answer[0]
    chosen = answer[1:]
    value = sum(item_value * take for (item_value, _), take in zip(items, chosen))
    weight = sum(item_weight * take for (_, item_weight), take in zip(items, chosen))
    correct = all(take in (0, 1) for take in chosen)
    correct &= weight <= capacity and value == claimed_value

    status = "OK" if correct else "FAILED"
    print(f"{name}: {status} v={value} w={weight}/{capacity}")

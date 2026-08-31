#!/bin/bash

set -euo pipefail

input="${1:-data/ks_30_0}"

g++ -O2 -std=c++17 main.cpp -o a.out

echo
./a.out < "$input"

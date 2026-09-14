#!/bin/bash

set -euo pipefail

cd "$(dirname "$0")"
g++ -O2 -std=c++17 main.cpp -o a.out
python3 checker.py

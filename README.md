# Mahai
AI for playing mahjong

## Requirements
- C++23 compiler, most likely to work is gcc-13.2 on Ubuntu 24.04
- Python interpreter, most likely to work is python 3.12.3
- pybind-11 and range-v3, most likely to work, are from apt on Ubuntu 24.04

## Setup 
Compile Shared C++ python module
```bash
mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build . --target pymahai
```
Setup python virtual environment
```bash
python3 -m venv venv && ./venv/bin/pip install stable-baselines3[extra] matplotlib
```

## Documentation
Entrypoint is `main.py`
For more help, run
```bash
./venv/bin/python3 main.py --help
```
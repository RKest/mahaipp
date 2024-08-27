# Mahai
AI for playing mahjong - not well, but about 40 times better than random chance

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
python3 -m venv venv && ./venv/bin/pip install stable-baselines3[extra] matplotlib websockets
```

## Documentation
Entrypoint is `main.py`
For more help, run
```bash
./venv/bin/python3 main.py --help
```

Note: For the `-c play` option, the app also needs to be viewed with the associated index.html file in the browser.\
You can simply run a local server with `python3 -m http.server 8000` and open the browser at http://localhost:8000\
For the best experience, download to the root directory the following font https://github.com/kfarwell/Mahjong-Colored, as well as use browser that supports open type format features.\
I found firefox workd better than chrome.

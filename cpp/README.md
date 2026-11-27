# SOLCAT POLYGLOT TOOLS — C++ 版

同一套工具（计算器 / 单位换算 / 密码生成器 / 2048 逻辑）的现代 C++17 实现。

## 构建

```bash
cmake -S . -B build
cmake --build build          # 产物在 build/bin/tool
cp build/bin/tool bin/       # 可选：归档到 bin/
```

也可直接：

```bash
g++ -std=c++17 -O2 -o bin/tool main.cpp
```

Windows 下用 CMake + MinGW/VS 生成 `tool.exe`。

## 用法

```bash
./bin/tool calc "1+2*(3-4)"
./bin/tool convert 100 cm m
./bin/tool password 16
./bin/tool 2048 left
./bin/tool easter     # 彩蛋
```

## 特性（面向对象 + STL）

- `Calculator` 类：递归下降解析四则运算/括号/一元符号
- `Converter` 类：`std::map` 因子表，温度经摄氏度中转
- `PasswordGenerator` 类：`std::random_device` + `mt19937` + 均匀分布
- `Game2048` 类：`std::array` 棋盘，`move()` 支持四方向合并

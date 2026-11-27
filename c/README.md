# SOLCAT POLYGLOT TOOLS — C 版

同一套工具（计算器 / 单位换算 / 密码生成器 / 2048 逻辑）的 C99 实现，单文件。

## 构建

```bash
make            # gcc -O2 -std=c99 -o bin/tool main.c -lm
make windows    # 需 mingw-w64：生成 bin/tool.exe
```

Windows 下也可直接用 MinGW/Visual Studio 编译 `main.c` 为 EXE。

## 用法

```bash
./bin/tool calc "1+2*(3-4)"
./bin/tool convert 100 cm m
./bin/tool password 16
./bin/tool 2048 left
./bin/tool easter     # 彩蛋：ASCII 署名
```

## 特性

- 递归下降解析器：四则运算、括号、一元正负号
- 长度 / 重量 / 温度换算（温度经摄氏度中转）
- 密码基于 `rand()` + `time(NULL)` 种子（功能演示用，非加密安全）
- 4x4 棋盘 2048 移动/合并逻辑
- 仅标准库依赖，`-lm` 链接数学库

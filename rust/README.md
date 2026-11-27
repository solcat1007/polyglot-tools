# SOLCAT POLYGLOT TOOLS — Rust 版

同一套工具（计算器 / 单位换算 / 密码生成器 / 2048 逻辑）的 Rust 实现，仅用标准库。

## 构建

```bash
cargo build --release
cp target/release/tool bin/tool
```

交叉编译 Windows EXE（需安装 target）：

```bash
rustup target add x86_64-pc-windows-gnu
cargo build --release --target x86_64-pc-windows-gnu
# 产物：target/x86_64-pc-windows-gnu/release/tool.exe
```

## 用法

```bash
./bin/tool calc "1+2*(3-4)"
./bin/tool convert 100 cm m
./bin/tool password 16
./bin/tool 2048 left
./bin/tool easter     # 彩蛋
```

## 特性

- `Parser` 结构体递归下降解析四则运算/括号/一元符号
- `HashMap` 因子表换算，温度经摄氏度中转
- 纯标准库 xorshift 伪随机密码（无 `rand` 依赖；如需加密级别可换 `rand` crate）
- `[[i32;4];4]` 棋盘 + `move_board()` 四方向合并逻辑

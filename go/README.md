# SOLCAT POLYGLOT TOOLS — Go 版

同一套工具（计算器 / 单位换算 / 密码生成器 / 2048 逻辑）的 Go 实现。

## 构建

```bash
go build -o bin/tool .
```

交叉编译为 Windows EXE：

```bash
GOOS=windows GOARCH=amd64 go build -o bin/tool.exe .
```

## 用法

```bash
./bin/tool calc "1+2*(3-4)"
./bin/tool convert 100 cm m
./bin/tool password 16
./bin/tool 2048 left
./bin/tool easter     # 或 solcat：ASCII 署名彩蛋
```

## 特性

- 递归下降解析器实现四则运算 + 括号 + 一元正负号
- 长度 / 重量 / 温度换算（因子法，温度走摄氏度中转）
- `crypto/rand` 安全随机密码
- 独立 2048 移动/合并算法（left/right/up/down）
- 标准库 `flag` 解析子命令参数，无外部依赖

# SOLCAT POLYGLOT TOOLS — C# 版

同一套工具（计算器 / 单位换算 / 密码生成器 / 2048 逻辑）的 .NET 6 控制台实现。

## 构建

```bash
dotnet build -c Release -o bin/           # Linux/macOS：生成 bin/tool
```

生成 Windows 单文件 EXE：

```bash
dotnet publish -c Release -r win-x64 /p:PublishSingleFile=true --self-contained false -o bin/win
# 产物：bin/win/tool.exe
```

完全独立（含运行时，体积较大）：把 `--self-contained false` 改为 `true`。

> 注：首次 `publish -r win-x64` 需联网获取对应 RID 的运行时包；离线环境可退回到 `dotnet build` 产物，或在 Windows 机器上执行 publish。

## 用法

```bash
./bin/tool calc "1+2*(3-4)"
./bin/tool convert 100 cm m
./bin/tool password 16
./bin/tool 2048 left
./bin/tool easter     # 彩蛋
```

## 特性

- `Calculator` 类：递归下降解析器，`double` + `InvariantCulture`
- `Converter` 类：`Dictionary<string,double>` 因子表，温度经摄氏度中转
- `PasswordGenerator`：`RandomNumberGenerator.GetInt32` 加密安全随机
- `Game2048` 类：`int[,]` 棋盘 + 四方向合并
- 顶层 `Program.Main` 手动分派子命令

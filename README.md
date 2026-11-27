# SOLCAT POLYGLOT TOOLS — 同一套工具的多语言实现

本项目把**同一套核心工具**用多种编程语言各写了一遍：计算器（四则运算 + 括号表达式）、单位换算（长度/重量/温度）、密码生成器、2048 游戏逻辑。所有实现的算法逻辑保持一致，结果可互相对照。命令行版本均支持彩蛋参数 `easter`（或输入 `solcat`）输出 ASCII 署名 `SOLCAT POLYGLOT TOOLS`。

## 总览

| 语言 | 目录 | 构建命令 | 产物 | 特点 |
|------|------|----------|------|------|
| Go | `go/` | `go build -o bin/tool .`；Windows: `GOOS=windows GOARCH=amd64 go build -o bin/tool.exe .` | `bin/tool`, `bin/tool.exe` | 标准库 `flag` 解析，递归下降解析器，`crypto/rand` 密码 |
| C | `c/` | `make`（Windows: `make windows`，需 mingw-w64） | `bin/tool`（`bin/tool.exe`） | 单文件 C99，仅标准库，`-lm` |
| C++ | `cpp/` | `cmake -S . -B build && cmake --build build` | `build/bin/tool` → `bin/tool` | 现代 C++17，类 + STL，`Game2048` 类 |
| C# | `csharp/` | `dotnet build -c Release -o bin/`；单文件 EXE: `dotnet publish -c Release -r win-x64 /p:PublishSingleFile=true -o bin/win` | `bin/tool`, `bin/win/tool.exe` | .NET 6，`RandomNumberGenerator` 加密随机 |
| Rust | `rust/` | `cargo build --release && cp target/release/tool bin/` | `bin/tool` | 仅标准库，xorshift 伪随机，`Result` 错误处理 |
| Java | `java/` | `javac -d bin Tool.java` | `bin/*.class`, `bin/tool.jar` | JDK 标准库，`SecureRandom`，Java 8+ |
| Android | `android/` | Android Studio 打开并 Build APK，或 `gradlew assembleDebug` | `app/build/outputs/apk/debug/app-debug.apk` | Material 3 计算器 UI，完整 Gradle 项目骨架 |

## 统一命令行接口（CLI 六语言一致）

```bash
tool calc "1+2*(3-4)"          # -1
tool convert 100 cm m          # 1.000000 m
tool convert 212 f c           # 100.000000 c
tool password 16               # 随机密码
tool 2048 left                 # 4x4 棋盘左移合并
tool easter                    # ASCII 署名彩蛋（或 solcat）
```

Java 版运行方式为 `java -cp bin Tool <子命令>`。

## 算法一致性说明

- **calc**：递归下降解析 `expr → term → factor`，支持 `+ - * /`、括号、一元正负号；优先级和结合性一致。
- **convert**：长度/重量采用「基准单位 × 系数 / 目标系数」；温度先转摄氏度再转目标单位。
- **password**：字符集 `a-zA-Z0-9!@#$%^&*()-_=+`，长度默认 16；各语言用各自随机源（加密安全或时间种子）。
- **2048**：固定 4×4 测试棋盘，四方向移动时「去零 → 相邻相同合并一次 → 补零」。

## 编译验证状态

Go / C / C++ / C# / Rust / Java 六个 CLI 版本均已用对应工具链**实际编译通过并运行冒烟测试**，且 `calc`、`convert`、`2048` 的跨语言输出逐一对照完全一致。Android 项目提供完整源码与 Gradle 配置，全部 XML 已校验合法；当前环境未安装 Android SDK，`gradle assembleDebug` 因依赖下载超时未能产出 APK。

各目录的 `README.md` 含更详细的构建与交叉编译说明。

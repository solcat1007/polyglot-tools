# SOLCAT POLYGLOT TOOLS — Java 版

同一套工具（计算器 / 单位换算 / 密码生成器 / 2048 逻辑）的 Java 实现，仅用 JDK 标准库。

## 构建与运行

```bash
javac -d bin Tool.java
java -cp bin Tool calc "1+2*(3-4)"
```

生成可执行 JAR（可选）：

```bash
jar cfe bin/tool.jar Tool -C bin .
java -jar bin/tool.jar convert 100 cm m
```

## 用法

```bash
java -cp bin Tool calc "1+2*(3-4)"
java -cp bin Tool convert 100 cm m
java -cp bin Tool password 16
java -cp bin Tool 2048 left
java -cp bin Tool easter     # 彩蛋
```

## 特性

- 静态内部类 `Parser`：递归下降解析四则运算/括号/一元符号
- `HashMap` 因子表换算，温度经摄氏度中转
- `SecureRandom` 加密安全随机密码
- `move2048()` + `int[][]` 棋盘四方向合并逻辑
- Java 8+ 语法，javac 直接编译，无第三方依赖

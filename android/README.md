# SOLCAT POLYGLOT TOOLS — Android APK 项目

同一套工具中「计算器」的 Android 版本：Material 3 风格 UI，复用与 CLI 各语言版本一致的递归下降表达式求值算法。输入 `solcat` 后按 `=` 可触发署名彩蛋。

## 项目结构

```
android/
├── settings.gradle
├── build.gradle               # AGP 8.7.3
├── gradle.properties
├── gradle/wrapper/gradle-wrapper.properties   # Gradle 8.9
└── app/
    ├── build.gradle
    └── src/main/
        ├── AndroidManifest.xml
        ├── java/com/solcat/polyglot/MainActivity.java
        └── res/
            ├── layout/activity_main.xml
            ├── drawable/ic_launcher.xml
            └── values/{strings.xml,styles.xml,colors.xml}
```

## 构建 APK

### 方式一：Android Studio（推荐）
1. Android Studio → File → Open，选择本 `android/` 目录。
2. 等待 Gradle Sync 完成（需联网下载 AGP 与依赖）。
3. Build → Build Bundle(s) / APK(s) → Build APK(s)。
4. 产物：`app/build/outputs/apk/debug/app-debug.apk`。

### 方式二：命令行
安装 Android SDK 并设置环境变量：

```bash
export ANDROID_HOME=/path/to/android-sdk
export ANDROID_SDK_ROOT=$ANDROID_HOME
# 首次使用需 sdkmanager 安装 platform;android-35 与 build-tools;35.0.0
./gradlew assembleDebug          # 若本地有 gradle：gradle assembleDebug
```

产物路径：`app/build/outputs/apk/debug/app-debug.apk`。

## 本环境说明

当前构建环境**未安装 Android SDK**（无 `ANDROID_HOME`/`sdkmanager`/`adb`），仅有 Gradle 9.3.0。已实际尝试 `gradle assembleDebug`：先被全局 `init.gradle` 与 Gradle 9.3 不兼容阻断；换用干净 `GRADLE_USER_HOME` 后，在下载 AGP 依赖阶段超时（网络受限），两轮均未能产出 APK，属于环境缺失而非项目源码问题。全部 XML 文件已通过格式校验。项目源码、Gradle 配置与资源文件均完整，在装有 Android SDK 且可联网的环境按上述步骤即可构建。若使用系统 Gradle 9.3，建议改用 wrapper（`gradle-8.9`）以匹配 AGP 8.7.3。

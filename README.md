# Xbox Game Demo - 2D Coin Collection Game

一个基于 DirectX 12 和 Microsoft GDK 的 2D 游戏演示项目，展示了使用 CMake 和 vcpkg 构建 Xbox/Windows 游戏的完整工作流程。

## 🎮 游戏特性

- **2D 精灵渲染**：使用 DirectXTK12 的 SpriteBatch 进行高效 2D 渲染
- **Coin 收集系统**：10 个随机分布的 coin，带有上下浮动动画
- **碰撞检测**：使用平方距离优化，无需 sqrt 计算
- **游戏状态管理**：Title、Playing、Paused、Win 四种状态
- **输入支持**：支持手柄（GameInput）和键盘（WASD/方向键）
- **实时日志显示**：屏幕下半部分显示调试日志
- **HUD 信息**：显示 FPS、玩家位置、分数和剩余 coin 数

## 🎯 游戏玩法

1. **开始游戏**：按 `Space`/`Enter` 或手柄 `A` 键开始
2. **移动玩家**：使用 `WASD`/方向键或手柄左摇杆移动青色方块
3. **收集 Coins**：接近金色 coin（距离 < 30 像素）时自动收集
4. **完成目标**：收集所有 10 个 coin 即可获胜
5. **重新开始**：获胜后按 `A` 键或 `Space`/`Enter` 重新开始

## 🚀 快速开始

### 前置要求

- Windows 10/11 或 Xbox 开发环境
- Visual Studio 2022 (v17.6 或更高版本)
- CMake 3.21 或更高版本
- [Microsoft Game Runtime](https://aka.ms/GamingRepairTool)（如果未安装，运行修复工具）

### 构建项目

1. **克隆仓库**
   ```bash
   git clone https://github.com/dongjingyang/xboxgamedemo.git
   cd xboxgamedemo
   ```

2. **使用 Visual Studio**
   - 打开 Visual Studio 2022
   - 选择 "Open a local folder"
   - 选择项目根目录
   - Visual Studio 会自动检测 CMake 并配置项目

3. **使用命令行**
   ```bash
   cmake --preset x64-Debug
   cmake --build --preset x64-Debug
   ```

### 运行游戏

构建完成后，可执行文件位于：
- Debug: `out/build/x64-Debug/testfirst/testfirst.exe`
- Release: `out/build/x64-Release/testfirst/testfirst.exe`

## 📁 项目结构

```
testfirst/
├── Assets/              # 游戏资源
│   ├── arial.spritefont # 字体文件
│   └── *.png            # 图标资源
├── Game.cpp/h           # 游戏主逻辑（状态管理、coin 系统、渲染）
├── DeviceResources.cpp/h # Direct3D 12 设备资源管理
├── Main.cpp              # 程序入口和窗口消息处理
├── pch.cpp/h            # 预编译头文件
└── StepTimer.h           # 游戏计时器

External/
└── DirectXTK12/          # DirectX Tool Kit for DirectX 12（子模块）

CMakeLists.txt            # CMake 主配置文件
vcpkg.json                # vcpkg 依赖清单
vcpkg-configuration.json  # vcpkg 配置
```

## 🛠️ 技术栈

- **图形 API**: DirectX 12
- **游戏框架**: Microsoft GDK
- **渲染库**: DirectXTK12 (SpriteBatch, SpriteFont)
- **输入系统**: GameInput API
- **构建系统**: CMake + vcpkg
- **编程语言**: C++17

## 📦 依赖项

项目使用 vcpkg 管理依赖，主要包含：

- `ms-gdk` - Microsoft Game Development Kit
- `directxtk12` - DirectX Tool Kit for DirectX 12
- `directxmath` - DirectX Math 库
- `gameinput` - GameInput API
- `directx-dxc` - DirectX Shader Compiler
- `winpixevent` - PIX 事件支持

所有依赖在首次构建时由 vcpkg 自动下载和编译。

## 🎨 游戏实现细节

### Coin 系统

- **数据结构**：`vector<Coin>`，每个 coin 包含位置和存活状态
- **初始化**：进入 Playing 状态时随机生成 10 个 coin，避开屏幕边缘 50 像素
- **动画**：使用 `sin(time * 2.0) * 10.0` 实现上下浮动效果
- **碰撞检测**：使用平方距离比较（`dx² + dy² < 30²`），避免 sqrt 计算

### 游戏状态

- **Title**: 标题屏幕，显示 "Press A to Start"
- **Playing**: 游戏进行中，玩家可以移动和收集 coin
- **Paused**: 暂停状态，按 ESC 或 Menu 键暂停/继续
- **Win**: 胜利状态，收集完所有 coin 后显示 "You Win - Press A to Restart"

### 日志系统

- 所有 `OutputDebugStringA` 输出都会显示在屏幕下半部分
- 最多显示 20 行日志
- 线程安全的日志缓冲区
- 自动滚动显示最新日志

## 🔧 CMake + vcpkg 集成

本项目演示了完全独立的方法来使用 Microsoft GDK，无需安装任何额外工具。所有依赖通过 [vcpkg](https://aka.ms/vcpkg) 包管理器自动管理。

### vcpkg 配置

项目使用 vcpkg 的 "manifest mode"，依赖项在 `vcpkg.json` 中声明：

```json
{
  "dependencies": [
    "ms-gdk",
    "directxtk12",
    "directxmath",
    "gameinput",
    "directx-dxc",
    "winpixevent"
  ]
}
```

### 更新依赖版本

要更新到新版本的依赖，修改 `vcpkg-configuration.json` 中的 baseline：

```json
{
  "default-registry": {
    "kind": "builtin",
    "baseline": "最新提交哈希"
  }
}
```

### 使用静态库

要使用静态库而非 DLL，在 `CMakePresets.json` 中将 `VCPKG_TARGET_TRIPLET` 从 `x64-windows` 改为 `x64-windows-static-md`。

## 📝 字体文件生成

项目使用 `.spritefont` 格式的字体文件。如果字体文件缺失，可以使用提供的脚本生成：

```powershell
.\download_makespritefont.ps1
```

然后使用生成的 MakeSpriteFont 工具：

```bash
MakeSpriteFont.exe "Arial" testfirst\Assets\arial.spritefont /FontSize:32
```

详细说明请参考 `testfirst/Assets/README_FONT.md`。

## 📦 打包

使用 PowerShell 脚本创建打包布局：

```powershell
powershell -File PackageLayout.ps1 -Destination layout -Configuration Release
```

`PackageLayout.flt` 文件列出了要排除的文件模式（如 `.exp`、`.pdb`）。

## ⚠️ 已知问题

1. **路径长度限制**：如果项目路径过长（包含 vcpkg_installed 和 triplet 文件夹），可能超过 `_MAX_PATH`。建议将项目放在较浅的目录中。

2. **Game Runtime 未安装**：如果看到 "Game Runtime is not installed" 错误，运行 [Gaming Services Repair Tool](https://aka.ms/GamingRepairTool) 或使用 winget：
   ```bash
   winget install 9MWPM2CQNLHN -s msstore
   ```

3. **ms-gdk 包名变更**：从 2025 年 10 月起，ms-gdk 包名已更改。如果使用旧版本 GDK，需要更新 CMakeLists.txt 中的包名。

## 📚 进一步阅读

- [Microsoft GDK 文档](http://aka.ms/gdkdocs)
- [DirectX 12 Agility SDK](https://aka.ms/directx12agility)
- [DirectXTK12 Wiki](https://github.com/microsoft/DirectXTK12/wiki)
- [vcpkg for Xbox](https://learn.microsoft.com/vcpkg/users/platforms/xbox/)
- [GameInput API](http://aka.ms/gameinput)

## 📄 许可证

本项目基于 Microsoft GDK 模板项目，遵循相应的许可证条款。

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

---

**注意**：这是一个演示项目，展示了使用现代 C++ 和 DirectX 12 开发 Xbox/Windows 游戏的基础架构。可以作为学习 DirectX 12 和 Microsoft GDK 的起点。

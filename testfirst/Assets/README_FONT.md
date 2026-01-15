# 字体文件生成说明

## 方法 1: 使用 MakeSpriteFont 工具（推荐）

### 步骤 1: 下载 DirectX Tool Kit for DirectX 11

MakeSpriteFont 工具包含在 DirectX Tool Kit for DirectX 11 中。

**选项 A: 使用提供的 PowerShell 脚本**
```powershell
.\download_makespritefont.ps1
```

**选项 B: 手动下载**
1. 访问: https://github.com/microsoft/DirectXTK
2. 点击 "Code" -> "Download ZIP"
3. 解压文件

### 步骤 2: 编译 MakeSpriteFont

1. 打开 Visual Studio
2. 打开 `DirectXTK-main\MakeSpriteFont\MakeSpriteFont_Win10_Desktop_2019.sln`
3. 选择 Release 配置
4. 编译项目
5. 编译后的 exe 在: `DirectXTK-main\MakeSpriteFont\bin\Desktop_2019\Release\MakeSpriteFont.exe`

### 步骤 3: 生成字体文件

在命令提示符中运行：

```cmd
MakeSpriteFont.exe "Arial" testfirst\Assets\arial.spritefont /FontSize:32
```

或者使用完整路径：

```cmd
cd C:\Users\dongj\source\repos\testfirst
"路径\到\MakeSpriteFont.exe" "Arial" testfirst\Assets\arial.spritefont /FontSize:32
```

## 方法 2: 使用预编译的 MakeSpriteFont（如果可用）

某些 DirectX SDK 或工具包可能包含预编译的 MakeSpriteFont.exe。

## 注意事项

- 确保系统中已安装 Arial 字体（Windows 默认包含）
- `/FontSize:32` 可以调整为其他大小（如 16, 24, 48 等）
- 生成的文件应放在 `testfirst\Assets\` 目录下
- 文件名必须是 `arial.spritefont`（与代码中的路径匹配）

## 测试

生成字体文件后，重新编译并运行项目，应该能看到文本显示。

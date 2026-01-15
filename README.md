# Xbox Game Demo - 2D Snake Game

A DirectX 12 and Microsoft GDK based 2D snake game demonstration project showcasing a complete workflow for building Xbox/Windows games using CMake and vcpkg.

## 🎮 Gamepad Support

**Full GameInput API Support with Haptic Feedback**

This game features comprehensive gamepad support using the GameInput API:

- **Gamepad Input**: Full support for Xbox controllers and compatible gamepads
  - Left thumbstick for direction control (discrete 4-direction movement)
  - A button to start/resume/restart game
  - Menu (Start) button to pause/unpause
- **Haptic Feedback**: Rumble feedback when eating food
  - Triggers on food collection with configurable intensity
  - Automatic rumble management with timer-based stopping
  - Works with Xbox controllers and other GameInput-compatible devices
- **Keyboard Fallback**: WASD/Arrow keys for direction control, Space/Enter for actions, ESC for pause
- **Multi-input Support**: Seamlessly switches between gamepad and keyboard input

### Controller Requirements

- Xbox Wireless Controller (recommended)
- Any GameInput-compatible gamepad
- Windows 10/11 with GameInput API support
- Microsoft Game Runtime installed

## 🎮 Game Features

- **2D Sprite Rendering**: Efficient 2D rendering using DirectXTK12's SpriteBatch
- **Classic Snake Gameplay**: Discrete grid-based movement (20 pixels per cell, 10 cells/second)
- **Collision Detection**: Boundary collision and self-collision detection
- **Game State Management**: Title, Playing, Paused, GameOver, and Win states
- **Input Support**: Gamepad (GameInput) and keyboard (WASD/Arrow keys) with 4-direction control
- **Food System**: Single food item that respawns randomly when eaten (not on snake body)
- **Real-time Log Display**: Debug logs shown in the bottom half of the screen
- **HUD Information**: Displays FPS, score, and snake length

## 🎯 How to Play

1. **Start Game**: Press `Space`/`Enter` or gamepad `A` button to start
2. **Control Snake**: Use `WASD`/Arrow keys or left thumbstick to change direction
   - Snake moves automatically in discrete steps (10 cells per second)
   - Cannot make 180-degree turns (prevents instant death)
3. **Eat Food**: Guide the snake head (cyan) to the golden food square
   - **Haptic Feedback**: Feel the controller rumble when eating food!
   - Snake grows by one segment each time you eat
   - Food respawns at a random location (not on snake body)
4. **Avoid Collisions**: 
   - Don't hit the screen boundaries
   - Don't collide with your own body
5. **Game Over**: If you collide, the game ends. Press `A` button or `Space`/`Enter` to restart

## 🚀 Quick Start

### Prerequisites

- Windows 10/11 or Xbox development environment
- Visual Studio 2022 (v17.6 or later)
- CMake 3.21 or later
- [Microsoft Game Runtime](https://aka.ms/GamingRepairTool) (run repair tool if not installed)

### Building the Project

1. **Clone the repository**
   ```bash
   git clone https://github.com/dongjingyang/xboxgamedemo.git
   cd xboxgamedemo
   ```

2. **Using Visual Studio**
   - Open Visual Studio 2022
   - Select "Open a local folder"
   - Select the project root directory
   - Visual Studio will automatically detect CMake and configure the project

3. **Using Command Line**
   ```bash
   cmake --preset x64-Debug
   cmake --build --preset x64-Debug
   ```

### Running the Game

After building, the executable will be located at:
- Debug: `out/build/x64-Debug/testfirst/testfirst.exe`
- Release: `out/build/x64-Release/testfirst/testfirst.exe`

**Note**: Connect your Xbox controller before launching the game for the best experience with haptic feedback!

## 📁 Project Structure

```
testfirst/
├── Assets/              # Game resources
│   ├── arial.spritefont # Font file
│   └── *.png            # Icon resources
├── Game.cpp/h           # Main game logic (state management, coin system, rendering)
├── DeviceResources.cpp/h # Direct3D 12 device resource management
├── Main.cpp              # Program entry point and window message handling
├── pch.cpp/h            # Precompiled header files
└── StepTimer.h           # Game timer

External/
└── DirectXTK12/          # DirectX Tool Kit for DirectX 12 (submodule)

CMakeLists.txt            # Main CMake configuration file
vcpkg.json                # vcpkg dependency manifest
vcpkg-configuration.json  # vcpkg configuration
```

## 🛠️ Technology Stack

- **Graphics API**: DirectX 12
- **Game Framework**: Microsoft GDK
- **Rendering Library**: DirectXTK12 (SpriteBatch, SpriteFont)
- **Input System**: GameInput API (gamepad, keyboard, mouse)
- **Build System**: CMake + vcpkg
- **Programming Language**: C++17

## 📦 Dependencies

The project uses vcpkg to manage dependencies, primarily including:

- `ms-gdk` - Microsoft Game Development Kit
- `directxtk12` - DirectX Tool Kit for DirectX 12
- `directxmath` - DirectX Math library
- `gameinput` - GameInput API
- `directx-dxc` - DirectX Shader Compiler
- `winpixevent` - PIX event support

All dependencies are automatically downloaded and compiled by vcpkg on first build.

## 🎨 Game Implementation Details

### Snake Game System

- **Data Structure**: `deque<XMFLOAT2>` for snake body segments, each segment is grid-aligned
- **Initialization**: Snake starts with 3 segments (head + 2 body segments) in the center, facing right
- **Movement**: Discrete grid-based movement (20 pixels per cell, 0.10 seconds per step)
- **Direction Control**: 4-direction control (Up/Down/Left/Right) with 180-degree turn prevention
- **Food System**: Single food item that respawns at random grid positions (not on snake body)
- **Growth**: Snake grows by one segment each time food is eaten (tail is not removed)

### Collision Detection

- **Boundary Collision**: Checks if snake head is outside screen bounds (0 to width/height)
- **Self Collision**: Compares snake head position with all body segments (index 1 onwards)
- **Grid Alignment**: All positions are aligned to 20-pixel grid cells for precise collision

### Game States

- **Title**: Title screen, displays "Press A to Start"
- **Playing**: Game in progress, snake moves automatically, player controls direction
- **Paused**: Paused state, press ESC or Menu button to pause/resume (snake stops moving)
- **GameOver**: Game over state, displays "Game Over - Press A to Restart" when collision occurs
- **Win**: Victory state (kept for compatibility), displays "You Win - Press A to Restart"

### Haptic Feedback System

- **Trigger**: Activates when eating food during Playing state
- **Intensity**: Configurable low-frequency (0.6) and high-frequency (0.7) motors
- **Duration**: 0.08 seconds with automatic stop
- **Device Support**: Automatically detects and uses connected gamepad device
- **Fallback**: Gracefully handles cases where no gamepad is connected
- **State Management**: Automatically stops rumble when entering Paused or GameOver states

### Logging System

- All `OutputDebugStringA` output is displayed in the bottom half of the screen
- Maximum of 20 log lines displayed
- Thread-safe log buffer
- Automatic scrolling to show latest logs

## 🔧 CMake + vcpkg Integration

This project demonstrates a fully standalone method for using Microsoft GDK without installing any additional tools. All dependencies are automatically managed through the [vcpkg](https://aka.ms/vcpkg) package manager.

### vcpkg Configuration

The project uses vcpkg's "manifest mode", with dependencies declared in `vcpkg.json`:

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

### Updating Dependencies

To update to newer dependency versions, modify the baseline in `vcpkg-configuration.json`:

```json
{
  "default-registry": {
    "kind": "builtin",
    "baseline": "latest commit hash"
  }
}
```

### Using Static Libraries

To use static libraries instead of DLLs, change `VCPKG_TARGET_TRIPLET` in `CMakePresets.json` from `x64-windows` to `x64-windows-static-md`.

## 📝 Font File Generation

The project uses `.spritefont` format font files. If the font file is missing, you can generate it using the provided script:

```powershell
.\download_makespritefont.ps1
```

Then use the generated MakeSpriteFont tool:

```bash
MakeSpriteFont.exe "Arial" testfirst\Assets\arial.spritefont /FontSize:32
```

For detailed instructions, see `testfirst/Assets/README_FONT.md`.

## 📦 Packaging

Use the PowerShell script to create a package layout:

```powershell
powershell -File PackageLayout.ps1 -Destination layout -Configuration Release
```

The `PackageLayout.flt` file lists filename patterns to exclude (such as `.exp`, `.pdb`).

## ⚠️ Known Issues

1. **Path Length Limitation**: If the project path is too long (including vcpkg_installed and triplet folders), it may exceed `_MAX_PATH`. It is recommended to place the project in a shallower directory.

2. **Game Runtime Not Installed**: If you see "Game Runtime is not installed" error, run the [Gaming Services Repair Tool](https://aka.ms/GamingRepairTool) or use winget:
   ```bash
winget install 9MWPM2CQNLHN -s msstore
```

3. **ms-gdk Package Name Change**: As of October 2025, the ms-gdk package name has changed. If using older GDK versions, update the package name in CMakeLists.txt.

## 📚 Further Reading

- [Microsoft GDK Documentation](http://aka.ms/gdkdocs)
- [DirectX 12 Agility SDK](https://aka.ms/directx12agility)
- [DirectXTK12 Wiki](https://github.com/microsoft/DirectXTK12/wiki)
- [vcpkg for Xbox](https://learn.microsoft.com/vcpkg/users/platforms/xbox/)
- [GameInput API](http://aka.ms/gameinput)

## 📄 License

This project is based on the Microsoft GDK template project and follows the corresponding license terms.

## 🤝 Contributing

Issues and Pull Requests are welcome!

---

**Note**: This is a demonstration project showcasing the basic architecture for developing Xbox/Windows games using modern C++ and DirectX 12. It can serve as a starting point for learning DirectX 12 and Microsoft GDK.

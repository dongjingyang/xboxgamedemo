# Xbox Game Demo - 2D Snake Game

A DirectX 12 and **Microsoft GDK (Xbox Game Development Kit)** based 2D snake game demonstration project, showcasing a complete workflow for building Xbox/Windows games using modern C++, CMake, and vcpkg.

## 🎮 Core Features

### Xbox GDK Technology Stack

This project fully integrates the **Microsoft GDK** ecosystem:

- **Microsoft GDK** - Official Xbox Game Development Kit
- **DirectX 12** - Modern graphics API supporting Xbox Series X|S and Windows
- **DirectXTK12** - DirectX Tool Kit for DirectX 12, providing SpriteBatch, SpriteFont, and other 2D rendering components
- **GameInput API** - Unified input system supporting Xbox controllers, keyboard, and mouse
- **XAudio 2.9** - Audio engine support
- **vcpkg** - Dependency management with zero manual installation required

### Complete Gamepad Support

**Optimized for Xbox controllers with full gaming experience:**

- ✅ **Native Xbox Controller Support** - Using GameInput API, perfect support for Xbox Wireless Controller
- ✅ **Haptic Feedback (Rumble)** - Automatic controller vibration when eating food, enhancing gameplay
- ✅ **Seamless Multi-input Switching** - Supports simultaneous gamepad and keyboard use with automatic switching
- ✅ **Direction Control** - Left thumbstick or D-pad controls snake movement direction
- ✅ **Button Mapping** - A button for start/continue/restart, Menu button for pause/resume

### Game Features

- 🐍 **Classic Snake Gameplay** - Discrete grid-based movement, grow by eating food
- ✨ **Visual Effects** - Particle explosion effects and screen shake feedback
- 🎯 **Collision Detection** - Boundary and self-collision detection
- 📊 **Real-time HUD** - Displays FPS, score, and snake length
- 🎨 **Modern Rendering** - Efficient 2D rendering using DirectX 12 and SpriteBatch

## 🎮 Gamepad Support Details

### Full Xbox Controller Support

This project uses **GameInput API** to implement complete gamepad support, the modern input system recommended by Microsoft GDK:

**Input Features:**
- **Left Thumbstick** - Controls snake movement direction (4-direction: Up/Down/Left/Right)
- **A Button** - Start game, continue game, restart game
- **Menu Button** - Pause/resume game
- **D-pad** - Can also be used for direction control (same as thumbstick)

**Haptic Feedback:**
- 🎯 **Automatic Vibration on Food Collection** - Provides instant feedback, enhancing gameplay
- ⚙️ **Configurable Intensity** - Supports low-frequency and high-frequency vibration motors
- ⏱️ **Automatic Management** - Vibration stops automatically, won't get stuck

**Compatibility:**
- ✅ Xbox Wireless Controller (recommended)
- ✅ All GameInput API-compatible gamepads
- ✅ Windows 10/11 and Xbox Series X|S
- ✅ Microsoft Game Runtime required (usually pre-installed)

**Keyboard Support (Fallback):**
- `WASD` / Arrow keys - Control direction
- `Space` / `Enter` - Start/continue/restart
- `ESC` - Pause/resume

## 🎯 How to Play

1. **Start Game** - Press gamepad `A` button or keyboard `Space`/`Enter`
2. **Control Direction** - Use left thumbstick or D-pad/WASD to change snake direction
3. **Eat Food** - Guide the snake head (cyan square) to the golden food
   - 🎮 **Haptic Feedback** - Controller automatically vibrates when eating food
   - ✨ **Visual Effects** - Particle explosion and screen shake effects
   - 📈 **Growth** - Snake grows by one segment each time you eat
4. **Avoid Collisions** - Don't hit screen boundaries or your own body
5. **Pause Game** - Press `Menu` button or `ESC` to pause/resume
6. **Restart** - Press `A` button after game over to restart

## 🚀 Quick Start

### Prerequisites

- Windows 10/11 or Xbox development environment
- Visual Studio 2022 (v17.6 or later)
- CMake 3.21 or later
- [Microsoft Game Runtime](https://aka.ms/GamingRepairTool) (usually pre-installed, run repair tool if needed)

### Building the Project

**Method 1: Using Visual Studio (Recommended)**
1. Open Visual Studio 2022
2. Select "Open a local folder"
3. Select the project root directory
4. Visual Studio will automatically detect CMake and configure the project
5. Press `F5` to run

**Method 2: Using Command Line**
```bash
cmake --preset x64-Debug
cmake --build --preset x64-Debug
```

### Running the Game

After building, the executable will be located at:
- Debug: `out/build/x64-Debug/bin/testfirst.exe`
- Release: `out/build/x64-Release/bin/testfirst.exe`

**💡 Tip**: Connect your Xbox controller before launching the game for the full haptic feedback experience!

## 🛠️ Technology Stack

### Microsoft GDK Ecosystem

This project demonstrates the complete **Microsoft GDK** technology stack:

| Technology | Description | Purpose |
|------------|-------------|---------|
| **Microsoft GDK** | Xbox Game Development Kit | Core framework providing unified Xbox/Windows development experience |
| **DirectX 12** | Modern graphics API | High-performance 2D/3D rendering |
| **DirectXTK12** | DirectX Tool Kit | SpriteBatch, SpriteFont, and other 2D rendering components |
| **GameInput API** | Unified input system | Xbox controller, keyboard, and mouse input handling |
| **XAudio 2.9** | Audio engine | Game sound effects and background music support |
| **vcpkg** | C++ package manager | Automatic dependency management, no manual installation |

### Development Tools

- **CMake** - Cross-platform build system
- **C++17** - Modern C++ standard
- **Visual Studio 2022** - Recommended IDE

## 📦 Dependency Management

The project uses **vcpkg manifest mode** to automatically manage all dependencies:

**Core Dependencies:**
- `ms-gdk` - Microsoft Game Development Kit (Xbox GDK)
- `directxtk12` - DirectX Tool Kit for DirectX 12
- `gameinput` - GameInput API (gamepad input)
- `directxmath` - DirectX Math library
- `directx-dxc` - DirectX Shader Compiler
- `winpixevent` - PIX performance analysis support

**Advantages:**
- ✅ No manual installation of tools or SDKs required
- ✅ All dependencies automatically downloaded and compiled
- ✅ Version locking ensures build consistency
- ✅ Cross-platform support (Windows/Xbox)

## 📁 Project Structure

```
testfirst/
├── Game.cpp/h              # Main game class (device management, main loop, state orchestration)
├── SnakeGame.cpp/h         # Gameplay logic module (snake movement, collision, food system)
├── Effects2D.cpp/h         # Effects module (particle system, screen shake)
├── InputRouter.cpp/h       # Input handling module (GameInput wrapper)
├── DeviceResources.cpp/h   # DirectX 12 device resource management
├── Main.cpp                # Program entry point and window message handling
└── Assets/                 # Game resources (fonts, icons, etc.)

External/
└── DirectXTK12/           # DirectX Tool Kit for DirectX 12 (submodule)
```

### Architecture Design

The project uses a **modular architecture**, separating game logic, effects, and input handling:

- **SnakeGame** - Pure gameplay logic, independent of rendering system
- **Effects2D** - Independent effects system supporting particles and screen shake
- **InputRouter** - Unified input interface, simplifying input handling
- **Game** - Main controller responsible for resource management and module coordination

## 📚 Related Resources

- [Microsoft GDK Official Documentation](http://aka.ms/gdkdocs)
- [DirectX 12 Agility SDK](https://aka.ms/directx12agility)
- [DirectXTK12 Wiki](https://github.com/microsoft/DirectXTK12/wiki)
- [vcpkg for Xbox](https://learn.microsoft.com/vcpkg/users/platforms/xbox/)
- [GameInput API Documentation](http://aka.ms/gameinput)

## ⚠️ Common Issues

### Game Runtime Not Installed

If you see "Game Runtime is not installed" error:

1. Run the [Gaming Services Repair Tool](https://aka.ms/GamingRepairTool)
2. Or install using winget:
   ```bash
   winget install 9MWPM2CQNLHN -s msstore
   ```

### Path Too Long

If the project path is too long (including vcpkg_installed and triplet folders), it may exceed `_MAX_PATH` limit. It's recommended to place the project in a shallower directory.

### Font File Missing

If the font file is missing, you can generate it using the provided script:

```powershell
.\download_makespritefont.ps1
MakeSpriteFont.exe "Arial" testfirst\Assets\arial.spritefont /FontSize:32
```

For detailed instructions, see `testfirst/Assets/README_FONT.md`.

## 🎯 Project Highlights

### Why Choose This Project?

1. **Complete Xbox GDK Integration** - Demonstrates proper use of Microsoft GDK ecosystem
2. **Modern C++ Practices** - Uses C++17 features with modular architecture design
3. **Zero-Configuration Dependency Management** - Uses vcpkg manifest mode, no manual tool installation
4. **Complete Gamepad Support** - Full implementation of GameInput API + haptic feedback
5. **Extensible Architecture** - Clear module separation, easy to add new features

### Use Cases

- ✅ Learning Xbox GDK and DirectX 12 development
- ✅ Understanding modern game engine architecture design
- ✅ Starting point for Xbox/Windows game projects
- ✅ Researching GameInput API and haptic feedback implementation

---

## 📄 License

This project is based on the Microsoft GDK template project and follows the corresponding license terms.

## 🤝 Contributing

Issues and Pull Requests are welcome!

---
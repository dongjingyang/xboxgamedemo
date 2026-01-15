# Xbox GameCore Prototype – 2D Gameplay (GDK / DirectX 12)


A small but complete **2D game prototype** built with **Microsoft GDK** and **DirectX 12**, designed as a foundation for Xbox and Windows game development using modern C++.

This project focuses on **clean architecture**, **controller-first interaction**, and **real console feedback**, rather than being a minimal API sample.

---

## 🎮 Project Overview

This repository contains a fully playable **Snake-style game** implemented with a modular C++ architecture on top of **Microsoft GDK (GameCore)**.

It is intended to serve as:

* A **starter project** for Xbox / Windows games using GDK
* A **reference architecture** for small-to-mid scale gameplay prototypes
* A **validation project** for input, rendering, effects, and feedback loops on Xbox hardware

Despite its simplicity, the project follows real-world console development patterns.

---

## ✨ Gameplay Features

* Classic grid-based snake gameplay
* Snake grows when collecting food
* Game over on wall collision or self-collision
* Pause / resume / restart flow
* Stable fixed-step movement independent of framerate

---

## 🎮 Controller-First Experience

The game is designed around **Xbox controller usage**, not keyboard-first input.

### Input

* Left thumbstick or D-pad for movement
* `A` button to start / continue / restart
* `Menu` button to pause and resume
* Keyboard input is supported as a fallback

### Feedback

* **Controller rumble** when collecting food
* **Screen shake** to reinforce interaction
* **Particle burst** on successful collection

These feedback elements are intentionally subtle, matching typical console UX expectations.

---

## 🧱 Architecture Highlights

The project is structured around clear module boundaries, similar to real production codebases:

```
Game
├── SnakeGame        # Pure gameplay logic (movement, collision, scoring)
├── InputRouter      # Unified input abstraction (GameInput + keyboard)
├── Effects2D        # Visual feedback (particles, screen shake)
├── Renderer (2D)    # SpriteBatch-based scene rendering
└── Platform Layer   # GDK / DX12 device & lifecycle management
```

### Design Principles

* Gameplay logic is **rendering-agnostic**
* Effects are **event-driven**, not hard-coded into gameplay
* Input is **platform-abstracted**
* `Game.cpp` acts as an **orchestrator**, not a god object

This structure allows the project to scale beyond a simple demo without architectural rewrites.

---

## 🛠️ Technology & Platform

This project is built using the **official Xbox Game Development stack**:

* **Microsoft GDK (GameCore)**
* **DirectX 12**
* **DirectX Tool Kit for DX12**
* **GameInput API**
* **XAudio 2.9**
* **CMake + vcpkg (manifest mode)**

The same codebase runs on:

* Windows 10 / 11 (Gaming.Desktop)
* Xbox Series X|S (GameCore)

---

## 🚀 Build & Run

### Requirements

* Windows 10/11
* Visual Studio 2022
* CMake 3.21+
* Microsoft Game Runtime (usually preinstalled)

### Build (Visual Studio)

1. Open Visual Studio
2. Open the project folder (CMake-based)
3. Build and run (`F5`)

### Build (Command Line)

```bash
cmake --preset x64-Debug
cmake --build --preset x64-Debug
```

Executable output:

```
out/build/x64-Debug/bin/testfirst.exe
```

---

## 📁 Assets & Fonts

Text rendering uses a `.spritefont` generated with the DirectXTK `MakeSpriteFont` tool.

If the font file is missing:

```powershell
MakeSpriteFont.exe "Arial" Assets/arial.spritefont /FontSize:32
```

---

## 📌 Intended Use

This project is **not** meant to be a polished commercial title.

It is intended as:

* A **technical prototype**
* A **learning and validation project**
* A **starting point for further gameplay iteration**
* A **proof of platform integration** (input, feedback, rendering)

---

## 📚 References

* Microsoft GDK Documentation: [https://aka.ms/gdkdocs](https://aka.ms/gdkdocs)
* GameInput API: [https://aka.ms/gameinput](https://aka.ms/gameinput)
* DirectXTK12: [https://github.com/microsoft/DirectXTK12](https://github.com/microsoft/DirectXTK12)

---

## 📄 License

Based on the Microsoft GDK template and subject to its licensing terms.

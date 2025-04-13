# 🎮 Tetris in C++ — Terminal & OpenGL Versions

A fun and educational implementation of **Tetris** in C++, featuring two versions:

- 🖥️ **Terminal Version**: Classic text-based gameplay in the terminal.
- 🟦 **OpenGL Version**: Graphical version using OpenGL and GLFW for a modern twist.

---

## 📹 Demo Videos

### ▶️ Terminal Version

https://github.com/HarshK97/tetris/blob/main/video/tetris-terminal.mkv

<details>
  <summary>Click to view embedded</summary>

<video src="video/tetris-terminal.mkv" controls width="600"></video>

</details>

---

### ▶️ OpenGL Version

https://github.com/HarshK97/tetris/blob/main/video/tetris-opengl.mkv

<details>
  <summary>Click to view embedded</summary>

<video src="video/tetris-opengl.mkv" controls width="600"></video>

</details>

---

## 🧰 Technologies

| Terminal Version  | OpenGL Version           |
| ----------------- | ------------------------ |
| C++               | C++                      |
| Standard Library  | OpenGL                   |
| I/O & Logic       | GLFW for input & context |
| Grid + Loop Logic | Real-time rendering      |

---

## 🎯 Features

- Falling tetromino blocks
- Real-time movement with arrow keys
- Line clearing and scoring
- Collision and boundary detection
- Rotation (in OpenGL version)
- Game over detection

---

## 🕹 Controls

| Key        | Action              |
| ---------- | ------------------- |
| ← / →      | Move left / right   |
| ↓          | Move down faster    |
| ↑          | Rotate block        |
| Q / Ctrl+C | Quit (terminal ver) |

---

## 🛠️ Building and Running

### 🔹 Terminal Version

```bash
cd tetris-terminal
g++ main.cpp -o tetris_terminal
./tetris_terminal
```

### 🔹 OpenGL Version

```bash
cd tetris-opengl
g++ main.cpp -lglfw -lGL -o tetris_opengl
./tetris_opengl
```

## What I Learned

- Writing a game loop from scratch
- Handling real-time keyboard input
- Implementing grid-based logic
- Tetromino rotation and collision detection
- Using OpenGL for rendering
- Drawing shapes and colors
- Fixing segmentation faults using GDB

## Future Improvements

- Add a scoring UI overlay (OpenGL version)
- Implement a pause feature
- Include hold/next block preview
- Add sound effects
- Support for different game modes (e.g., timed, endless)
- Port the game to SDL or WebAssembly

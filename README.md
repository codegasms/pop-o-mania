# Pop-o-Mania

Pop-o-Mania is an engaging rhythm-based game where players can test their timing and reflexes to hit notes in sync with the beat of the music. The game supports custom beatmaps stored in `.osz` files, allowing for a dynamic and customizable experience.

## Features

- **Dynamic Beatmaps**: Load and play custom `.osz` beatmap files.
- **Intuitive Gameplay**: Simple and enjoyable rhythm mechanics.
- **Customizable Experience**: Play different songs and improve your skills.
- **Graphics and Audio**: Built with raylib for smooth rendering and immersive sound.

## Technologies Used

- **Programming Languages**: C, C++
- **Game Engine**: [raylib](https://www.raylib.com/) for rendering and input handling.
- **File Parsing**: Reads `.osz` files for beatmap data.

## Getting Started

### Prerequisites

Ensure you have the following installed on your system:

- GCC or any C/C++ compiler
- [raylib](https://www.raylib.com/) (Ensure raylib is installed and linked correctly.)
- Make (build tool)

### Installation

1. Clone the repository:

   ```bash
   git clone https://github.com/yourusername/pop-o-mania.git
   cd pop-o-mania
   ```

2. Build the project using the Makefile:

   ```bash
   make all
   ```

3. Run the game:

   ```bash
   ./pop-o-mania
   ```

## How to Play

1. Start the game and load a `.osz` file.
2. Match the falling notes with the beat by pressing the corresponding keys.
3. Score points for accurate hits and improve your timing!

## Directory Structure

```
Pop-o-Mania/
├── src/                # Source code files
├── assets/             # Assets like audio, textures, and sample .osz files
├── include/            # Header files
├── Makefile            # Build instructions
└── README.md           # Project documentation
```

## Contributing

We welcome contributions! Feel free to open an issue or submit a pull request.

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.

## Acknowledgments

- [raylib](https://www.raylib.com/) for simplifying game development.
- The open-source community for their support and resources.

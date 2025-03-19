# Maze Game

A 3D maze game using raycasting, similar to Wolfenstein 3D. Built with SDL2 and C.

## Features

- 3D raycasting engine
- Wall textures
- Floor and ceiling textures
- Enemy sprites
- Collision detection
- Minimap
- Weather effects (rain)
- Weapon display
- Smooth movement and rotation

## Controls

- W/S: Move forward/backward
- A/D: Strafe left/right
- Left/Right arrows: Rotate camera
- M: Toggle minimap
- R: Toggle rain effect
- ESC: Quit game

## Requirements

- SDL2 library
- GCC compiler
- Make

## Installation

1. Install SDL2:
```bash
sudo apt-get install libsdl2-dev
```

2. Clone the repository:
```bash
git clone https://github.com/yourusername/maze-game.git
cd maze-game
```

3. Compile the game:
```bash
make
```

## Running the Game

Run the game with a map file:
```bash
./maze_game maps/sample.map
```

## Map Format

The map is a text file where:
- '1' represents a wall
- '0' represents empty space
- '2' represents an enemy spawn point

## Project Structure

```
maze-game/
├── inc/
│   └── maze.h
├── src/
│   ├── main.c
│   ├── game.c
│   ├── raycast.c
│   ├── map.c
│   ├── texture.c
│   └── effects.c
├── maps/
│   └── sample.map
├── textures/
│   ├── wall.bmp
│   ├── floor.bmp
│   ├── ceiling.bmp
│   ├── weapon.bmp
│   └── enemy.bmp
├── Makefile
└── README.md
```

## License

This project is licensed under the MIT License - see the LICENSE file for details. 

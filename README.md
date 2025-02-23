# so_long 🎮 2D Game Development Project

## Project Description

### Core Purpose
Create a simple 2D game using **MinilibX** (42's graphics library) where a player collects items and exits a map. This project introduces graphical programming and event-driven application development.

### Key Technical Concepts
- **Graphics programming**: Window management and image rendering
- **Map parsing**: Validate and interpret .ber map files
- **Game mechanics**: Collision detection, item collection, and win/lose conditions
- **Memory management**: Handling textures and sprites

### Why This Project Matters
So_long serves as 42 students' first foray into:
- Graphical application development
- Real-time input handling
- Game development fundamentals
- Complex system integration (graphics + logic + IO)

---

## Learning Objectives 🎯

### Key Programming Concepts
1. **MinilibX Mastery**: Window/Image management
2. **Game Loop Design**: Continuous rendering + event handling
3. **Map Validation**: 
   ```c
   /* Example map validation check */
   if (map->player_count != 1 || map->exit_count != 1)
       error_exit("Invalid map configuration");
    ```
4. **State Management**: Tracking collectibles and player position

### Skills Developed
* Working with external graphical libraries
* Event-driven programming (key hooks, window events)
* Pixel-perfect rendering techniques
* Debugging graphical applications

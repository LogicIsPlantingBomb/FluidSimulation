# Water Simulation Project.

This project is an initial phase of a simple water simulation using SDL2. The program allows users to interactively place water and solid cells on a grid-based environment, laying the foundation for a physics-based fluid simulation.

## Features
- Grid-based rendering using SDL2
- Interactive cell placement (water and solid)
- Toggle between water and solid using the **Space** key
- Simple rendering with color-coded cells
- Basic event handling for mouse movements and keyboard input

## Prerequisites
Make sure you have the following installed on your system:
- **SDL2**
- **GCC** or any C compiler
- **Make** (optional, for build automation)

## Installation and Compilation
1. Clone the repository:
   ```sh
   git clone <repository-url>
   cd <repository-folder>
   ```
2. Install SDL2 (if not already installed):
   ```sh
   sudo pacman -S sdl2   # For Manjaro/Arch-based systems
   ```
3. Compile the program:
   ```sh
   gcc -o water_simulation water_simulation.c -lSDL2
   ```
4. Run the program:
   ```sh
   ./water_simulation
   ```

## Usage
- Click and drag the mouse to place cells on the grid.
- Press **Space** to toggle between **solid** and **water** cell types.
- Close the window to exit the program.

## Future Improvements
- Implement physics-based water flow simulation
- Improve rendering efficiency
- Add user interface elements for better interaction
- Introduce more types of materials and obstacles

## License
This project is open-source. Feel free to modify and distribute as needed.

## Contributions
Contributions are welcome! Feel free to open an issue or submit a pull request to improve the project.

## Author
**Dhruv Bhardwaj**



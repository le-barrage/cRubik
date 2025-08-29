# RubiksCube

Sources:

- https://www.raylib.com/examples.html
- https://www.raylib.com/cheatsheet/cheatsheet.html

- https://www.dil.univ-mrs.fr/~regis/c-sys/index.html

- https://en.wikipedia.org/wiki/Spherical_coordinate_system
- https://ruwix.com/the-rubiks-cube/notation/
- https://www.javatpoint.com/rotate-matrix-by-90-degrees-in-java
- https://www.worldcubeassociation.org/regulations/#article-4-scrambling
- https://en.cppreference.com/w/c/io/fprintf
- https://en.cppreference.com/w/c/string/byte/strcpy
- https://en.cppreference.com/w/c/experimental/dynamic/strdup
- https://www.speedsolving.com/wiki/index.php/Scrambling

- https://github.com/hkociemba/CubeExplorer
- https://github.com/hishamcse/Rubiks-Cube-Solver

## Current functionalities:

- Cube visualization in 3D (in any NxNxN size):
  - Cube outer layers can be rotated using the corresponding keys;
  - Cube can be reset to its original solved state.
- Scramble generations:
  - Works with every cube size.
- Timer.
- Kociemba's algorithm (usually finds a solution in 20-22 moves only 3x3x3).
- Cube rotation animation.

## Usage

Because we are using Raylib and RayGui, make sure the following libraries are installed:

```
libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev
```

Run the following commands to compile and run the project:

```bash
./build.sh && ./rubiks
```

or

```bash
make && ./rubiks
```

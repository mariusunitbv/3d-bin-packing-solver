# 3D Bin Packing Solver

A university project focused on solving the 3D Bin Packing Problem using multiple heuristic and metaheuristic algorithms, with real-time 3D visualization and performance benchmarking.

The application allows loading custom packing problems, testing different solving strategies, and visualizing how boxes are placed inside a 3D container.

Built with modern C++ modules, Raylib, and ImGui.

---

## Features

- Multiple 3D bin packing algorithms
- Interactive 3D visualization
- Real-time statistics and benchmarking
- Support for box rotations/orientations
- Save/load problem configurations
- Genetic Algorithm and Hill Climbing implementations
- Modular architecture using C++20 modules
- Cross-platform support (desktop + Emscripten/Web)

---

## Implemented Solvers

| Solver | Description |
|---|---|
| Bottom-Left-Back | Places boxes starting from the bottom-left-back corner |
| Layer-Based | Packs objects layer by layer |
| Extreme Points | Uses dynamically generated extreme points |
| Greedy (Volume-Based) | Sorts and places boxes greedily |
| Genetic Algorithm | Evolutionary optimization using crossover and mutation |
| Hill Climbing | Local search optimization approach |

---

## Technologies Used

- C++20 Modules
- Raylib
- ImGui
- ImPlot
- rlImGui
- nlohmann/json
- tinyfiledialogs
- Emscripten

---

## Project Structure

```text
src/
├── benchmark/        # Benchmarking utilities
├── loader/           # File loading/saving
├── model/            # Core problem representation
│   └── types/        # Box/orientation definitions
├── random/           # Random number utilities
├── scene/            # Rendering and UI
├── solver/           # Solver framework
│   ├── extremepoints/
│   ├── genetic/
│   ├── greedy/
│   ├── hillclimbing/
│   ├── layerbased/
│   └── solvers/
└── pch/              # Precompiled headers
```

---

## Core Concepts

### Problem Model

The application works with a container and a collection of boxes.

Each box contains:

- Size
- Color
- Allowed orientations

The container size can also be customized.

---

### Box Orientations

The solver supports all 6 valid axis-aligned orientations:

```cpp
ORIENTATION_WLH
ORIENTATION_WHL
ORIENTATION_LWH
ORIENTATION_LHW
ORIENTATION_HWL
ORIENTATION_HLW
```

Each box can restrict which orientations are allowed.

---

## Genetic Algorithm Overview

The Genetic Algorithm implementation includes:

- Random population initialization
- Tournament selection
- Crossover
- Mutation
- Fitness evaluation
- Configurable decoder strategies

Chromosomes are represented using:

```cpp
struct Gene {
    uint32_t m_boxIndex;
    Orientation m_orientation;
};
```

Fitness is based on packing quality and volume utilization.

---

## Visualization

The application includes a real-time 3D viewer where you can:

- Rotate/move the camera
- Inspect packed boxes
- Preview orientations
- Compare solver results
- View performance statistics and plots

---

## File Support

Problems can be:

- Loaded from JSON files
- Saved to JSON files
- Loaded/saved using file picker dialogs

---

## Build Requirements

### Compiler

A compiler with C++20 module support is required.

Recommended:
- Visual Studio 2022
- MSVC with `/std:c++20`

### Dependencies

Required libraries:

- Raylib
- ImGui
- ImPlot
- rlImGui
- nlohmann/json
- tinyfiledialogs

---

## Building the Project

### Visual Studio

1. Open the solution file
2. Set configuration to `Release` or `Debug`
3. Build the project

---

## Running

After building, simply run the executable.

You can:

1. Create or load a problem
2. Select a solver
3. Run the solver
4. Inspect the generated packing solution

---

## Performance Metrics

Each solver reports:

- Volume utilization
- Number of placed boxes
- Total boxes
- Execution time

Example:

```cpp
struct SolverStats {
    float m_volumeUtilization;
    int m_boxesPlaced;
    int m_totalBoxes;
    float m_timeTaken;
};
```

---

## Architecture Notes

The project uses a shared solver interface:

```cpp
class ISolver {
    virtual void solveInternal() = 0;
};
```

All algorithms inherit from this base class, making it easy to add new solvers.

---

## Future Improvements

Possible future additions:

- Simulated Annealing
- Tabu Search
- GPU acceleration
- Better visualization tools
- Multi-container support
- Constraint-based packing
- Benchmark datasets

---

## Authors

- Gindea
- Tocu

---

## License

This project was developed for educational purposes.

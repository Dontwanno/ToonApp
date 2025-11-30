# ToonApp

A real-time 3D rendering engine with toon/cel shading capabilities, built using modern C++ and OpenGL. Features physics simulation, model loading, and an interactive ImGui-based control panel.

## Features

- **Toon Shading**: Custom GLSL shaders implementing cel-shading with discrete lighting levels
- **Physics Simulation**: Integrated Bullet Physics engine for realistic rigid body dynamics
- **Model Loading**: Support for 3D model formats (OBJ, etc.) via Assimp
- **Camera Controls**: First-person style camera with keyboard and mouse input
- **Post-Processing**: Framebuffer-based rendering pipeline for post-process effects
- **Interactive UI**: Real-time parameter adjustment through ImGui interface
- **Entity-Component Architecture**: Flexible scene management with parent-child relationships

## Screenshots

*Coming soon*

## Dependencies

This project uses [vcpkg](https://github.com/microsoft/vcpkg) for dependency management. The following libraries are required:

| Library | Purpose |
|---------|---------|
| [GLFW3](https://www.glfw.org/) | Window creation and input handling |
| [GLM](https://github.com/g-truc/glm) | Mathematics library for graphics |
| [Assimp](https://github.com/assimp/assimp) | 3D model importing |
| [GLAD](https://glad.dav1d.de/) | OpenGL loader |
| [stb](https://github.com/nothings/stb) | Image loading (stb_image) |
| [Bullet3](https://github.com/bulletphysics/bullet3) | Physics simulation |
| [Dear ImGui](https://github.com/ocornut/imgui) | Immediate mode GUI |

## Requirements

- CMake 3.24 or higher
- C++17 compatible compiler
- OpenGL 4.1 compatible GPU
- vcpkg package manager

## Building

### 1. Install vcpkg

```bash
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh  # Linux/macOS
# or
.\bootstrap-vcpkg.bat  # Windows
```

### 2. Install Dependencies

```bash
vcpkg install glfw3 glm assimp glad stb bullet3 imgui[glfw-binding,opengl3-binding]
```

### 3. Configure and Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake
cmake --build .
```

**Note for macOS users**: The project is configured for Apple Silicon (arm64) by default. Modify `CMakeLists.txt` if building for Intel Macs.

### 4. Run

```bash
./ToonGame
```

## Controls

| Key | Action |
|-----|--------|
| W / S | Move forward / backward |
| A / D | Move left / right |
| Space | Move up |
| Left Shift | Move down |
| Mouse | Look around |
| Scroll | Zoom in/out |
| Left Alt | Toggle mouse capture (switch between game/UI mode) |
| Escape | Exit application |

## Project Structure

```
ToonApp/
├── assets/           # 3D models and textures
│   ├── backpack/     # Example model
│   └── shapes/       # Basic shape primitives
├── include/          # Header files
│   ├── Camera.h      # FPS-style camera
│   ├── Entity.h      # Scene entity with physics
│   ├── FrameBuffer.h # Render target management
│   ├── Mesh.h        # Mesh data structures
│   ├── Model.h       # Model loading
│   ├── PhysicsWorld.h # Bullet physics wrapper
│   ├── Scene.h       # Scene management
│   ├── Shader.h      # Shader compilation
│   └── ToonApp.h     # Main application
├── shaders/          # GLSL shader programs
│   ├── toonshader.glsl     # Toon/cel shading
│   ├── regularshader.glsl  # Standard lighting
│   ├── postprocess.glsl    # Post-processing effects
│   └── passthrough.glsl    # Simple passthrough shader
├── src/              # Source files
├── CMakeLists.txt    # Build configuration
└── vcpkg.json        # Dependency manifest
```

## Shaders

The engine includes several shader programs:

- **toonshader.glsl**: Implements cel-shading with discrete lighting bands
- **regularshader.glsl**: Standard Phong-style lighting
- **postprocess.glsl**: Post-processing effects pipeline
- **passthrough.glsl**: Simple texture passthrough for framebuffer display

## UI Controls

The ImGui control panel allows real-time adjustment of:

- **Light Position**: Drag to move the scene light source
- **Light Color**: Color picker for light color
- **Background Color**: Scene background color
- **FPS Display**: Current frame rate

## License

*License information not specified*

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Acknowledgments

- [Learn OpenGL](https://learnopengl.com/) - Excellent OpenGL tutorials
- [Bullet Physics](https://pybullet.org/) - Physics engine documentation
- [Dear ImGui](https://github.com/ocornut/imgui) - Immediate mode GUI library

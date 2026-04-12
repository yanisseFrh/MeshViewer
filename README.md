# Mesh Viewer : a simple object visualizer

[![pipeline status](https://gitlab.com/yanisseF69/MeshViewer/badges/main/pipeline.svg)](https://gitlab.com/yanisseF69/MeshViewer/-/commits/main)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows-lightgrey.svg)]()
[![Qt](https://img.shields.io/badge/Qt-6.6.2-green.svg)]()

**Light and cross-platform 3D visualizer** for meshes (.obj, .off, .txt), made with **C++ / Qt / OpenGL**.  
Load, wiew and export 3D objects and point clouds easly.

## Table of contents

* [Overview](#overview)
* [Features](#features)
* [Installation](#installation)
    * [Prerequisites for compilation](#prerequisites-for-compilation)
    * [Compilation (Linux)](#compilation-linux)
* [Usage](#usage)
    * [Example](#example)
* [Project structure](#project-structure)
* [Author](#author)

## Overview

<img src="https://github.com/yanisseFrh/MeshViewer/blob/main/resources/screenshots/app.png" alt="App screenshot"/>

## Features

- Load 3D mesh (`.obj`, `.off`, `.txt`)
- Export in different formats
- Load textures
- Navigate around the object
- Modern and responsive Qt interface

## Installation

You can download the windows (.zip with all the .dlls needed) or the linux (AppImage) version [here](https://github.com/yanisseFrh/MeshViewer/releases/tag/continuous).

### Prerequisites for compilation
- Qt 6.6+
- CMake ≥ 3.22
- g++ or clang

### Compilation (Linux)

Here is a minimal example for compiling the project in linux, you can also use `ninja` instead of `make`

```bash
git clone https://github.com/yanisseF69/MeshViewer.git
cd MeshViewer
mkdir build && cd build
cmake ..
make -j$(nproc)
./MeshViewer
```

## Usage

Lanch the app and open your objects with :
**File → Load...**

You can :
- Navigate around the object and zoom with your mouse
- Change rendering mode (display the full object or only the mesh triangles)
- Load a texture if the object file has texture coordinates
- Export with the **Save** button

### Example

After loading a file (.txt point cloud), we zoomed in it and displayed only the triangular mesh by activating the **wireframe** checkbox.

<img src="https://github.com/yanisseFrh/MeshViewer/blob/main/resources/screenshots/base&wireframe.png" alt="Base vs wireframe"/>


## Project structure

```markdown
MeshViewer/
├── doc/ # documentation generate by doxygen (only in gh-pages branch)
├── include/ # headers
├── resources/ # icons, screenshots, .desktop file for linux
├── src/ # main source code (C++ / Qt)
├── .gitlab-ci.yml # CI/CD pipeline
├── CMakeLists.txt
├── Doxyfile
└── README.md
```

## Author

Developped by **Yanisse F.** — [GitHub](https://github.com/yanisseFrh)

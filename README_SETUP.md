# StochFit Development Environment Setup

This guide covers setting up StochFit for development on Windows, Linux, and macOS, including optional GPU acceleration (CUDA on Windows/Linux, Metal on macOS).

## Common Prerequisites

### 1. Git
- **Windows**: Download from https://git-scm.com/
- **Linux**: `sudo apt install git` (Ubuntu/Debian) or `sudo yum install git` (RedHat/CentOS)
- **macOS**: `xcode-select --install` (Xcode command line tools)

### 2. CMake (≥3.21)
- **Windows/macOS**: Download from https://cmake.org/download/
- **Linux**: `sudo apt install cmake` (Ubuntu/Debian)

### 3. C++ Compiler
- **Windows**: Visual Studio 2022 Community (free) — install "Desktop development with C++"
  - https://visualstudio.microsoft.com/vs/community/
- **Linux**: `sudo apt install build-essential` (GCC + make)
- **macOS**: `xcode-select --install` (Clang)

### 4. Node.js + npm (for GUI)
- Download LTS from https://nodejs.org/
- Verify: `node --version && npm --version`

---

## Windows Setup

### Step 1: Install MSVC Toolchain
1. Download **Visual Studio 2022 Community** (free)
2. Run installer → select **"Desktop development with C++"**
3. Ensure **MSVC v143+** and **Windows 11 SDK** are included
4. Restart after installation

### Step 2: Clone Repository
```bash
git clone https://github.com/yourusername/stochfit.git
cd stochfit
```

### Step 3: Configure Build
```bash
cmake --preset windows --fresh
```

This automatically:
- Downloads vcpkg and dependencies (LAPACK, OpenMP, GTest)
- Detects CUDA Toolkit if installed (optional, requires NVIDIA GPU)
- Configures Ninja build system with MSVC

### Step 4: Build C++ Components
```bash
cmake --build build --parallel
```

Produces:
- `build/stochfit.dll` — core fitting library
- `build/levmardll.dll` — Levenberg-Marquardt library
- `build/mirefl.exe` — console tool
- `build/stochfit_tests.exe` — unit tests

### Step 5: Run Tests
```bash
./build/stochfit_tests.exe
```

### Step 6: Build GUI (Electron)
```bash
cmake --build build --target gui
```

Produces packaged Electron app in `build/electron/` (e.g., `StochFit.exe`)

---

## Linux Setup

### Step 1: Install Build Tools
**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install -y build-essential cmake git python3-dev
```

**RedHat/CentOS:**
```bash
sudo yum groupinstall -y "Development Tools"
sudo yum install -y cmake git python3-devel
```

### Step 2: Clone Repository
```bash
git clone https://github.com/yourusername/stochfit.git
cd stochfit
```

### Step 3: Configure Build
```bash
cmake --preset default --fresh
```

This automatically:
- Downloads vcpkg and dependencies
- Detects CUDA if present (optional, requires NVIDIA GPU + CUDA Toolkit)
- Uses system compiler (GCC/Clang)

### Step 4: Build C++ Components
```bash
cmake --build build --parallel
```

Produces:
- `build/libstochfit.so` — core fitting library
- `build/liblevmardll.so` — Levenberg-Marquardt library
- `build/mirefl` — console tool
- `build/stochfit_tests` — unit tests

### Step 5: Run Tests
```bash
./build/stochfit_tests
```

### Step 6: Build GUI (Electron)
```bash
cmake --build build --target gui
```

Produces packaged Electron app in `build/electron/`

---

## macOS Setup

### Step 1: Install Xcode Command Line Tools
```bash
xcode-select --install
```

### Step 2: Install Homebrew (optional but recommended)
```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### Step 3: Clone Repository
```bash
git clone https://github.com/yourusername/stochfit.git
cd stochfit
```

### Step 4: Configure Build
```bash
cmake --preset default --fresh
```

This automatically:
- Downloads vcpkg and dependencies
- Detects Metal GPU support (macOS 10.15+)
- Uses Clang compiler

### Step 5: Build C++ Components
```bash
cmake --build build --parallel
```

Produces:
- `build/libstochfit.dylib` — core fitting library
- `build/liblevmardll.dylib` — Levenberg-Marquardt library
- `build/mirefl` — console tool
- `build/stochfit_tests` — unit tests

### Step 6: Run Tests
```bash
./build/stochfit_tests
```

### Step 7: Build GUI (Electron)
```bash
cmake --build build --target gui
```

Produces packaged Electron app in `build/electron/` (e.g., `StochFit.app`)

---

## GPU Acceleration (Optional)

### CUDA (Windows/Linux)

**Requirements:**
- NVIDIA GPU with compute capability ≥ 7.5 (Turing+: RTX 3050, 4060, 4080, etc.)
- NVIDIA CUDA Toolkit 11.2+ installed

**Windows:**
1. Download CUDA Toolkit from https://developer.nvidia.com/cuda-downloads
2. Run installer (adds to PATH automatically)
3. Reconfigure: `cmake --preset windows --fresh`
4. Rebuild: `cmake --build build --parallel`

The build system will auto-detect CUDA and enable GPU path.

**Linux:**
```bash
# Ubuntu 22.04
wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/cuda-keyring_1.1-1_all.deb
sudo dpkg -i cuda-keyring_1.1-1_all.deb
sudo apt update
sudo apt install cuda-toolkit
```

Then reconfigure and rebuild.

### Metal (macOS)

Metal GPU support is automatically enabled on macOS 10.15+ if the Metal framework is available (included with Xcode). No additional installation needed.

---

## Verification

After building, verify the installation:

```bash
# C++ components built
ls -lh build/stochfit* build/levmardll*

# Tests pass
./build/stochfit_tests

# GUI available
ls -lh build/electron/
```

---

## Troubleshooting

### CMake not found
- **Windows**: Add CMake to PATH or use installer with PATH option
- **Linux/macOS**: `which cmake` — if empty, reinstall

### vcpkg downloads fail
- Check internet connection
- Try: `cmake --preset windows --fresh` (forces clean download)

### "npm not found"
- Install Node.js from https://nodejs.org/
- Verify: `npm --version`
- Then: `cmake --build build --target gui`

### CUDA not detected (Windows)
- Install CUDA Toolkit from https://developer.nvidia.com/cuda-downloads
- Ensure CUDA is in PATH: `nvcc --version`
- Reconfigure: `cmake --preset windows --fresh`

### Linker errors on macOS
- Update Xcode: `xcode-select --install`
- Or update via App Store

---

## Development Workflow

1. **Edit code** in `src/`, `include/`, or `gui/`
2. **Rebuild C++**: `cmake --build build --parallel`
3. **Rebuild GUI** (if you changed TypeScript): `cmake --build build --target gui`
4. **Run tests**: `./build/stochfit_tests` (C++) or tests in GUI app
5. **Commit**: `git add -A && git commit -m "..."`

---

## Multi-Config Builds

**Debug build (slower, better error messages):**

- **Windows**: `cmake --preset windows-debug --fresh && cmake --build build`
- **Linux/macOS**: `cmake --preset debug --fresh && cmake --build build`

Release is the default in CMakePresets.json.

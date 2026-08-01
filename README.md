# rag-qt

RAG Client written by qt

## Cuda Support

to enable cuda support, modify vcpkg.json add "cuda" to hanjingo-high-jump features.

vcpkg.json:

```json
"dependencies": [
    {
      "name": "hanjingo-high-jump",
      "features": ["cuda"]
    }
  ]
```

## Build & Compile

to build this project, you should install & config base environment (vcpkg, c/c++ compile tools).

macos:

```bash
brew install ninja cmake autoconf automake autoconf-archive libtool unixodbc libomp
sudo xcode-select --install
sudo xcodebuild -downloadComponent MetalToolchain

export OpenMP_ROOT=/opt/homebrew/opt/libomp

# Build All
cmake -S . -B build/release -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build/release -j8

# Enable Lite compile
cmake -S . -B build/release-lite -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_RELEASE_LITE=ON
cmake --build build/release-lite -j8
```

linux:

```bash
sudo apt-get install ninja-build cmake autoconf automake autoconf-archive libtool unixodbc libomp-dev

# -DBUILD_RELEASE_LITE: Enable Lite compile
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Windows (PowerShell; RECOMMEND TO USE Visual Studio IDE 2022):

```powershell
# from project root
# Build All
cmake -S . -B build/release -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release
cmake --build build/release -j8

# Enable Lite compile
cmake -S . -B build/release-lite -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release -DBUILD_RELEASE_LITE=ON
cmake --build build/release-lite -j8
```

## Pack

macos:

```sh
cd build/release-lite
cpack -C Release -G DragNDrop
```

Windows (PowerShell):

```powershell
cd build/release-lite
cpack -C Release -G NSIS
```
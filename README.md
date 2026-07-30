# rag-qt

RAG Client written by qt

## Cuda Support

to enable cuda support:

```sh
git checkout qwidget-cuda

cd 3rd/rag-core
git checkout cuda
```

## Build & Compile

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

# -DBUILD_RELEASE_LITE: Enable Lite compile
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Windows (PowerShell):

```powershell
# from project root
# -DBUILD_RELEASE_LITE: Enable Lite compile
cmake -S . -B build -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build
```

## Pack

macos:

```sh
cd build/release-lite
cpack -C Release
```
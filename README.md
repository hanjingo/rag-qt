# rag-qt
RAG Client written by qt

macos:

```bash
brew install ninja cmake autoconf automake autoconf-archive libtool unixodbc libomp
sudo xcode-select --install
sudo xcodebuild -downloadComponent MetalToolchain

export OpenMP_ROOT=/opt/homebrew/opt/libomp
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_RELEASE_LITE=ON
cmake --build build
```

linux:

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Windows (PowerShell):

```powershell
# from project root (enable CUDA: -DVCPKG_MANIFEST_FEATURES="ggml[cuda],whisper-cpp[cuda]")
cmake -S . -B build -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build
```
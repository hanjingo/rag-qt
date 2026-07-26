# rag-qt
RAG Client written by qt

macos:

```bash
brew install ninja cmake
sudo xcode-select --install

cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake 
cmake --build build
```

linux:

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake 
cmake --build build
```

Windows (PowerShell):

```powershell
# from project root (enable CUDA: -DVCPKG_MANIFEST_FEATURES="ggml[cuda],whisper-cpp[cuda]")
cmake -S . -B build -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build
```
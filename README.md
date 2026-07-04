# Scrap Mechanic Challenge Pack Extractor
Extract Challenges posted to the work shop into the challenge editor

## Building
1. Install Cmake
2. clone the repo
3. run the following commands in the repo folder
```shell
mkdir build
cd build
cmake .. -DBUILD_AS_WIN32=ON 
cmake --build . --config Release
```

If you have a compiler and build tools installed on your system, this should create a runnable executable file somewhere in the build folder.

Only supports Windows (because Scrap Mechanic only supports windows)
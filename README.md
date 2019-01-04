# BsZenLib

Importers for file from the games Gothic and Gothic II for the [bs:framework](https://github.com/GameFoundry/bsf).

# Documentation

https://regoth-project.github.io/BsZenLib

# Building

You will need the bsframework for building BsZenLib. You can either download or compile it yourself, or have CMake download prebuilt binaries 
for you.

## Let CMake download bs:f

This will download all needed dependencies automatically. 

```sh
git clone --recursive https://github.com/REGoth-project/BsZenLib.git

cd BsZenLib
mkdir build
cd build
cmake -DBSZENLIB_DOWNLOAD_BSF_BINARIES=On ..
cmake --build . -j 8
```

## With external bs:f installation

For this you will have to have an external bs:f-installation:

```sh
git clone --recursive https://github.com/REGoth-project/BsZenLib.git

cd BsZenLib
mkdir build
cd build
cmake -bsf_INSTALL_DIR=path/to/bsf ..
cmake --build . -j 8
```

## Note for Windows users

Make sure to have CMake use the `Win64` kind of the Visual-Studio generator by passing `-G"Visual Studio 15 2017 Win64"` to it.
Otherwise linking with bs:f will likely fail, since the prebuilt binaries are all 64-bit.

## Building the Documentation

You will need:

 * Doxygen
 * Python
  * Sphinx (install via `pip install sphinx`)
  * Breathe (install via `pip install breathe`)

With a cmake-project generated, run this from within the `build`-directory:

```sh
cmake --build . --target BsZenLib_docs
```

This will generate the documentation as HTML into `build/docs-source/html`. 
To update Github-pages, copy the contents of that directory into the `docs` directory at the repository root.
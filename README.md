MfxPlugins
==========

### About

This is a set of simple [OpenMfx](http://openmesheffect.org/) plugins. They can be used in any Digital Content Creation tool able to load OpenMfx plug-ins, for instance [this branch of Blender](https://github.com/eliemichel/OpenMfxForBlender).

### Downloading

You can either use the *download zip* button provided by GitHub or clone the repository using git:

```
git clone https://github.com/eliemichel/MfxPlugins.git
```

### Building

This is a standard [CMake](https://cmake.org/) project. Building it consits in running:

```
mkdir build
cd build
cmake ..
```

Once CMake has run, you can build the project using your favorite IDE or with the following command line in the `build` directory:

```
cmake --build . --config Debug
```

### Running

The output of the build is not an executable. It is a set of OpenFX plug-ins called `MfxSomething.ofx`. They are created within the `build` directory, in `src` or `src/Debug` or `src/Release` or something similar depending on your compiler.

You can open this plug-in in any OpenMfx host, for instance the [OpenMfx for Blender branch](https://github.com/eliemichel/OpenMfxForBlender) using an *OpenMfx modifier* or an *OpenMfx Geometry Node*.

### License

This software as a whole is released under the terms of the MIT License.

```
MfxPlugins is a set of simple OpenMfx plug-ins.

Copyright (c) 2019-2022 -- Élie Michel <elie.michel@exppad.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the “Software”), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

The Software is provided “as is”, without warranty of any kind, express or
implied, including but not limited to the warranties of merchantability,
fitness for a particular purpose and non-infringement. In no event shall the
authors or copyright holders be liable for any claim, damages or other
liability, whether in an action of contract, tort or otherwise, arising
from, out of or in connection with the software or the use or other dealings
in the Software.

```

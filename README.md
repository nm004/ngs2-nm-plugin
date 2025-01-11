NINJA GAIDEN SIGMA 2 NM Plugins
===============================
Author: Nozomi Miyamori

This mod currently supports NINJA GAIDEN SIGMA 2 Master Collection on Steam only.

Features
--------

- NG2 gore effects.
- Built-in mod loader.
- Fix some critical bugs.

Installation
------------

Just copy and paste the contents of "install" folder (not the folder itself) into the game
folder, namely "NINJA GAIDEN Σ2". The game folder looks like below after the installation:

```
<game folder>
  NINJA GAIDEN SIGMA2.exe
  dbghelp.dll (This is the mod's main plugin)
  |
  plugin (other plugins are here)
  |  nmeffect.dll
  |  ...
  |
  mods
  |  01234.dat
  |  05678.dat
  |  ...
  |
  databin
  |  databin
  |
  <other files and folders>
```

"dbghelp.dll" is capable of loading plugins (DLLs) under the "plugin" folder.

For Steam Deck (Linux) users
----------------------------

This mod supports Steam Deck. To enable this mod, you have to specify the "Launch Options" of
the game as below:

WINEDLLOVERRIDES=dbghelp=n,b %command%

Development
-----------

### Build Requirements

 - CMake
 - MinGW-w64 or Visual Studio 2022

### How to build

```
cmake -B build .
make -C build all
```

Acknowledgements
----------------

I would like to express my special thanks to the people who deliver the following
great works:

- TeamNinja and KOEI TECMO for NINJA GAIDEN 2 and NINJA GAIDEN SIGMA 2
- Fiend Busa for NINJA GAIDEN SIGMA 2 BLACK
- ShowR18 for Custom Move Neo

Special Thanks
--------------

- enhuhu
- Guilty
- ZANGETSU
- morbis

License
-------

The source code is distributed under the Creative Commons' CC0 license. The binary
distributable follows the third parties' terms and conditions.

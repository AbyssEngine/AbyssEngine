# Abyss Engine

Abyss Engine clean-room reimplementation of **Diablo 2**, written in C.
The goal is to recreate the original game engine, but on a portable platform that can also easily be modded and
extended.

This is **not** a reverse-engineering project, and no original code from the game is used. It will also not be
compaible with the original game's save files or multiplayer systems.

Before running the engine, make sure it is [configured](#configuration) properly.

You can hang out with the developers and other community members on `#AbyssEngine` at `irc.libera.chat`.
We no longer maintain a Discord presence. Any such servers are not affiliated with this project.

## Supported Platforms

* Windows 10+ (x64, Arm64)
* macOS (Arm64)
* Linux (x64, Arm64)

Other platforms may work, but are not officially supported.

## Building

### Prerequisites

* CMake 3.20 or later
* A C99 compliant compiler (GCC, Clang, MSVC)

Please make sure you recursively pull the submodules when cloning the repository:

```bash
git submodule update --init --recursive
```

### Compiling

#### Visual Studio 2022 / Clion / VSCode (with CMake Tools extensions)

Open the root project folder in the IDE and build the solution.

#### XCode

Generate an XCode project with cmake:

```bash
mkdir build
cd build
cmake .. -G Xcode
```

Then load the generated Xcode project and build it.

#### Command Line

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
```

### Running

On **Linux** and **Windows**, you can run the engine by executing the `abyss` executable in the build directory.

For **macOS**, you can run the engine by executing the `Abyss Engine.app` bundle in the build directory.

Please note that the engine requires the `abyss.ini` configuration file. If you haven't set it,
please refer to the [Configuration](#configuration) section below.

### Output Logs

If you run AbyssEngine from the command line, it will output logs to the console.
On **macOS**, you will need to run the binary directly by executing the binary inside
of the app bundle on the command line:

```bash
./AbyssEngine.app/Contents/MacOS/AbyssEngine
```

## Configuration

### MPQ files

This engine requires the original Diablo 2+LOD MPQ files to run. These files are not included in the repository.
You can legally obtain them by purchasing a physical copy of the game, or from a digital distribution platform
such as battle.net.

There are a lot of pirated versions of the game available online, but we do not condone piracy, and most of them
are modded in a way that is not supported by the engine. Please don't waste developer resources by reporting issues
while using pirated versions of the game.

### abyss.ini

In order for Abyss Engine to run, it needs to load abyss.ini. A template of this file is located at
`/content/abyss.ini` in the source repo, and should be placed in the local settings folder for abyss
based on the platform:

- Windows: `%APPDATA%/abyss/abyss.ini`
- Linux: `~/.config/abyss/abyss.ini`
- MacOS: `~/Library/Application Support/abyss/abyss.ini`

Please make sure that you have copied the template file into that location, and updated it to match your system.
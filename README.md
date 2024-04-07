# Abyss Engine

## Building

### Prerequisites

* CMake 3.20 or later
* A C99 compliant compiler (GCC, Clang, MSVC)

Please make sure you recursively pull the submodules when cloning the repository:

```bash
git submodule update --init --recursive
```

### Compiling

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
```

### Running

On **Linux** and **Windows**, you can run the engine by executing the `abyss` executable in the build directory.

For **macOS**, you can run the engine by executing the `Abyss Engine.app` bundle in the build directory.

### Output Logs

If you run AbyssEngine from the command line, it will output logs to the console.
On **macOS**, you will need to run the binary directly by executing the binary inside
of the app bundle on the command line:

```bash
./AbyssEngine.app/Contents/MacOS/AbyssEngine
```

## Configuration

In order for Abyss Engine to run, it needs to load abyss.ini. A template of this file is located at
`/content/abyss.ini` in the source repo, and should be placed in the local settings folder for abyss
based on the platform:

- Windows: `%APPDATA%/abyss/abyss.ini`
- Linux: `~/.config/abyss/abyss.ini`
- MacOS: `~/Library/Application Support/abyss/abyss.ini`

Please make sure that you have copied the template file into that location, and updated it to match your system.
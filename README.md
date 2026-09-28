# HOI4 Ironman Enabler for Linux

This is a simple project that lets you play with achievements on modded saves.
This project makes the game believe you have a valid checksum, so modded games can unlock achievements.

# Usage

You must install the following requirements:
 * A c compiler: `gcc` or `clang`
 * `gdb`
 * `make`

Use `make` to build the binary.

As root (or using sudo), run the `load.sh` script before starting the game.
Then, the script will prompt you to start the game.

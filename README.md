# wadcli

`wadcli` is a tool for Windows and Linux that allows the reading, creation, and manipulation of Doom WADs through command-line, made with the purpose of faster development for the creation and modification of WAD files, without the need of GUI applications.

For this purpose, `wadcli` is feature-rich, allowing users to add, delete or rename lumps, merge WADs together, extract lumps into a folder, compressing and decompressing them (for compatible Doom games), change and swap lumps' positions, and so on. It is also possible to do multiple actions together, such as renaming lumps that would be added to a WAD file, delete a different lump, change another lump's position, and then compress the WAD, all in one line.

## Support me!

If you feel you would really benefit from this software, please show your support by donating me a Ko-Fi (minimum 3 euro). A simple donation can make my day, and I would really appreciate it. Kind words and feedback are also appreciated, as well.

<a href='https://ko-fi.com/jugadorxei' target='_blank'><img height='36' style='border:0px;height:36px;' src='https://az743702.vo.msecnd.net/cdn/kofi3.png?v=0' border='0' alt=':)' /></a>

## Build instructions

Linux users are recommended to build `wadcli`. Here are the steps to do so:

1. Get g++, make, git and liblzf. In Debian-based systems, using a terminal, this can be done through `sudo apt install g++ make git liblzf`.
2. Clone this repository. Do so through `git clone https://github.com/JugadorXEI/wadcli.git` (or downloading the code directly from GitHub).
3. Enter the project folder using `cd wadcli`.
4. Do `make`. This will build `wadcli`.
5. After building is done, do `sudo make install`. This will allow the use of `wadcli` from anywhere.
6. Done!

The makefile allows for additional flags:
* `WINDOWS=1`: builds `wadcli` for Windows, which requires the 64-bit Mingw32 G++ compiler.
* `STATIC=1`: builds `wadcli` with static libraries. This requires making a static library of `liblzf`, which goes beyond the scope of this readme.

## Examples

Todo.

## List of Commands

Todo.
# wadcli

`wadcli` is a tool for Windows and Linux that allows the reading, creation, and manipulation of Doom WADs through command-line, made with the purpose of faster development for the creation and modification of WAD files, without the need of GUI applications.

For this purpose, `wadcli` is feature-rich, allowing users to add, delete or rename lumps, merge WADs together, extract lumps into a folder, compressing and decompressing them (for compatible Doom games), change and swap lumps' positions, and so on. It is also possible to do multiple actions together, such as renaming lumps that would be added to a WAD file, delete a different lump, change another lump's position, and then compress the WAD, all in one line.

## Support me!

If you feel you would really benefit from this software, please show your support by donating me a Ko-Fi (minimum 3 euro). A simple donation can make my day, and I would really appreciate it. Kind words and feedback are also appreciated, as well.

<a href='https://ko-fi.com/jugadorxei' target='_blank'><img height='36' style='border:0px;height:36px;' src='https://az743702.vo.msecnd.net/cdn/kofi3.png?v=0' border='0' alt=':)' /></a>

## Build instructions

Linux users are recommended to build `wadcli`. Here are the steps to do so:

1. Get g++, make, git, liblzf1 and liblzf-dev. In Debian-based systems, using a terminal, this can be done through `sudo apt install g++ make git liblzf1 liblzf-dev`.
  * For Ubuntu 20.04 and other derivatives, you may need to access the "Universe" repository. You may do so using `sudo add-apt-repository universe` and then `sudo apt-get update`. See also [here](https://askubuntu.com/questions/148638/how-do-i-enable-the-universe-repository).
2. Clone this repository. Do so through `git clone https://github.com/JugadorXEI/wadcli.git` (or downloading the code directly from GitHub).
3. Enter the project folder using `cd wadcli`.
4. Do `make`. This will build `wadcli`.
5. After building is done, do `sudo make install`. This will allow the use of `wadcli` from anywhere.
6. Done!

The makefile allows for additional flags:
* `WINDOWS=1`: builds `wadcli` for Windows, which requires the 64-bit Mingw32 G++ compiler.
* `STATIC=1`: builds `wadcli` with static libraries. This requires making a static library of `liblzf`, which goes beyond the scope of this readme.
* `DEBUG=1`: prints more verbose information.

Windows builds are compiled using `make WINDOWS=1 STATIC=1`.

## Examples

For any further help, do `wadcli --help`.

ZWADs (compressed WADs) behave the same as decompressed WAD files (such as IWADs or PWADs) when it comes to arguments. Files that are added to a ZWAD will be compressed automatically.

### Reading

* `wadcli yourwad.wad` will read `yourwad.wad`, displaying the position of lumps, their size and offset.

### Adding

* `wadcli yourwad.wad --add FILE` will add `FILE` inside `yourwad.wad`, at the bottom of the WAD.
* `wadcli yourwad.wad --add MYFLAT --within F` will add `MYFLAT` between the markers `F_START` and `F_END` if they exist, above `F_END`.
* `wadcli yourwad.wad --add MyReallyLongLuaFile.lua --rename LUA_COOL` will add `MyReallyLongLuaFile.lua` at the bottom of the WAD, and rename it to `LUA_COOL`.
* `wadcli yourwad.wad --add MAINCFG --overwrite` will add `MAINCFG` inside `yourwad.wad`, and overwrite the same file inside the wad if it exists.
* `wadcli yourwad.wad --create-markers P` will create markers `P_START` and `P_END` inside `yourwad.wad`.

### Removing

* `wadcli yourwad.wad --delete FILE1 FILE2 FILE3` will delete `FILE1`, `FILE2`, `FILE3` inside `yourwad.wad`.
* `wadcli yourwad.wad --delete ?3` will delete the WAD positioned at index 3 inside `yourwad.wad`.

### Lump Positioning

* `wadcli yourwad.wad --input LUMP1 LUMP2 --swap` will swap the positions of `LUMP1` and `LUMP2` inside `yourwad.wad`.
* `wadcli yourwad.wad --input LUMP1 LUMP2 --position +3` will move `LUMP1` and `LUMP2` three lumps above their index inside `yourwad.wad`.
* `wadcli yourwad.wad --input LUMP1 --position 7` will move `LUMP1` into the index 7 inside `yourwad.wad`.

### Compression

This is only for games that support ZWADS, such as [Sonic Robo Blast 2](https://git.do.srb2.org/STJr/SRB2) and derivatives, such as [Kart](https://git.do.srb2.org/KartKrew/Kart-Public) and [Persona](https://git.do.srb2.org/SinnamonLat/SRB2/tree/srb2p_22).

* `wadcli yourwad.wad --compress` will compress `yourwad.wad` and turn it into a ZWAD.
* `wadcli yourwad.wad --decompess` will decompress `yourwad.wad` and turn it into a PWAD. Passing `--decompress I` will turn it into an IWAD instead.

### Extracting Lumps

* `wadcli yourwad.wad --extract LUMP1` will extract `LUMP1` from `yourwad.wad`.
* `wadcli yourwad.wad --extract-all --path ./your/folder/here --no-extension` will extract all lumps from `yourwad.wad`, remove the default extension given to the files, and put them in `./your/folder/here`.

### Other utilities

* `wadcli yourwad.wad --input LUMP1 LUMP2 --rename LUA_HI SOC_BUZZ` will rename the lumps `LUMP1` and `LUMP2`, inside `yourwad.wad`, into `LUA_HI` and `SOC_BUZZ`, respectively.
* `wadcli yourwad.wad [some other actions here] --output newwad.wad` will, after any actions done by the user, be exported as `newwad.wad`.
* `wadcli yourwad.wad --merge coolwad.wad funnywad.wad` will merge the contents of `yourwad.wad`, `coolwad.wad` and `funnywad.wad` together.

## Missing Features

* Converting image files into graphics lumps is currently not supported.
* Wildcards are currently not supported.
# HunkInfo
***Object file information tool for the Amiga Hunk binary files***

HunkInfo is a tool to get structural information from an Amiga Hunk binary object, just like otool in MacOS (Mach-O files) or readelf in Linux (ELF files).

Amiga Hunk is the binary format used by classic AmigaOS and it's used for regular executables as well as for libraries, devices and other stuff.

For general information on the Amiga Hunk check out the [Wikipedia entry](https://en.wikipedia.org/wiki/Amiga_Hunk).

### Build

	$ make hunkinfo

### Usage

	$ hunkinfo FILENAME

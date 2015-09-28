# foo_subsonic

**This is highly experimental code.**

*This code is far away from being "ready to use".*

Only a few things are tested, and receiving the complete album list of subsonic is currently disabled.
The search feature also needs some improvements.

I'm not a C++ programmer and just playing around here, so please be careful.
I except to have a lot of memory leaks and/or doing bad practices here!

## Requirements:
	* TinyXML (http://sourceforge.net/projects/tinyxml/)
	* SQLite3 amalgamation source (https://www.sqlite.org/download.html)
	* SQLiteCPP (https://github.com/SRombauts/SQLiteCpp)
	* Foobar2000 SDK (http://www.foobar2000.org/SDK)
	* MS Visual Studio (at least version 2010, I use 2015 Community Edition)

## Build Setup
	1) Extract Foobar2000 SDK
	2) Clone or extract foo_subsonic source into the the same folder as foobar2000 SDK (so foo_subsonic directory is in the same path as foo_sample)
	3) Extract TinyXML source in the same directory as foo_subsonic source
	4) Extract SQLite3 amalgamation source to foo_subsonic/sqlite3
	5) Extract SQLiteCPP and copy the *.cpp files to foo_subsonic/sqlite3. Then copy the files from includes/SQLiteCPP/*.h to foo_subsonic/sqlite3/SQLiteCPP
	6) Start Visual Studio and open the foo_subsonic.sln

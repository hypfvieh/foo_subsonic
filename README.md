# foo_subsonic

**!!This project is retired!!**

**This is highly experimental code. Use at your own risk!**


I'm not a C++ programmer and just playing around here, so please be careful.
I expect to have a lot of memory leaks and/or doing bad practices here!

## License
My code is licensed with the [WTFPL](http://www.wtfpl.net).

All used libraries and the foobar SDK are licensed under different license agreements.
Please look at the specific project for further details.

## Requirements:
* [WTL 9.x](http://wtl.sourceforge.net/) (I used 9.0) 
* [TinyXML](http://sourceforge.net/projects/tinyxml/)
* [SQLite3 amalgamation source](https://www.sqlite.org/download.html)
* [SQLiteCPP](https://github.com/SRombauts/SQLiteCpp)
* [Foobar2000 SDK](http://www.foobar2000.org/SDK) (I used 2015-01-14, but newer versions should also be fine)
* MS Visual Studio (at least version 2010, I use 2015 Community Edition)

## Build Setup
1. Extract Foobar2000 SDK
2. Clone or extract foo_subsonic source into the the same folder as foobar2000 SDK (so foo_subsonic directory is in the same path as foo_sample)
3. Extract TinyXML source in the same directory as foo_subsonic source
4. Extract SQLite3 amalgamation source to foo_subsonic/sqlite3
5. Extract SQLiteCPP and copy the *.cpp files to foo_subsonic/sqlite3. Then copy the files from includes/SQLiteCPP/*.h to foo_subsonic/sqlite3/SQLiteCPP
6. Start Visual Studio and open the foo_subsonic.sln


## Help
Usage and FAQ can be found in the [Wiki](https://github.com/hypfvieh/foo_subsonic/wiki)

## Missing Features
* Drag and Drop is not working (any help appreciated)
* List should have a built-in search box (edit control) to do direct filtering

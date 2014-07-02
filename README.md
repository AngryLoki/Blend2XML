Blend2XML
=========

Command-line tool that converts [Blender 3D](http://www.blender.org/) files into human-readable XML. 

Requires Qt Core and Qt XML libraries. Compile with `qmake && make`.

Usage
-----

```
blend2xml.exe [options] source

Options:
  -?, -h, --help       Displays this help.
  -v, --version        Displays version information.
  -o, --output <file>  Destination .xml file.
  --notypes <file>     Disable type info.
  --nodata             Disable data info.
  --rawpointers        Print raw pointers.

Arguments:
  source               Source .blend file.
```

Example output (with `--notypes` option):

```xml
<?xml version="1.0" encoding="UTF-8"?>
<blend identifier="BLENDER" pointer-size="8" endianness="v" version-number="271">
    <Link block="REND">
        <next type="Link*">0xDEADBEEF</next>
        <prev type="Link*">0xDEADBEEF</prev>
    </Link>
    <Link block="TEST">
        <next type="Link*">0xDEADBEEF</next>
        <prev type="Link*">0xDEADBEEF</prev>
    </Link>
    <FileGlobal block="GLOB">
        <subvstr type="char[4]" mode="ascii">   0</subvstr>
        <subversion type="short">0</subversion>
        <pads type="short">0</pads>
        <minversion type="short">270</minversion>
        <minsubversion type="short">5</minsubversion>
        <displaymode type="short">1</displaymode>
        <winpos type="short">256</winpos>
        <curscreen type="bScreen*">0xDEADBEEF</curscreen>
        <curscene type="Scene*">0xDEADBEEF</curscene>
        <fileflags type="int">0b10000000000001000000000000</fileflags>
        <globalf type="int">270464</globalf>
        <build_commit_timestamp type="uint64_t">0</build_commit_timestamp>
        <build_hash type="char[16]" mode="ascii">unknown</build_hash>
        <filename type="char[1024]" mode="ascii">C:\Users\user101\Documents\zak.blend</filename>
    </FileGlobal>
    <wmWindowManager block="WM">
        <id type="ID">
            <next type="void*">NULL</next>
            <prev type="void*">NULL</prev>
            <newid type="ID*">NULL</newid>
            <lib type="Library*">NULL</lib>
            <name type="char[66]" mode="ascii">WMWinMan</name>
            <!-- Skip another XXXXX lines -->
```

Note, that this tool produces huge XML files. Some editors won't be able to open 10 MB of XML.

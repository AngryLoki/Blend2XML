Blend2XML
=========

Command-line tool that converts [Blender 3D](http://www.blender.org/) files into human-readable XML. 

Requires Qt Core and Qt XML libraries. Compile with `qmake && make`.

Usage
-----

```
blend2xml filename.blend > output.xml
```

Example output:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<blend>
    <header identifier="BLENDER" pointer-size="8" endianness="v" version-number="268"/>
    <REND sdna="Link">
        <next type="Link*">0xDEADBEEF</next>
        <prev type="Link*">0xDEADBEEF</prev>
    </REND>
    <GLOB sdna="FileGlobal">
        <subvstr type="char[4]">
            <string>   0</string>
        </subvstr>
        <subversion type="short">0</subversion>
        <pads type="short">0</pads>
        <minversion type="short">262</minversion>
        <minsubversion type="short">0</minsubversion>
        <displaymode type="short">1</displaymode>
        <winpos type="short">256</winpos>
        <curscreen type="bScreen*">0xDEADBEEF</curscreen>
        <curscene type="Scene*">0xDEADBEEF</curscene>
        <fileflags type="int">33558528</fileflags>
        <globalf type="int">262272</globalf>
        <revision type="int">0</revision>
        <pad type="int">0</pad>
        <filename type="char[1024]">
            <string>/home/lockal/bin/blender/tiny.blend</string>
        </filename>
    </GLOB>
    <WM sdna="wmWindowManager">
        <id type="ID">
            <next type="void*">NULL</next>
            <prev type="void*">NULL</prev>
            <newid type="ID*">NULL</newid>
            <lib type="Library*">NULL</lib>
            <name type="char[66]">
                <string>WMWinMan</string>
            </name>
    <!-- Skip another XXXXX lines -->
```

Note that `blend2xml` does not print pointers: it is mostly useless, because blender changes pointers on each save.

Also note, that this tool produces huge XML files. Some editors won't be able to open 10 MB of XML.

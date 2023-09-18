<img src="assets/mouse.png" align="right" />

# Enhance Pointer Precision Tool

[![MSBuild](https://github.com/nefarius/EPPT/actions/workflows/msbuild.yml/badge.svg)](https://github.com/nefarius/EPPT/actions/workflows/msbuild.yml) ![GitHub all releases](https://img.shields.io/github/downloads/nefarius/EPPT/total)

A simple, self-contained Windows tool to display and toggle "Enhanced Pointer Precision" a.k.a. mouse acceleration.

## About

This little tool was birthed during an FPS gaming session where we tested different mouse settings and quickly realized that a lot of mouse configuration software (like iCUE from CORSAIR) likes to silently turn mouse acceleration on which distorts the aim feel in games like Quake Champions. This option is buried somewhere deep within Windows' nonsensical UI so I made this tool for it. It's deliberately kept minimal; just a modal window with a checkbox that represents the current state and lets you toggle it.

## Screenshots

![EPPT_ohWuHUA6P8.png](assets/EPPT_ohWuHUA6P8.png)

## Command Line Switches

You can toggle acceleration on and off from the command line (autostart) if you like as follows; disable:

```cmd
.\EPPT.exe --disable --exit
```

And enable:

```cmd
.\EPPT.exe --enable --exit
```

If you omit the `--exit` switch the main window will get displayed.

### Register in Autostart

The following sequence registers the tool in the current user's autostart, disables acceleration and then exits without creating a window:

```cmd
.\EPPT.exe -r --with-disable --with-exit -a
```

### Unregister from Autostart

The following sequence removes the tool from the current user's autostart and then exits without creating a window:

```cmd
.\EPPT.exe -u -a
```

## 3rd party credits

- [Dear ImGui](https://github.com/ocornut/imgui)
- [Simple and Fast Multimedia Library](https://www.sfml-dev.org/)
- [ImGui-SFML](https://github.com/SFML/imgui-sfml)
- [Argh! A minimalist argument handler](https://github.com/adishavit/argh)

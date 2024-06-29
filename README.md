
# Unreal Editor Stream Deck Plugin

The `Unreal Editor Stream Deck Plugin` a plugin for controlling `Unreal Editor` blueprint debugging using `Stream Deck`

## Description

The `Unreal Editor Stream Deck Plugin` provides several buttons required for debugging in the `Unreal Editor`. You can directly control the debugging of the `Unreal Editor` through the `Stream Deck`. The debug status of `Unreal Editor` will also be displayed in your `Stream Deck`.

<img width="359" alt="streamdeck_all_buttons" src="https://github.com/Augkit/unreal-editor-streamdesk-plugin/assets/23498912/30b8b5ee-1dc7-4201-823c-181206d431c8">

## How to Install

A short guide to help you get started quickly.

### Clone the repo

Execute the following command in the `<Unreal Project | Unreal Engine>/Plugins` directory

```
git clone https://github.com/Augkit/unreal-editor-streamdesk-plugin --recursive
```

> `<Unreal Project>` is the directory where `*.uproject` is located.

### Install to Stream Deck

Create symlink to Stream Deck plugins directory.

```
./Scripts/symlink.deck.bat
```

### Enjoy It

Restart Stream Deck app and run Unreal Editor.

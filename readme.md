# GW2 Mounts

A plugin to show a convenient, customizable [radial menu overlay](https://giant.gfycat.com/SnoopyEarlyBlacknorwegianelkhound.mp4) to select a mount on the fly for *Guild Wars 2: Path of Fire*.

## Installation
- Place in your bin64 directory (default path: ``C:\Program Files\Guild Wars 2\bin64``).
- Run the game!

## Usage
- When in game, press ``Shift+Alt+M`` for the options menu.
- Set a keybind to use the overlay. This is the only keybind you will use in practice.
- Set each game keybind for the mounts (you must set these first in your game options, under controls, after you've unlocked the mounts). These are used by the plugin to trigger mounting.
- Hold the main key to show overlay, move mouse in the desired direction, release key to trigger mounting/dismounting.

## FAQ

### Q: I want to load up ReShade/SweetFX/GW2Hooks/something else which also needs to be called ``d3d9.dll``, how do I load both?

A: You have two options: either the other thing you want to run supports *chainloading* this, in which case you should look up the documentation for that plugin (e.g. ArcDPS supports chainloading by renaming this plugin to ``d3d9_chainload.dll``), or you can make this plugin chainload something else by renaming that other plugin to ``d3d9_mchain.dll``.

### Q: Does this support the [REDACTED] mount?

A: Yes, activate it in the settings! It'll appear in the middle of the radial menu such that merely pressing and immediately releasing the main keybind (without moving the mouse) will select it.

### Q: Can I use this code for my project?

A: Absolutely! This whole repository is MIT licensed, so everything here is up for grabs. I would of course appreciate a small note or crediting if you do end up using something, but none of that is required.

### Q: I'm getting a bug/crash/issue that's not mentioned here, what do I do?

A: Please make an issue in the Github page and label it as ``bug`` or ``question`` as relevant to let me know, I'll do my best to help!

### Q: I have an idea for a new feature/new addon!

A: Feel free to make an issue in the Github page and label it as ``suggestion``!
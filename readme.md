# GW2 Radial

[![GitHub all releases](https://img.shields.io/github/downloads/Friendly0Fire/GW2Radial/total)](https://github.com/Friendly0Fire/GW2Radial/releases/latest) [![GitHub Workflow Status](https://img.shields.io/github/workflow/status/Friendly0Fire/GW2Radial/CI)](https://github.com/Friendly0Fire/GW2Radial/actions/workflows/main.yml) [![Discord](https://img.shields.io/discord/384735285197537290?label=Discord)](https://discord.gg/zqeHCEg)


An [*ArenaNET-approved<sup>TM</sup>*](https://www.reddit.com/r/Guildwars2/comments/746mar/mount_radial_menu_addon_very_alpha_much_untested/dnwqj9x/) addon to show a convenient, customizable radial menu overlay to select a mount, novelty item and more, on the fly, for *Guild Wars 2*.

[![GW2Radial Demo](https://thumbs.gfycat.com/IgnorantIllfatedCrocodileskink-size_restricted.gif)](https://gfycat.com/ignorantillfatedcrocodileskink)
(click to see a better video)

**Highlights include:**
* Predefined menus for mounts, novelties and markers
* Simple system to add custom radial menus and share them
* Conditional menus, making it possible to have multiple menus on the same key depending on the situation
* Smart automount: Skimmer is auto-selected when underwater or Warclaw when in WvW
* Input queuing: combat (and underwater without the Skimmer mastery) prevents mount usage, so the selected mount is "queued" until out of combat:

[![Input Queuing GW2Radial Demo](https://thumbs.gfycat.com/NegativeBlushingHake-size_restricted.gif)](https://gfycat.com/negativeblushinghake)
(click to see a better video)

## Installation

### Automatic Installation
- Download the [GW2 Addon Manager](https://github.com/gw2-addon-loader/GW2-Addon-Manager).
- From the list of addons, select GW2Radial.
- Click Update.
- Run the game!

### Manual Installation
- Install the [addon loader](https://github.com/gw2-addon-loader/loader-core) and [d3d wrapper](https://github.com/gw2-addon-loader/d3d9_wrapper).
- Download and extract the archive ``gw2radial.zip`` found in the [latest release](https://github.com/Friendly0Fire/GW2Radial/releases/latest).
- Place ``gw2addon_gw2radial.dll`` in your addons folder inside a new folder named `gw2radial` (with the default game install path, this would be ``C:\Program Files\Guild Wars 2\addons\gw2radial``).
- Run the game! If everything was setup properly, you should be greeted by a prompt on your first launch.

## Usage
- When in game, press ``Shift+Alt+M`` for the options menu.
- Set a keybind to use the overlay. This is the only keybind you will use in practice.
- Set each game keybind for the mounts (you must set these first in your game options, under controls, after you've unlocked the mounts). These are used by the plugin to trigger mounting. They should be matching up like so: ![Setting your keybinds](https://i.imgur.com/gvQPQfX.png)
- Hold the main key to show the overlay, move the mouse in the desired direction, then release the key to trigger mounting/dismounting.

## Custom Radial Menus [New!]
Please refer to the readme in the `custom_examples` folder for more information on how to create or download new menus.

## Credits
- @QuitarHero and @TanukiSoup for extensive testing
- The Xunlai Addons Discord server members for their continued support
- Cerulean Dream for providing me with the initial motivator and inspiration in making his AutoHotkey-based radial menu
- Ghost for the new mount art found in v0.3+
- Skiff and fishing icons by John Mausson
- [freepik](https://www.freepik.com/) for novelty art
- deltaconnected and Bhagawan for their amazing addons which helped frame this and direct the approach to take
- WoodenPotatoes for the great feedback and exposure, as well as the Spud Club for helping with testing
- /u/that_shaman for another really nice radial menu concept which requires far better Photoshop skills than I have to reproduce

## FAQ

### Q: I want to thank you in some way, how do I do that?

A: I do this for fun/because I wanted this to exist. If you really want to, feel free to throw me a few dollars on [Ko-fi](https://ko-fi.com/friendlyfire) or send me mail in game to my account ``FriendlyFire.6275`` and toss a bit of gold or mats my way!

### Q: The addon won't load or crashes.

A: There can be a lot of reasons for this. Try the following debugging steps:

1. Look in your Guild Wars 2 installation folder for a file named ``gw2radial.log``.
2. If it exists, look inside the log file for details on what might be going wrong. If you cannot find anything, also look if there are files of the format ``gw2radial_*.dmp``. If those exist, the addon is loading but crashing at some point. Zip up the files and provide upload them somewhere safe, then open an issue.
3. If it does not exist, look for a file named ``gw2al_log.txt``.
4. If it does not exist, your addon loader is not installed correctly. Head over to the [addon loader](https://github.com/gw2-addon-loader/loader-core) project for support.
5. Inside the log file, look for the line "Loaded gw2radial". If it is not present, the addon loader cannot load the addon correctly. Make sure you have installed the [d3d wrapper](https://github.com/gw2-addon-loader/d3d9_wrapper) addon and look at the other log messages for information on what might be going wrong.

If all else fails, please open an issue or [reach out on Discord](https://discord.gg/NrnW29fVpn).

### Q: What is the "show in center" keybind used for?

A: It's a convenience feature for Action Camera users. When using Action Camera, the cursor is hidden and replaced with a targeting reticule fixed in the middle of the screen. Since that means the radial menu would appear all over the place, that keybind can be used to make it show up in the middle of the screen instead. As a bonus, it'll recenter the mouse to the middle of the radial menu so it's as easy as possible to select a mount *and* it'll show a temporary cursor so you know exactly where you're pointing.

Unfortunately, I can't make this automatic (i.e. switching to that mode when Action Camera is enabled) without hooking game functions, which would then require this become closed source again to avoid cheaters using it. On top of that, I'd be far more likely to break the EULA, so I'm afraid it's off limits without an official API.

### Q: Can you make it so selecting a mount while already mounted will directly swap to the new mount?

No. I talked with ArenaNet devs about this and they've decided that the can of worms it could potentially open is not worth it. Unfortunately, you'll need to separately unmount then select the new mount you want to use. This will not change unless ArenaNet's policy on addons changes.

### Q: Can I use this code for my project?

A: Absolutely! This whole repository is MIT licensed, so everything here is up for grabs. I would of course appreciate a small note or crediting if you do end up using something, but none of that is required.

### Q: I'm getting a bug/crash/issue that's not mentioned here, what do I do?

A: Please make an issue in the Github page and label it as ``bug`` or ``question`` as relevant to let me know, I'll do my best to help! You can also [reach out on Discord](https://discord.gg/NrnW29fVpn).

### Q: I have an idea for a new feature/new addon!

A: Feel free to make an issue in the Github page and label it as ``suggestion``! You can also [reach out on Discord](https://discord.gg/n2fgSAXkEc).

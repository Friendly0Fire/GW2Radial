# GW2 Mounts

A plugin to show a convenient, customizable [radial menu overlay](https://giant.gfycat.com/SnoopyEarlyBlacknorwegianelkhound.mp4) to select a mount on the fly for *Guild Wars 2: Path of Fire*.

## Installation
- Download the archive ``gw2_mounts.zip`` found in the [latest release](https://github.com/Friendly0Fire/GW2Addons/releases/latest) (link is at the very bottom).
- Place ``d3d9.dll`` in your bin64 directory (default path: ``C:\Program Files\Guild Wars 2\bin64``).
- Run the game!

## Usage
- When in game, press ``Shift+Alt+M`` for the options menu.
- Set a keybind to use the overlay. This is the only keybind you will use in practice.
- Set each game keybind for the mounts (you must set these first in your game options, under controls, after you've unlocked the mounts). These are used by the plugin to trigger mounting. They should be matching up like so (credit /u/Mikun for the image): ![Keybinds](https://i.imgur.com/y1aG8jA.png)
- Hold the main key to show overlay, move mouse in the desired direction, release key to trigger mounting/dismounting.

## Credits
- Cerulean Dream for providing me with the initial motivator and inspiration in making his AutoHotkey-based radial menu
- Ghost for the new mount art found in v0.3+
- Lavender for showing me where to get the mount concept art (especially the separate Skimmer image)
- deltaconnected and Bhagawan for their amazing addons which helped frame this and direct the approach to take, including the ReShade compatibility fixes
- Shaun Lebron for his nice, straightforward [D3D9 wrapper](https://gist.github.com/shaunlebron/3854bf4eec5bec297907)
- Dulfy and /u/Levi4than for the Griffon ingame art screenshot
- /u/that_shaman for another really nice radial menu concept which requires far better Photoshop skills than I have to reproduce

## FAQ

### Q: I want to thank you in some way, how do I do that?

A: I do this for fun/because I wanted this to exist, but if you really want to, feel free to send me mail in game to my account ``FriendlyFire.6275`` and toss a few gold my way or just say hi!

### Q: I want to load up ArcDPS/GW2Hook/something else which also needs to be called ``d3d9.dll``, how do I load both?

A: You have two options: either the other thing you want to run supports *chainloading* this, in which case you should look up the documentation for that plugin (e.g. ArcDPS supports chainloading by renaming this plugin to ``d3d9_chainload.dll``), or you can make this plugin chainload something else by renaming that other plugin to ``d3d9_mchain.dll``.

The most common use case would be combining ArcDPS, GW2Hook and this. For this instance, I heavily recommend setting things up as follows:
- ArcDPS is named ``d3d9.dll``.
- GW2Mounts is named ``d3d9_chainload.dll``.
- GW2Hook is named ``ReShade64.dll``.

This should allow all addons to load properly. Note that there is special code present within GW2Mounts to load GW2Hook properly, but this could break unexpectedly if GW2Hook changes.

Finally, note that combining addons is largely unsupported. I will attempt to keep ArcDPS and GW2Hook compatible with this, but that is the most I am able to do in a reasonable amount of time.

### Q: I'm having a crash on launch mentioning "Coherent DLL", what do?

A: There seems to be a lot of potential reasons for this particular crash. [BGDM's website](http://gw2bgdm.blogspot.com/p/faq.html#2.5) lists quite a few. I'd especially recommend making sure you have the very latest [VC++ Redist](https://go.microsoft.com/fwlink/?LinkId=746572).

### Q: What is the "center locked" keybind used for?

A: It's a convenience feature for Action Camera users. When using Action Camera, the cursor is hidden and replaced with a targeting reticule fixed in the middle of the screen. Since that means the radial menu would appear all over the place, that keybind can be used to make it show up in the middle of the screen instead. As a bonus, it'll recenter the mouse to the middle of the radial menu so it's as easy as possible to select a mount *and* it'll show a temporary cursor so you know exactly where you're pointing.

Unfortunately, I can't make this automatic (i.e. switching to that mode when Action Camera is enabled) without hooking game functions, which would then require this become closed source again to avoid cheaters using it. On top of that, I'd be far more likely to break the EULA, so I'm afraid it's off limits without an official API.

### Q: Can I use this code for my project?

A: Absolutely! This whole repository is MIT licensed, so everything here is up for grabs. I would of course appreciate a small note or crediting if you do end up using something, but none of that is required.

### Q: I'm getting a bug/crash/issue that's not mentioned here, what do I do?

A: Please make an issue in the Github page and label it as ``bug`` or ``question`` as relevant to let me know, I'll do my best to help!

### Q: I have an idea for a new feature/new addon!

A: Feel free to make an issue in the Github page and label it as ``suggestion``!
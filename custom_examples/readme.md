# Custom Radial Menus How-To

Creating custom radial menus is a fairly simple process. Everything happens in the `addons/gw2radial/custom` folder of your Guild Wars 2 installation directory.

To create a new menu, first make a new folder. Its name does not matter. Inside, create a file named `config.ini` which must at minimum contain the following:

```ini
[General]
nickname = <some name using only alphanumerical characters and underscores>
display_name = <the name of the radial menu in game>
only_out_of_combat = <true|false, whether the radial menu works in combat, will queue input otherwise, defaults to false>
only_above_water = <true|false, whether the radial menu works in water, will queue input otherwise, defaults to false>
```

The two entries are self-explanatory. From then on, you can add any number of menu entries as follows:

```ini
[<entry nickname>]
name = <optional, name of the entry, will also be used as a fallback if no icon is provided, defaults to the nickname>
color = <color of the entry in menus, will also be used to colorize the entry on the radial menu, defaults to white>
icon = <optional, path to an image relative to this folder that will be used in the radial menu>
shadow_strength = <optional, intensity of the shadow drawn under the icon, defaults to 1 (i.e. 100%)>
colorize_strength = <optional, amount of tinting towards the provided color that will be applied to the icon, defaults to 1 (i.e. 100%)>
premultiply_alpha = <optional, if transparency is wrong, set this to false, defaults to true>
```

A few notes:
* If `name` is not provided, the entry's nickname will be used as is, including capitalization.
* `color` is a trio of numbers from 0 to 255 separated by commas, e.g. 255, 0, 0 for pure red.
* `icon` is the file name for an image, relative to the configuration file's own folder. Any file format supported by the [Windows Imaging Component](https://docs.microsoft.com/en-us/windows/win32/wic/-wic-about-windows-imaging-codec#native-codecs) is usable.
* `shadow_strength` is a value from 0 to 1, with 1 indicating a fully opaque shadow.
* Likewise, `colorize_strength` is a value from 0 to 1, with 1 indicating a fully colorized icon. This can be useful if using a black and white (or grayscale) image for the icon, as it will tint the image with the provided `color` automatically.

Two examples are provided, one using the bare minimum features and the other using most available features.

## Distribution

In addition to a simple folder, custom radial menus can also be zipped up and shared directly. Simply dropping the ZIP file directly into the `custom` folder is enough, no need to unzip it.

## Development

To make creation of new custom radial menus as easy as possible, some settings are available in the Misc tab of the addon's menu.

* "Reload custom wheels on focus" will automatically reload all custom radial menus whenever you tab back into the game. This can make the iteration process very quick and easy. Note that this setting is reset after every launch to prevent forgetting it.
* "Reload custom wheels" is just a button that allows manually reloading all custom radial menus on demand.

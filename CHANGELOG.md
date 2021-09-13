First of all: sorry for how long this release took. It's been a long and bizarre year. But, we've arrived: version 2.1, the ultimate GW2Radial, is finally with us.

The changelog is pretty massive. Some highlights:
* Custom radial menus are here! Head on over to the [examples](https://github.com/Friendly0Fire/GW2Radial/tree/master/custom_examples) folder to learn more, the process is very simple. You are encouraged to share your custom menus to the community!
    * Use your own images, or let the addon generate text on the fly.
    * Enable as many radial menus as you'd like.
    * Each menu has its own full array of customization options like the built-in ones.
* Added *Conditions*, which allow you to only enable certain radial menus under specific conditions, such as being in combat or using a character of a specific profession. Multiple radial menus can now share keybinds! *N.B. Behavior is undetermined if two or more menus are enabled at the same time on the same keybind.*
* Smart Water™\*: the radial menu will automagically select the Skimmer while underwater (if the mastery checkbox is ticked in the settings) or on the surface of water. No more faffing about to find the one mount that works!
* Smart War™\*: the Warclaw is now automatically mounted when in WvW without displaying the menu. This works even if the Warclaw is hidden from the radial menu.
* Added ability to "queue" up a mount command based on certain conditions, which will automatically mount you up with the selected mount when the conditions pass. This "queue" will expire after a customizable timer has elapsed, which is displayed over your cursor if active.
   * Main check: combat! You can now select a mount while in combat and you'll automatically get mounted when you break combat.
   * Secondary check: water! You will not be mounted while underwater (unless you have the mastery and ticked the option in the settings menu), and you'll get mounted when you surface. This of course synergizes with Smart Water™.

*: Not really a trademark. Patents not pending.

Here's the full rundown:
* Complete rewrite of keyboard input handling. This should hopefully fix all the keyboard layout issues for good. *You may need to redefine certain keybinds after this update, sorry!*
* Tweaked visual appearance to try to match to the game's UI even more.
* Added opacity slider for radial menus.
* You can now choose to reset the cursor back to its original location after selecting an option on a radial menu.
* You can now choose to left click on an option in a radial menu instead of releasing the menu's opening keybind over it.
* You can now disable a radial menu while in combat, preventing its keybinds from opening it.
* The Mounts radial menu will not be shown if you are mounted, its keybinds will simply dismount you immediately.
* Radial menus will not send keybinds to text boxes such as the chat box. This will effectively cancel the action however.
* Similarly, radial menus will not be triggered while input is in a text box.
* Better [d912pxy](https://github.com/megai2/d912pxy) integration.
* [Addon loader](https://github.com/gw2-addon-loader) support. It is recommended to use the [GW2 Addon Manager](https://github.com/gw2-addon-loader/GW2-Addon-Manager) to handle this easily. *This is now the recommended method to install GW2Radial.*
* Added user-friendly tooltips for most options.
* Improved settings menu in various other ways.
* UI/DPI scaling support for some elements.
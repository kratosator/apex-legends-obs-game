# Apex Legends OBS Game

## Features

 - Automatic recognition of in-game character in all HUDs (in-game, inventory, looting, map view)
 - Possibility to change a text source with the name of the current character (with in-game HUD only as for now)
 - Possibility to disable/enable sources depending on which HUD is currently displayed
 - Frame accurate character recognition and source activation/deactivation

## Description

OBS plugin that allows stream customization targeted specifically to Apex Legends game. The plugin is a filter that can be applied on any source or scene.

For example, it allows to apply a character card overlay in the correct screen position depending on the HUD showed by the game. There are 4 HUDs where the player card is recognized: in-game, inventory, looting box, map view.

In order to work properly it is required to bind F4 as key used to show character information (Settings -> Mouse/Keyboard -> Character Info).
## Configuration

Configuration is pretty straight forward, apply the filter "Apex Game" on the source/scene that contains Apex Legends gameplay. Configure sources in the menu, each source will be activated only when the corresponding HUD condition is showed in the game. Character name MUST be a text source and will be changed by the filter.

[![Build Status](https://i.imgur.com/hnitya2.png)](https://i.imgur.com/hnitya2.png)

## Screenshots

Here a couple of screenshot of what is possible to create with this plugin.

[![Build Status](https://i.imgur.com/ZNJDu1R.png)](https://i.imgur.com/ZNJDu1R.png)
[![Build Status](https://i.imgur.com/TJ4RVhD.png)](https://i.imgur.com/TJ4RVhD.png)

## Problems

This plugin is little bit CPU intensive and will add a couple of milliseconds to the frame rendering time. It should work without too much problems on a PC that is also playing Apex Legends, but on weaker hardware it may make the game lag.

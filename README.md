# Apex Legends OBS Game

## Features

 - Automatic recognition of in-game character in all HUDs (in-game, inventory, looting, map view)
 - Possibility to disable/enable sources depending on which HUD is currently displayed
 - Frame accurate character recognition and source activation/deactivation
 - Works with game language set to English or Italiano (with others languages this plugin may not work, but support can be added on request)

## Description

OBS plugin that allows stream customization targeted specifically to Apex Legends game. The plugin is a filter that can be applied on any source or scene.

It allows, for example, to apply a character card overlay in the correct screen position depending on the HUD showed by the game. There are 4 HUDs where the player card is recognized: in-game, inventory, looting box, map view.

The plugin works recognizing elements of the HUD of the game to determine the current HUD showed. Currently it works only if these requirements are met

 - game is running on PC using mouse and keyboard
 - game's language set to either English or Italiano
 - screen's resolution set to 1920x1080

In order to work properly it is required have the following settings:

| Configuration                                   | Value  |
| ----------------------------------------------- | ------ |
| *Settings* → *Gameplay* → *Buttons Hints*       | **On** |
| *Settings* → *Mouse/Keyboard* → *Equip Grenade* | **G**  |
| *Settings* → *Mouse/Keyboard* → *Map*           | **M**  |

## Installation

Compile the plugin or download the latest release available. Place the apex-game.dll in the plugins directory of your OBS installation (`%OBS_INSTALL_FOLDER%\obs-plugins\64bit\apex-game.dll`), open OBS and you're ready to go!

## Configuration

Configuration is pretty straight forward, apply the filter "Apex Game" on the source/scene that contains Apex Legends gameplay. Configure sources in the menu, each source will be activated only when the corresponding HUD condition is showed in the game.

[![Configuration example](https://i.imgur.com/iYMGh8c.png)](https://i.imgur.com/iYMGh8c.png)

## Screenshots

Here a couple of screenshot of what is possible to create with this plugin.

[![Game example 1](https://i.imgur.com/FdHhQc3.png)](https://i.imgur.com/FdHhQc3.png)
[![Game example 2](https://i.imgur.com/Hz0Unwx.png)](https://i.imgur.com/Hz0Unwx.png)

## Problems

This plugin is little bit CPU intensive and will add a couple of milliseconds to the frame rendering time. It should work without too much problems on a PC that is also playing Apex Legends, but on weaker hardware it may make the game lag.

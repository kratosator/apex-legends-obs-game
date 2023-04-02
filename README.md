# Apex Legends OBS Game

## Features

 - Automatic recognition of in-game character in all HUDs (in-game, inventory, looting, map view, spectate)
 - Possibility to disable/enable sources depending on which HUD is currently displayed
 - Frame accurate character recognition and source activation/deactivation
 - Works with game language set to English/Italiano/Chinese Simplified and resolution set to FullHD or 2K

## Description

OBS plugin that allows stream customization targeted specifically to Apex Legends game. The plugin is a filter that can be applied on any source or scene.

It allows, for example, to apply a character card overlay in the correct screen position depending on the HUD showed by the game. There are 5 HUDs where the player card is recognized: in-game, inventory, looting box, map view, spectate.

The plugin works recognizing elements of the HUD of the game to determine the current HUD showed. It works when using mouse and keyboard or a PS4 controller, other input devices may create problems. Currently it works only if these requirements are met:

 - game's language set to either English or Italiano or Chinese Simplified
 - screen's resolution set to 1920x1080 or 2560x1440

Other languages or resolutions may be supported on request.

In order to work properly it is required have the following settings:

| Configuration                                   | Value  |
| ----------------------------------------------- | ------ |
| *Settings* → *Gameplay* → *Buttons Hints*       | **On** |
| *Settings* → *Mouse/Keyboard* → *Equip Grenade* | **G**  |
| *Settings* → *Mouse/Keyboard* → *Map*           | **M**  |

## Installation

Compile the plugin or download the latest release available. Place `apex-game.dll` file in the plugins directory of your OBS installation (`%OBS_INSTALL_FOLDER%\obs-plugins\64bit\apex-game.dll`), open OBS and you're ready to go!

## Configuration

Configuration is pretty straight forward, apply the filter "Apex Game" on the source/scene that contains Apex Legends gameplay. Configure your input device and game's language and set sources in the menu, each source will be activated only when the corresponding HUD condition is showed in the game.

[![Configuration example](https://i.imgur.com/jrXFSvE.png)](https://i.imgur.com/jrXFSvE.png)

## Screenshots

Here a couple of screenshot of what is possible to create with this plugin.

[![Game example 1](https://i.imgur.com/FdHhQc3.png)](https://i.imgur.com/FdHhQc3.png)
[![Game example 2](https://i.imgur.com/Hz0Unwx.png)](https://i.imgur.com/Hz0Unwx.png)

## Problems

This plugin is little bit CPU intensive and will add a couple of milliseconds to the frame rendering time. It should work without too much problems on a PC that is also playing Apex Legends, but on weaker hardware it may make the game lag.
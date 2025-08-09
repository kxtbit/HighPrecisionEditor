# High Precision Editor

This mod changes a few parts of the level editor to allow the use of higher precision numbers than would normally be possible. Specifically, this mod does the following (configurable):

- Overrides the default saving logic for objects in the editor to save each object's position, rotation, warp, and scale at full in-game precision, eliminating the issue in the default game where objects that are positioned very precisely drift out of place by tiny amounts when saving or duplicating them.
- Changes the UI for most triggers to allow decimal values (e.g. the rotation degrees value for the rotate trigger) to be input with an arbitrary amount of decimal places as opposed to being truncated to a set maximum (usually two). The mod also changes the internal logic for saving these values to make sure no precision is lost, as the unmodded game usually truncates these numbers to six or so significant digits.
- Adds a numeric input field for the opacity slider on the alpha and color triggers, allowing exact values to be set with greater precision than normal.
- Changes the X and Y parameters of the move trigger and camera offset trigger to allow decimal values, which are supported in the main game but are normally inaccessible.

Considerations:

- Levels created using this mod are, to my knowledge, perfectly compatible with unmodded games when playing, meaning players will not have to install this mod to play your levels. However, the same is not true for the editor; since Geometry Dash by default re-saves every object when saving a level, even those that have not been modified, the higher precision information will be lost if a level made using this mod is opened and saved in an unmodded game. This means that, if someone copies a level you made using this mod but does not have the mod installed in their own game, they will not be able to edit the level without permanently losing the high precision elements. For most levels, the worst this will do is make a few objects out of position by a pixel or so, but if the level relies on the enhanced precision (for example, in item edit triggers), the level will not work properly.
- Usage of this mod could potentially result in a small hit to performance in the editor, as the modifications to the saving system require an extra step of parsing the normal save strings back in, overwriting certain values, and writing them back out as strings.
- More precise values require a little more space to save, and due to the way floating point to string conversion works, even levels that do not take advantage of high precision may still take up slightly more storage space.

### Keep backups of your levels!
This mod is still in its early stages and it is possible that something was missed in testing. Since this mod messes with the object save system in multiple ways, if something goes wrong, your level could be corrupted. I highly recommend using something like the mod Backups by HJfod to back up your saves and levels before and during use of this mod.
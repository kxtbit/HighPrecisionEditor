# 0.1.14
- Removed some unnecessary debug logs

# 0.1.13
- Fixed a bug where opening the live HSV editing popup would prevent clicks from registering any more in the editor
- Added an internal workaround for a conflict with NinKaz's Editor Utils that would cause mixed values of Follow triggers to be overwritten with -99999 when multi editing

# 0.1.12
- Added support for high precision in the HSV editing widget
- Fixed a bug in the normal game where the value from the Camera Offset move time slider would be saved without proper rounding, configurable by the new option "Miscellaneous Editor Fixes"
- Added experimental support for text inputs in every instance of the HSV widget
- Added an internal workaround for a conflict with NinKaz's Editor Utils that would cause the durations of Camera Offset triggers to be overwritten with -99999 when multi editing

# 0.1.11
- Fixed a few issues with macOS compatibility, contributed by hiimjasmine00
- Fixed a few bugs potentially causing issues with displaying values on macOS when Enable Slider Inputs is enabled but High Precision Trigger Parameters is not
- Added a new setting "Miscellaneous UI Fixes" which rectifies some small UI problems (currently only affects the size of the fade time slider for the alpha trigger, disabled by default)
- Changed the label in the TimeWarp trigger to correctly say "TimeMod" instead of "Opacity" with Enable Slider Inputs
- Decreased the height of the opacity input field in the color trigger with Enable Slider Inputs to be more consistent with the rest of the UI

# 0.1.10
- Fixed a bug that caused some number inputs to affect the wrong sliders
- Fixed a bug where high precision would not work properly on the hold time and fade out values of the pulse trigger

# 0.1.9
- Fixed a bug that caused the game to crash when entering the color picker menu in the particle trigger with Enable Slider Inputs
- Made Enable Slider Inputs compatible with the TimeWarp trigger and the HSV screen

# 0.1.8
- Fixed a bug that caused the hold time and fade out time parameters of the pulse trigger to take on the value of the fade in time when opening the Edit Object menu

# 0.1.7
- Fixed a bug that caused the parameters of some triggers to not save properly on iOS (and presumably macOS).

# 0.1.6
- Fixed a bug that caused levels to be corrupted when saving (this is why you should keep backups)

# 0.1.5
- Fixed an out-of-bounds read
- Cross-platform support, courtesy of hiimjasmine00

# 0.1.4
- Internal code refactoring

# 0.1.3
- Added debugging information to release build

# 0.1.2
- Fixed an issue that would cause the "Follow Group ID" input of the Follow trigger to not save when typed in directly

# 0.1.1
- Changed internal code to match Geode best practices

# 0.1.0
- Initial (unstable!) version
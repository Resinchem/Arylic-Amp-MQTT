### Home Assistant Sample Configurations

The files provided here are my YAML for the deskamp, as I have it built and configured.  It is highly likely that this YAML will need to be edited and cannot simply be copied into your own Home Assistant instance.  _These are provided only as examples for creating your own!_

**Please review the MQTT and UART commands and the Home Assistant Integration sections of the wiki before attempting to use any of this code!**

#### deskamp_package.yaml
This is my package file for all the amp-related entities, automations and scripts.  To use as a package, you must have packages enabled within your Home Assistant instance (see the Home Assistant documentation for more informaition on using packages).  If you are using a split configuration, or have all your configuration in one big configuration.yaml file, you cannot simply copy and paste this code.  It will not work!  Instead, you'll need to split out the various sections (e.g. sensors, automations, etc.) and will likely need to adjust indentations, etc.  My code is primarily meant to serve as an example for creating your own.

Even if using packages, it is highly likely that you will still need to edit the YAML.  If you modified the MQTT topics in the ESP32 code or want your entities to have different names (mine are all 'desk amp'), then you will need to update those in your version of the YAML.

#### dashboard.yaml
The file contains the YAML I used to create this dashboard:

![HA_Dashboard_02](https://user-images.githubusercontent.com/55962781/217309510-49e82392-10da-49a4-83f4-4ee910d7d225.jpg)

Again, it will likely need to be edited to match the entity names you used or created as part of the amp integration.  It is meant only as a guide for creating your own and not a copy/paste resource.  In this particular dashboard, a few custom add-ons are also used (these can be found in the Home Assistant Community Store (HACS):

- ios-dark-mode theme
- custom text-divider row
- custom button card

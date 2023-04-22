## Sirius XM Stream and TFT-Touch Add-ons

The files included here are for optional add-on components and are not required for the original amp build.  While related, each add-on could be implmented individually:

- Creation of local Sirius XM streaming server
- Addition of a TFT-touch panel for controlling the amp

To utilize the Sirius XM streaming server, you must have a valid Sirius XM subscription that includes streaming.  It will not give you access beyond your current subscription and is meant for private, individual non-commercial use.

SiriusXM and all related materials to this service and channels referenced are © 2023 Sirius XM Radio Inc.

The TFT touch panel does require Home Assistant, but the Sirius XM stream server can be implemented with or without Home Assistant.  For more details on the build and implementation, please see the following two sources:

- YouTube Video: [Local SiriusXM Server and Touch Controls for the DIY Amp](https://youtu.be/VQ3LSnCgpeE) (highlights)
- Blog Article: [Adding Local Sirius XM and Touch to the DIY Amp](https://resinchemtech.blogspot.com/2023/04/amp-siriusxm.html) (full details)

The files included here are my versions of how I implemented these add-ons and serve as examples or templates for implementing your own solution.  This means they will likely need to be modified to match your own entity names, reflect your own Sirius XM favorite stations, etc.  The files included are:

### deskamp_package_add_ons.yaml
This are the additional Home Assistant entities, automations and scripts related to use of the Sirius XM and the touch panel.  They are meant to be added to the main amp package, deskamp_package.yaml, found in the /homeassistant folder of this repo and likely will not work standalone.  But they can also server as examples for creating your own Home Assistant automations and scripts.

### ha_dashboard_sirius_subview.yaml
This is the YAML used to create my Sirius XM dashboard as shown in the video and blog article.  Mine is actually defined as a subview, called from a button press on the main amp Home Assistant dashboard.  The YAML for the main amp Home Assistant dashboard can be found in the /homeassistant folder.

### materialdesignicons-webfont.ttf
This is the font file for using material design icons on the touch panel.  It must be installed in your Home Assistant /esphome/fonts folder.  See the blog article linked above for more details on using icons on the touch display.

### sirius_docker_compose.yml
This is the docker-compose file used to create the local Sirius XM streaming server.  Note that the actual app itself is a Python application, so it can be run natively on any system that runs Python.  Docker is not a requirement.  See the [sxm-player repo](https://github.com/AngellusMortis/sxm-player) for more details. Please not I am NOT the author of this repo, so if you have issues or questions regarding the Sirius XM Player, please contact the author of that repo.

### sirius-tft_esphome.yaml
This is my main ESPHome file that gets uploaded to the ESP32 connected to the touch display.  It **_will_** require your own modifications, so this file is included as a starting point for your own implementation.  Note that it requires the header (.h) file and an external library (see below).

### sirius-tft-monitor.h
This is the header file that gets included as part of the main ESPHome YAML file above.  It, among other things, defines the pages and panel layout, panel text, color and fonts, etc.  Again, you will need to modify this for your particular project.

### Additional Files Required
The ESPHome and touch panel also requires the inclusion of an external library developed by Kevin Dorff (Dorffmeister).  It generally does not need to be modified, but simply included as part of the ESPHome.yaml file.  For more information, or to contact the author, please visit his [Github Repository](https://github.com/kdorff/esphome-display-panel).

### All files provided here are for reference only and will likely require modification for use in your own projects.

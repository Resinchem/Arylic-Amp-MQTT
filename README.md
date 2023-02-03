## Adding MQTT (and more) to the Arylic Up2Stream DIY Amplifier
An ESP32-based project that adds MQTT control and more to the Arylic DIY Amp without the need to install **any** mobile applications!

![Finished_Build_Small](https://user-images.githubusercontent.com/55962781/216686591-e848d1ba-e7c6-480c-b4ff-9059607b4078.jpg)

For build instructions, wiring diagrams, parts lists and more, please see the following:
- YouTube Video:
- Blog article: 

The Arylic DIY Up2Stream Amp has convenient interfaces that allow you to add controls such as a volume knob and push buttons for some features.  But many of its features and settings are normally only available via a mobile app (and resultant cloud account).  However, for those local-only minded folks, the amp also offers an HTTP API and  UART intefaces.  By adding an ESP32, this UART can be used to communicate MQTT topics to and from the amp to control nearly all the amp's features.  In addition, the ESP32 adds some addtional features that are either separate purchases or are simply not available otherwise.

![Amp_and_ESP_02](https://user-images.githubusercontent.com/55962781/216692174-f3b8c337-defc-4340-985e-c0c6e9065719.png)

The ESP32 and this project's code adds:

- An MQTT-to-UART translation.  This allows integration into platforms like Home Assistant with many more features and options other than the standard media player integration
- An optional OLED display that shows most state changes to the amp (e.g. source, volume, balance and more)
- Click option to the default rotary encoder that is provided by the amp's encoder interface
- Optional IR receiver that replaces the onboard receiver and allows the use of any IR remote to control the amp.

![HA_Dashboard_01](https://user-images.githubusercontent.com/55962781/216692697-9169711e-2550-4d4c-950d-c2796c9e8901.jpg)

### Please see the wiki for important information on adapting and using this firmware for your own projects

Because the board revisions and firmware versions for the Arylic amps can vary widely, and due to the lack of current 'official' documentation from Arylic on the UART command set, it is highly likely that you will need to adapt the code for your particular install, firmware version, etc.  During development, I found UART commands that were not documented at all... and others that appeared not to work any longer (or had been revised with no documentation).  For these reasons, you should be comfortable with modifying Arduino/C++ code and compiling and uploading that code to your own ESP32 to use this project.  Again, please refer to the wiki for more details.

Because of the above reasons (and other project prioirities), this code is being provided **as-is** to serve as a framework for your own project and build.  I will happily answer questions, but likely will not make any 'fixes', 'upgrades' or 'feature additions' as the current firmware works ideally for my particular amp build and firmware version.

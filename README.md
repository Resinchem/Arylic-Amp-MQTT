## Adding MQTT (and more) to the Arylic Up2Stream DIY Amplifier
An ESP32-based project that adds MQTT control and more to the Arylic DIY Amp

![Finished_Build_Small](https://user-images.githubusercontent.com/55962781/216686591-e848d1ba-e7c6-480c-b4ff-9059607b4078.jpg)

The Arylic DIY Up2Stream Amp has convenient interfaces that allow you to add controls such as a volume knob and push buttons for some features.  But it also offers a UART inteface.  By adding an ESP32, this UART can be used to communicate MQTT topics to and from the amp.  In addition, the ESP32 adds some addtional features that are either separate purchases or are simply not available otherwise.
![Amp_and_ESP_02](https://user-images.githubusercontent.com/55962781/216692174-f3b8c337-defc-4340-985e-c0c6e9065719.png)

The ESP32 and this project's code adds:

- An MQTT-to-UART translation.  This allows integration into platforms like Home Assistant with many more features and options other than the standard media player integration
- An optional OLED display that shows most state changes to the amp (e.g. source, volume, balance and more)
- Click option to the default rotary encoder that is provided by the amp's encoder interface
- Optional IR receiver that replaces the onboard receiver and allows the use of any IR remote to control the amp.

![HA_Dashboard_01](https://user-images.githubusercontent.com/55962781/216692697-9169711e-2550-4d4c-950d-c2796c9e8901.jpg)

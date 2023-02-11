### Determing a Remote's IR Codes

If you want to use your own remote with the custom IR receiver in this project, you will need to know the IR codes sent by the remote.  If you do not know these and don't have another method if determining these, you can easily create a simply IR Decoder to get the code associated with each button press. This should work with most IR remotes.

#### Build the decoder
![IR Get Code Wiring](https://user-images.githubusercontent.com/55962781/217076400-a7302264-0603-4c20-894e-c88e9a5cfcf2.jpg)

Using any ESP8266 or ESP32 dev board (I'm showing a Wemos D1 mini in the above), connect the data pin of an IR receiver to GPIO14 and the +V and ground to the 5V and GND pins on the dev board.  Connect the dev board to your computer via USB cable.

#### Flash the code and read the values
In the Arduino IDE, select the board you are using and the COM port the board is connected to.  Then load the above ir_remote_codes.ino file and flash it to your board. After the sketch loads and the board reboots, open up the serial monitor window.  Press a button on the remote and read the corresponding code in the serial monitor.

![Sample_IR_Output](https://user-images.githubusercontent.com/55962781/217077562-75a6d4ad-c865-452a-bede-da2fccb515b6.jpg)

Once you've determined and recorded the codes for the buttons you want to use with the amp, see the wiki page [Using a Custom IR Receiver and Remote](https://github.com/Resinchem/Arylic-Amp-MQTT/wiki/06-Using-a-Custom-IR-Receiver-and-Remote) for information on how to map these codes to the UART commands for the amp.

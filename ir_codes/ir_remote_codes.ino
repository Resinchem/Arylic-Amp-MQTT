// Use this sketch to capture IR code from a remote
#include <IRremote.h>

const int RECV_PIN = 14;
IRrecv irrecv(RECV_PIN);
decode_results results;

void setup(){
  Serial.begin(115200);
  irrecv.enableIRIn();
  //irrecv.blink13(true);
}

void loop(){
  if (irrecv.decode()){
        Serial.print("IR Code Received: ");
        Serial.println(irrecv.decodedIRData.command);
        delay(250);
        irrecv.resume();
  }
}

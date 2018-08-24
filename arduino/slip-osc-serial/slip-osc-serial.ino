/*
  SLIP-OSC.ino
  listen on USB Serial for slip encoded OSC packet
  to switch an LED on and off
  Depends on [PacketSerial](https://github.com/bakercp/PacketSerial)
  and [OSC](https://github.com/CNMAT/OSC/) libraries.
  Copyright Antoine Villeret - 2015
*/
#include <OSCBundle.h>
#include <PacketSerial.h>


PacketSerial_<SLIP, SLIP::END, 128> serial;

const uint8_t LED_PIN = 13;

void setup(){
  serial.setPacketHandler(&onPacket);
  serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);
}

void loop(){
  
  serial.update();
}

void onPacket(const uint8_t* buffer, size_t size){
  OSCBundle bundle;
  bundle.fill(buffer, size);

  // dispatch the various messages
  if (!bundle.hasError()){
    bundle.dispatch("/home", on_home);
    bundle.dispatch("/stepper", on_stepper);
  }
}

void on_stepper(OSCMessage& msg){

  digitalWrite(LED_PIN, LOW);

  String message = "stepperx:";
  
  if (msg.isInt(0)){
    message += msg.getInt(0);
    // TODO: move stepper x
    
  }
  if (msg.isInt(1)){
     message += "y:";
     message += msg.getInt(1);
    // TODO: move stepper y
  }

  char message_buffer[16];
  message.toCharArray(message_buffer, 16);
  // let the of app know we've received stuff
  serial.send(message_buffer, 16);
}

void on_home(OSCMessage& msg){
  if (msg.isInt(0)){
    digitalWrite(LED_PIN, HIGH);
    serial.send("home", 4);
  }
}


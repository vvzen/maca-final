#include <OSCBundle.h> .   // Copyright Antoine Villeret - 2015
#include <PacketSerial.h>  // library by bakercp - https://github.com/bakercp/PacketSerial

const int PACKET_SERIAL_BUFFER_SIZE = 128;

PacketSerial_<SLIP, SLIP::END, PACKET_SERIAL_BUFFER_SIZE> serial;

void setup(){
  serial.setPacketHandler(&onPacket);
  serial.begin(9600);
}

void loop(){
  
  serial.update();
}


// SERIAL communication
void onPacket(const uint8_t* buffer, size_t size){
  OSCBundle bundle;
  bundle.fill(buffer, size);

  // dispatch the various messages
  if (!bundle.hasError()){
    bundle.dispatch("/home", on_home);
    bundle.dispatch("/stepper", on_stepper);
  }
}

// OSC message handlers
// STEPPERS
void on_stepper(OSCMessage& msg){

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

// HOMING
void on_home(OSCMessage& msg){
  if (msg.isInt(0)){
    // TODO: home motors
    serial.send("home", 4);
  }
}


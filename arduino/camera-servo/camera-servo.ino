// PacketSerial.h library by Christopher Baker <https://christopherbaker.net>
// SPDX-License-Identifier:  MIT
//


#include <PacketSerial.h> // bakercp library for sending packets of data over serial
#include <Servo.h>


SLIPPacketSerial packet_serial;
Servo servo;

const int SERVO_PIN = 9;

void setup(){
  
  // We begin communication with our PacketSerial object by setting the
  // communication speed in bits / second (baud).
  packet_serial.begin(115200);

  // If we want to receive packets, we must specify a packet handler function.
  // The packet handler is a custom function with a signature like the
  // onPacketReceived function below.
  packet_serial.setPacketHandler(&onPacketReceived);

  servo.attach(SERVO_PIN);
  delay(100);
}


void loop(){

  // The PacketSerial::update() method attempts to read in any incoming serial
  // data and emits received and decoded packets via the packet handler
  // function specified by the user in the void setup() function
  packet_serial.update();
}

// This is our handler callback function.
// When an encoded packet is received and decoded, it will be delivered here.
// The `buffer` is a pointer to the decoded byte array. `size` is the number of
// bytes in the `buffer`.
void onPacketReceived(const uint8_t* buffer, size_t b_size){
  
  // Make a temporary buffer.
  uint8_t temp_buffer[b_size];
  uint8_t * ptr;
  ptr = temp_buffer;

  // Copy the packet into our temporary buffer.
  memcpy(temp_buffer, buffer, b_size);

  // check if it's a message for the servo
  if ((char) temp_buffer[0] == 's'){

    // skip first element (the 's')
    // this is done by incrementing the pointer
    ptr++;

    // now convert the char array to an integer (ie: from {'8, '0'} to 80 ) in order to get the angle
    // see https://stackoverflow.com/questions/10204471/convert-char-array-to-a-int-number-in-c
    int angle;
    sscanf(ptr, "%d", &angle);
    

    char message[] = "received angle message";
    packet_serial.send(message, sizeof(message));
    packet_serial.send(ptr, b_size-1);

    servo.write(angle);

    delay(100);
  }
}


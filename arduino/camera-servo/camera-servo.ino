#include <PacketSerial.h>
#include <Servo.h>

PacketSerial packet_serial;
Servo servo;
const int SERVO_PIN = 9;

void setup() {
  
  packet_serial.begin(115200);
  packet_serial.setPacketHandler(&on_packet_received);

  servo.attach(SERVO_PIN);
}

void loop() {
  
  // The PacketSerial::update() method attempts to read in any incoming serial
  // data and emits received and decoded packets via the packet handler
  // function specified by the user in the void setup() function.
  packet_serial.update();
}


// This is our handler callback function.
// When an encoded packet is received and decoded, it will be delivered here.
// The `buffer` is a pointer to the decoded byte array. `size` is the number of
// bytes in the `buffer`.
void on_packet_received(const uint8_t * buff, size_t buff_size){

  // make a temporary buffer
  uint8_t temp_buffer [buff_size];

  // copy the received packet into our buffer
  memcpy(temp_buffer, buff, buff_size);

  //reverse(temp_buffer, buff_size);
  delay(500);

  if (temp_buffer[0] ==  's'){
    manage_servo_command(&temp_buffer, buff_size);
  }

  // received buffer will be like that: 10
  int angle = atoi(buff);
  
  servo.write(angle);

  // print it
//   for (uint8_t i = 0; i < buff_size; i++){
//     packet_serial.send(temp_buffer, buff_size);
//   }
}

// TODO:
void remove_first_element(uint8_t* buffer, size_t size){

  if (buffer[0] == 's'){

    size_t i = 0;
    while (i < size - 1){
      buffer[i] == buffer[i+1];
    }
  }
}

// This function takes a byte buffer and reverses it.
void reverse(uint8_t* buffer, size_t size){

  uint8_t tmp;

  for (size_t i = 0; i < size / 2; i++){
    tmp = buffer[i];
    buffer[i] = buffer[size - i - 1];
    buffer[size - i - 1] = tmp;
  }
}
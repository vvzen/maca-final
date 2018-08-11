#include <PacketSerial.h>

PacketSerial packet_serial;

void setup() {
  
  packet_serial.begin(115200);
  packet_serial.setPacketHandler(&on_packet_received);

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
  delay(1000);

  // print it
  for (uint8_t i = 0; i < buff_size; i++){
    packet_serial.send(temp_buffer, buff_size);
  }
}

// This function takes a byte buffer and reverses it.
void reverse(uint8_t* buffer, size_t size)
{
  uint8_t tmp;

  for (size_t i = 0; i < size / 2; i++)
  {
    tmp = buffer[i];
    buffer[i] = buffer[size - i - 1];
    buffer[size - i - 1] = tmp;
  }
}

#include <PacketSerial.h> // library by bakercp - https://github.com/bakercp/PacketSerial
#include <Wire.h>

// MOTORS
// these arrays contain the values for the three motors.
// motor 1 (y1), motor 2 (y2) and motor 3 (x1) respectively
const int NUM_MOTORS = 2;
int dir_pins[]          = {5, 7};    // direction pins of the motors
int step_pins[]         = {6, 8};    // step pins of the motors
int switch_pins[]       = {2, 3};    // switch pins for each axis

int enable_pins[]       = {9, 10};  // enable pins of the motors

const int STEPS_PER_ROTATION = 200;
const int STEPS_PER_MM = 5; // see compute_linear_distance_from_steps.py

// SERIAL communication
SLIPPacketSerial packet_serial;

void setup() {
  
  for (int i = 0; i < NUM_MOTORS; i++){
    
    // set the switchs as input pullup
    pinMode(switch_pins[i], INPUT_PULLUP);
    
    // set the motors pins as output
    pinMode(dir_pins[i], OUTPUT);
    pinMode(step_pins[i], OUTPUT);
    // DEBUG
    pinMode(enable_pins[i], OUTPUT);

    // set enable to HIGH
    digitalWrite(enable_pins[i], LOW);
  }
  
  // serial
  packet_serial.begin(9600);
  packet_serial.setPacketHandler(&on_packet_received);

  //move_x_motor(500, false);
  //home_motors();
  test_motors();

  delay(1000);
}

void loop() {
  
  // read any incoming serial data (see on_packet_received() )
  //packet_serial.update();
}

// MOTORS MOVEMENT
void move_one_step(int motor_pin){
  digitalWrite(motor_pin, HIGH);
  delayMicroseconds(500);
  digitalWrite(motor_pin, LOW);
  delayMicroseconds(500);
}

void move_x_motor(int amount, bool check){
  for (int i = 0; i < amount * STEPS_PER_MM; i++){
    // if user told us to check the switch, do it
    if (check){
      if (digitalRead(switch_pins[2]) == 0) move_one_step(step_pins[2]);
    }
    else {
      move_one_step(step_pins[2]);
    }
  }
}

void move_y_motors(int amount, bool check){
  for (int i = 0; i < amount * STEPS_PER_MM; i++){
    // if user told us to check the switch, do it
    if (check){
      if (digitalRead(switch_pins[0]) == 0) move_one_step(step_pins[0]);
      if (digitalRead(switch_pins[1]) == 0) move_one_step(step_pins[1]);
    }
    else {
      move_one_step(step_pins[0]);
      move_one_step(step_pins[1]);
    }
  }
}

// for DEBUGGING
void test_motors(){

  for (int i = 0; i < 5; i++){

    // Changes the rotations direction
    digitalWrite(dir_pins[0], LOW);
    digitalWrite(dir_pins[1], LOW);
    
    for(int x = 0; x < 500; x++) {
      
      move_one_step(step_pins[0]);
      move_one_step(step_pins[1]);
    }
    delay(1000);
    
    // Change direction
    digitalWrite(dir_pins[0], HIGH);
    digitalWrite(dir_pins[1], HIGH);
    
    for(int x = 0; x < 500; x++) {
      move_one_step(step_pins[0]);
      move_one_step(step_pins[1]);
    }
    delay(1000);
  }
}

// MOTORS HOMING
void home_motors(){
  
  // start the homing of the motors
  while (true){
      
    // when the switch is true it means we're home  
    int y1_switch_value = digitalRead(switch_pins[0]);
    int y2_switch_value = digitalRead(switch_pins[1]);
    int x_switch_value = digitalRead(switch_pins[2]);

    // move forward first y motor
    if (y1_switch_value == 0) move_one_step(step_pins[0]);

    // move forward second y motor
    if (y2_switch_value == 0) move_one_step(step_pins[1]);

    // move forward x motor
    if (x_switch_value == 0) move_one_step(step_pins[2]);

    // exit condition: when they're all three true
    if (y1_switch_value == true && y2_switch_value == true && x_switch_value == true) break;
  }
  
  // set motors direction down
  digitalWrite(dir_pins[0], LOW);
  digitalWrite(dir_pins[1], LOW);
}

// SERIAL
// When an encoded packet is received and decoded, it will be delivered here.
// The `buffer` is a pointer to the decoded byte array. `size` is the number of
// bytes in the `buffer`.
void on_packet_received(const uint8_t * buff, size_t buff_size){

  // make a temporary buffer
  uint8_t temp_buffer[buff_size];

  // copy the received packet into our buffer
  memcpy(temp_buffer, buff, buff_size);

  // send back same message
  packet_serial.send(temp_buffer, buff_size);

  if (temp_buffer[0] == 'M'){
    handle_move_command(temp_buffer, buff_size);
  }
  if (temp_buffer[0] == 'H'){
    handle_home_command(temp_buffer, buff_size);
  }
  
  //delay(1000);
}

void handle_home_command(const uint8_t * buff, size_t buff_size){
  home_motors();
  packet_serial.send("home", 4);
}

void handle_move_command(const uint8_t * buff, size_t buff_size){

  int x_move, y_move;

  // parse the command
  // sscanf is safe here because we know that the buffer has fixed length
  sscanf(buff, "MX%dY%d", &x_move, &y_move);

  // FIXME:
  // This could definitely be optimized, but for now it works
  // (and premature optimization is the root of all evil)
  char x_move_buffer[3];
  char y_move_buffer[3];
  String x_move_string = String(x_move);
  String y_move_string = String(y_move);
  x_move_string.toCharArray(x_move_buffer, 3);
  y_move_string.toCharArray(y_move_buffer, 3);

  move_y_motors(y_move, true);
  
  packet_serial.send("x move:", 6);
  packet_serial.send(x_move_buffer, 3);
  packet_serial.send("y move:", 6);
  packet_serial.send(y_move_buffer, 3);
}


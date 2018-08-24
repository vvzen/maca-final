#include <OSCBundle.h> .   // Copyright Antoine Villeret - 2015
#include <PacketSerial.h> // library by bakercp - https://github.com/bakercp/PacketSerial
//#include <Wire.h>

// SERIAL communication
const int PACKET_SERIAL_BUFFER_SIZE = 128;
PacketSerial_<SLIP, SLIP::END, PACKET_SERIAL_BUFFER_SIZE> serial;

// MOTORS
// these arrays contain the values for the three motors.
// motor 1 (y1), motor 2 (y2) and motor 3 (x1) respectively
const int NUM_MOTORS = 3;
int dir_pins[]          = {23, 39, 2};    // direction pins of the motors
int step_pins[]         = {27, 43, 4};    // step pins of the motors
int enable_pins[]       = {31, 47};       // enable pins of the motors
int switch_pins[]       = {35, 51, 8};    // switch pins for each axis

// just using some vars for a better readability
const bool RIGHT = false;
const bool LEFT = true;
const bool UP = true;
const bool DOWN = false;

// stores the current position of the motor (x, y)
int current_pos[2];

const int STEPS_PER_ROTATION = 200;
// FIXME: with the current pulley I get a decimal amount of steps per mm, which is not good
const int STEPS_PER_MM = 3; // see compute_linear_distance_from_steps.py
const int STEPS_PER_10MM = 9;
// 9 steps equal 10 mm

void setup() {
  
  for (int i = 0; i < NUM_MOTORS; i++){
    
    // set the switchs as input pullup
    pinMode(switch_pins[i], INPUT_PULLUP);
    
    // set the motors pins as output
    pinMode(dir_pins[i], OUTPUT);
    pinMode(step_pins[i], OUTPUT);
  }

  // set enable pins for y axis to LOW
  pinMode(enable_pins[0], OUTPUT);
  pinMode(enable_pins[1], OUTPUT);

  digitalWrite(enable_pins[0], LOW);
  digitalWrite(enable_pins[1], LOW);
  
  // serial
  serial.setPacketHandler(&on_packet_received);
  serial.begin(9600);

  // set the current pos to a negative number so we know we're not home
  current_pos[0] = -1;
  current_pos[1] = -1;

  move_x_motor(100, LEFT, true);

  move_y_motors(200, UP, true);

  move_x_motor(100, RIGHT, true);

  move_y_motors(200, DOWN, true);

  //move_x_motor(100, false);
  //home_motors();
  //test_motors();
  //move_y_motors(200, false);

  delay(1000);
}

void loop() {
  
  // read any incoming serial data (see on_packet_received() )
  serial.update();
}

// SERIAL communication
void on_packet_received(const uint8_t* buffer, size_t size){
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

  int new_pos[2];


  // X
  if (msg.isInt(0)){
    // compose the message for debugging
    new_pos[0] = msg.getInt(0);
    message += new_pos[0];
    
    // compute difference to get direction
    int required_movement = current_pos[0] - new_pos[0];
    // check direction (if + go right, if - go left)
    bool dir = (required_movement >= 0) ? RIGHT : LEFT;
    // move the motor
    move_x_motor(abs(required_movement), dir, true);
    // update current pos
    current_pos[0] = new_pos[0];
  }
  // Y
  if (msg.isInt(1)){
    message += "y:";
    new_pos[1] = msg.getInt(1);
    message += new_pos[1];
    // compute difference to get direction
    int required_movement = new_pos[1] - current_pos[1];
    // check direction (if + go down, if - go up)
    bool dir = (required_movement >= 0) ? DOWN : UP;
    // move the motor
    move_y_motors(abs(required_movement), dir, true);
    // update current pos
    current_pos[1] = new_pos[1];
  }

  char message_buffer[16];
  message.toCharArray(message_buffer, 16);
  // let the of app know we've received stuff
  serial.send(message_buffer, 16);
}

void on_home(OSCMessage& msg){
  if (msg.isInt(0)){
    serial.send("home", 4);
    home_motors();
  }
}

// MOTORS MOVEMENT
void move_one_step(int motor_pin){
  digitalWrite(motor_pin, HIGH);
  delayMicroseconds(500);
  digitalWrite(motor_pin, LOW);
  delayMicroseconds(500);
}

// MOTORS HOMING
void home_motors(){

  // set motors direction up
  digitalWrite(dir_pins[0], LOW);
  digitalWrite(dir_pins[1], LOW);
  digitalWrite(dir_pins[2], LOW);

  // first home the x
  while (true){
    int x_switch_value = digitalRead(switch_pins[2]);

    // move forward x motor
    if (x_switch_value == 0) move_one_step(step_pins[2]);

    // exit condition
    if (x_switch_value == true) break;
  }
  
  // then put it at the center
  move_x_motor(100, true, false);

  // finally home the other ones
  while (true){
      
    // when the switch is true it means we're home  
    int y1_switch_value = digitalRead(switch_pins[0]);
    int y2_switch_value = digitalRead(switch_pins[1]);

    // move forward first y motor
    if (y1_switch_value == 0) move_one_step(step_pins[0]);

    // move forward second y motor
    if (y2_switch_value == 0) move_one_step(step_pins[1]);

    // exit condition: when they're both three true
    if (y1_switch_value == true && y2_switch_value == true) break;
  }
  
  // set motors direction down
  digitalWrite(dir_pins[0], HIGH);
  digitalWrite(dir_pins[1], HIGH);

  // move y down a little bit
  move_y_motors(50, false, false);

  // save the new current pos
  current_pos[0] = 0;
  current_pos[1] = 0;
}

// @amount --> movement in mm
// @dir    --> true for right, false for left
// @check  --> if you need to check the end stops
void move_x_motor(int amount, bool dir, bool check){

  // change direction
  if (dir){
    digitalWrite(dir_pins[2], HIGH);
  }
  else {
    digitalWrite(dir_pins[2], LOW);
  }

  // move
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

// @amount --> movement in mm
// @dir    --> true for up, false for down
// @check  --> if you need to check the end stops
void move_y_motors(int amount, bool dir, bool check){

  // change direction
  if (dir){
    digitalWrite(dir_pins[0], LOW);
    digitalWrite(dir_pins[1], LOW);
  }
  else {
    digitalWrite(dir_pins[0], HIGH);
    digitalWrite(dir_pins[1], HIGH);
  }

  // move
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

/*
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
  serial.send(temp_buffer, buff_size);

  if (temp_buffer[0] == 'M'){
    handle_move_command(temp_buffer, buff_size);
  }
  if (temp_buffer[0] == 'H'){
    handle_home_command(temp_buffer, buff_size);
  }
  
  //delay(1000);
}
*/

void handle_home_command(const uint8_t * buff, size_t buff_size){
  home_motors();
  serial.send("home", 4);
}

void handle_move_command(const uint8_t * buff, size_t buff_size){

  int x_move = 0;
  int y_move = 0;

  String x_move_string;
  x_move_string[0] = buff[2];
  x_move_string[1] = buff[3];
  x_move_string[2] = buff[4];
  x_move_string[3] = '\0';
  String y_move_string;
  y_move_string[0] = buff[6];
  y_move_string[1] = buff[7];
  y_move_string[2] = buff[8];
  y_move_string[3] = '\0';

  x_move = x_move_string.toInt();
  y_move = y_move_string.toInt();
  
  // parse the command
  // sscanf is safe here because we know that the buffer has fixed length
  //sscanf(buff, "MX%dY%d", &x_move, &y_move);

  // FIXME:
  // This could definitely be optimized, but for now it works
  // (and premature optimization is the root of all evil)

//  String x_move_string = String(x_move);
//  String y_move_string = String(y_move);
  
  //x_move_string.toCharArray(x_move_buffer, 4);
  //y_move_string.toCharArray(y_move_buffer, 4);

  //move_y_motors(y_move, false);
  //move_x_motor(x_move, true);

  char x_move_buffer[4];
  char y_move_buffer[4];
  x_move_string.toCharArray(x_move_buffer, 4);
  y_move_string.toCharArray(y_move_buffer, 4);
  
  serial.send("x move:", 6);
  serial.send(x_move_buffer, 4);
  serial.send("y move:", 6);
  serial.send(y_move_buffer, 4);
}


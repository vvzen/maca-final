#include <OSCBundle.h> .   // Copyright Antoine Villeret - 2015
#include <PacketSerial.h> // library by bakercp - https://github.com/bakercp/PacketSerial
//#include <Wire.h>
#include <Servo.h>

// SERIAL communication
const int PACKET_SERIAL_BUFFER_SIZE = 128;
PacketSerial_<SLIP, SLIP::END, PACKET_SERIAL_BUFFER_SIZE> serial;

// TRIGGER
Servo servo;
const int SERVO_PIN = 12;
const int SERVO_SHOOT_POS = 0;
const int SERVO_HOME_POS = 90;

// MOTORS
// these arrays contain the values for the three motors.
// motor 1 (y1), motor 2 (y2) and motor 3 (x1) respectively
const int NUM_MOTORS = 3;
int dir_pins[]          = {23, 39, 2};    // direction pins of the motors
int step_pins[]         = {27, 43, 4};    // step pins of the motors
int enable_pins[]       = {31, 47, 10};       // enable pins of the motors
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
const int STEPS_PER_MM = 5; // see compute_linear_distance_from_steps.py

void setup() {

  // Initialise servo pin
  servo.attach(SERVO_PIN);
  delay(15);
  servo.write(SERVO_HOME_POS);

  // Initialise motors pins
  for (int i = 0; i < NUM_MOTORS; i++){
    
    // set the switchs as input pullup
    pinMode(switch_pins[i], INPUT_PULLUP);

    // set enable pins to LOW
    pinMode(enable_pins[i], OUTPUT);
    digitalWrite(enable_pins[i], LOW);
    
    // set the motors pins as output
    pinMode(dir_pins[i], OUTPUT);
    pinMode(step_pins[i], OUTPUT);
  }
  
  // serial
  serial.setPacketHandler(&on_packet_received);
  serial.begin(9600);

  // set the current pos to a negative number so we know we're not home
  current_pos[0] = -1;
  current_pos[1] = -1;

  delay(1000);

  move_y_motors(200, UP, false);

  serial.send("ready", 5);
}

void loop() {

  // keep servo at the home position
  servo.write(SERVO_HOME_POS);
  delay(15);
  
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
    bundle.dispatch("/shoot", on_shoot);
  }
}

void debug_switches(){
  String messagex = "switchX:";
  String messagey = "switchYL: ";
  messagey += digitalRead(switch_pins[0]);
  messagey += " R: ";
  messagey += digitalRead(switch_pins[1]);
}

//////////////////////////////////////////
////////// OSC message handlers //////////
//////////////////////////////////////////

// SHOOT
void on_shoot(OSCMessage& msg){
  if (msg.isInt(0)){
    shoot_servo();
    serial.send("shoot", 5);
  }
}
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

//////////////////////////////////////////
///////////// SERVO TRIGGER //////////////
//////////////////////////////////////////
void shoot_servo(){
  // shoot
  for (int i = SERVO_HOME_POS; i < SERVO_SHOOT_POS; i--){
    servo.write(i);
    delay(10);
  }
  // and come back home
  for (int i = SERVO_SHOOT_POS; i < SERVO_HOME_POS; i++){
    servo.write(i);
    delay(10);
  }
}

//////////////////////////////////////////
///////////// MOTORS MOVEMENT ////////////
//////////////////////////////////////////
void move_one_step(int motor_pin){
  digitalWrite(motor_pin, HIGH);
  digitalWrite(motor_pin, LOW);
  delayMicroseconds(1000);
}

//////////////// HOMING ////////////////
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
  move_x_motor(400, true, false);

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
  move_y_motors(50, DOWN, false);

  // save the new current pos
  current_pos[0] = 0;
  current_pos[1] = 0;
}

///////////// MOVE X STEPPER /////////////
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

///////////// MOVE Y STEPPERS ////////////
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


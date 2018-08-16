#include <PacketSerial.h> // library by bakercp - https://github.com/bakercp/PacketSerial
#include <Wire.h>
#include <Adafruit_INA219.h>

// SENSORS
// this is a very handy I2C sensor to check voltage/amperage in the circuit
// it's here for debugging purposes
Adafruit_INA219 ina219;

// MOTORS
// these arrays contain the values for the three motors.
// motor 1 (y1), motor 2 (y2) and motor 3 (x1) respectively
const int NUM_MOTORS = 2;
bool homing_completed[] = {false, false, false}; // are the motors homed?
int dir_pins[]          = {5, 7};    // direction pins of the motors
int step_pins[]         = {6, 8};    // step pins of the motors
int switch_pins[]       = {2, 3};    // switch pins for each axis

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
    
    // set motors direction up
    digitalWrite(dir_pins[i], HIGH);
  }

  // serial
  packet_serial.begin(9600);
  packet_serial.setPacketHandler(&on_packet_received);
  
  ina219.begin();

  // move motors to their 0 position (up)
  home_motors();
}

void loop() {

  // DEBUGGING
  // reverse dir
  digitalWrite(dir_pins[1], LOW);
  move_stepper(step_pins[1], 300, switch_pins[1], false);
  delay(2000);
  digitalWrite(dir_pins[1], HIGH);
  move_stepper(step_pins[1], 300, switch_pins[1], false);
  delay(2000);
  
  // read any incoming serial data (see on_packet_received() )
  packet_serial.update();
}

// MOTORS MOVEMENT
void move_one_step(int motor_pin){
  digitalWrite(motor_pin, HIGH);
  delay(1);               
  digitalWrite(motor_pin, LOW);
  delay(1);
}

// move the stepper of the given amount of mm
void move_stepper(int motor_pin, int amount, int switch_pin, bool check){
  for (int c = 0; c < amount; c++){
    // each mm is 5 steps
    for (int i = 0; i < STEPS_PER_MM; i++){
      // if user told us to check the switch, do it
      if (check){
        if (digitalRead(switch_pin) == 0) move_one_step(motor_pin);
      }
      else {
        move_one_step(motor_pin); 
      }
    }
  }
}

// MOTORS HOMING
void home_motors(){
  
  // start the homing of the motors
  while (true){
      
    // when the switch is true it means we're home  
    int y1_switch_value = digitalRead(switch_pins[0]);
    int y2_switch_value = digitalRead(switch_pins[1]);

    // move forward first motor
    if (y1_switch_value == 0) move_one_step(step_pins[0]);

    // move forward second motor
    if (y2_switch_value == 0) move_one_step(step_pins[1]);

    // exit condition: when they're both true
    if (y1_switch_value == true && y2_switch_value == true) break;
  }
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
  else if (temp_buffer[0] == 'H'){
    handle_home_command(temp_buffer, buff_size);
  }
  
  //delay(1000);
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
  
  packet_serial.send("x move:", 6);
  packet_serial.send(x_move_buffer, 3);
  packet_serial.send("y move:", 6);
  packet_serial.send(y_move_buffer, 3);
}

void handle_home_command(const uint8_t * buff, size_t buff_size){
  home_motors();
  packet_serial.send("home", 4);
}

void log_measurements(){
  
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;
  
  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  
  Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
  Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
  Serial.print("Power:         "); Serial.print(power_mW); Serial.println(" mW");
  Serial.println("");
}


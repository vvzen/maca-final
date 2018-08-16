#include <PacketSerial.h> // library by bakercp - https://github.com/bakercp/PacketSerial
#include <Wire.h>
#include <Adafruit_INA219.h>

// SENSORS
// Declare an instance of INA219 (this sensor is connected via I2C)
// this is a very handy sensor to check voltage/amperage in the circuit
// here for debugging
Adafruit_INA219 ina219;

// MOTORS
// these arrays contain the values for the three motors.
// motor 1 (y1), motor 2 (y2) and motor 3 (x1) respectively
const int NUM_MOTORS = 2;
bool homing_completed[] = {false, false, false}; // are the motors homed?
int dir_pins[]          = {5, 7};    // direction pins of the motors
int step_pins[]         = {6, 8};    // step pins of the motors
int switch_pins[]       = {2, 3};    // switch pins for each axis

// SERIAL communication
SLIPPacketSerial packet_serial;

const int steps_per_rotation = 200;

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

  Serial.begin(9600);
  ina219.begin();

  // move motors to their 0 position (up)
  home_motors();
}

// make one step
void move_one_step(int motor_pin){
  digitalWrite(motor_pin, HIGH);
  delay(1);               
  digitalWrite(motor_pin, LOW);
  delay(1);
}

void loop() {

  // move steppers
  /*
  digitalWrite(Y1_DIR, HIGH);
  digitalWrite(Y1_DIR, HIGH);

  Serial.println("moving forward");
  for (int i = 0; i < steps_per_rotation; i++){
    digitalWrite(Y1_STEP, HIGH);
    delay(45);               
    digitalWrite(Y1_STEP, LOW);
    delay(45);
  }

  digitalWrite(Y1_DIR, LOW);
  Serial.println("moving backward");

  for (int i = 0; i < steps_per_rotation; i++){
    digitalWrite(Y1_STEP, HIGH);
    delay(45);               
    digitalWrite(Y1_STEP, LOW);
    delay(45);
  }

  */
}

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


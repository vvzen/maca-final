#define MAX_MESSAGE_SIZE 10

void setup() {
  Serial.begin(115200);

  delay(100);
  Serial.println(F("--- Ready ---"));
}

void loop() {
  
  while (Serial.available()){

    char message[MAX_MESSAGE_SIZE];

    // fill array with dashes ( - ) so that I can ignore them when I find them
    memset(message, '-', MAX_MESSAGE_SIZE);
    // read the bytes from the serial into the message buffer
    Serial.readBytes(message, MAX_MESSAGE_SIZE);

    int i = 0;
    while (i < MAX_MESSAGE_SIZE){

      if (i == 0){
        switch (message[i]){
          case 'M': {
            handle_move_command(message);
            break;
          }
          case 'P': {
            handle_pick_command(message);
            break;
          }
          case 'S': {
            handle_shoot_command(message);
            break;
          }
        }
      }

      //if (message[i] != '-') Serial.println((char) message[i]); 
      
      i++;
    }
  }

}

void handle_move_command(char command[]){
  Serial.println("Received move command");
}

void handle_pick_command(char command[]){
  Serial.println("Received pick command");
  
  if (command[1] == 'R'){
    Serial.println("picking red ball");  
  }
  else if (command[1] == 'B'){
    Serial.println("picking blue ball");
  }
}

void handle_shoot_command(char command[]){
  Serial.println("Received shoot command");
}


#include <Wire.h>
#include <ZumoShieldN.h>

int route[10] = {0};
char commands[13] = {0};

void print_array(int* array, int len){
  delay(50);
  Serial.print("[");
  for(int i = 0; i < len-1; i++){
    Serial.print(array[i]);
    Serial.print(", ");
  }
  Serial.print(array[len-1]);
  Serial.println("]");
  delay(50);
}

void GetCommand(int route[11]){
  int index = 0;
  char c;
  bool CommandComplete = false;
  
  while (!CommandComplete){
    while(!Serial.available()){}
    c = Serial.read();
    if (c != '\n'){
      Serial.println(c);
    }
    if (index >= 11){
      Serial.println("Command too long");
      delay(500);
      while(Serial.available()){
        Serial.read();
      }
    }
    // Add next vertex into route
    switch (c){
      case '\n':
        break;
      case '.':
        route[index++] = -1;
        CommandComplete = true;
        break;
      case '0':
        route[index++] = 0;
        break;
      case '1':
        route[index++] = 1;
        break;
      case '2':
        route[index++] = 2;
        break;
      case '3':
        route[index++] = 3;
        break;
      case '4':
        route[index++] = 4;
        break;
      case '5':
        route[index++] = 5;
        break;
      case '6':
        route[index++] = 6;
        break;
      case '7':
        route[index++] = 7;
        break;
      case '8':
        route[index++] = 8;
        break;
      case '9':
        route[index++] = 9;
        break;
      case 'a':
        route[index++] = 0xa;
        break;
      case 'b':
        route[index++] = 0xb;
        break;
      default:
        Serial.println("Invalid Character!");
        break;
    }
  }
  
}

void setup() {
  Serial.begin(9600);
  delay(500);
  Serial.println("please input command");
  GetCommand(route);
  print_array(route, 10);
  button.waitForPress();
  delay(100);
  setup_state_machine();
  while(next_command(commands)){
    Serial.println(commands);
  }

}

void loop(){
  state_machine();
}
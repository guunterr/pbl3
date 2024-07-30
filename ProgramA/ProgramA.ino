#include <Wire.h>
#include <ZumoShieldN.h>
#include "Route.h"

int route[11] = {0};
char commands[13] = {0};
bool spin = true;

void print_array(int* array, int len){
  //Pretty print an array of ints
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

int char_to_vertex_number(char c){
  // Convert vertex names as chars to vertex labels
  //Returns -1 for a failed conversion -> invalid character inputted
  switch (c){
    case '0':
      return 0;
      break;
    case '1':
      return 1;
      break;
    case '2':
      return 2;
      break;
    case '3':
      return 3;
      break;
    case '4':
      return 4;
      break;
    case '5':
      return 5;
      break;
    case '6':
      return 6;
      break;
    case '7':
      return 7;
      break;
    case '8':
      return 8;
      break;
    case '9':
      return 9;
      break;
    case 'a':
      return 0xa;
      break;
    case 'b':
      return 0xb;
      break;
    default:
      return -1;
      break;
  }
}

void GetCommand(int route[11]){
  int index = 0;
  char c; char last = 0;
  bool CommandComplete = false;
  
  while (!CommandComplete){
    // Wait for a character in our buffer
    while(!Serial.available()){}
    c = Serial.read();
    if (c != '\n'){
      //Debug print
      Serial.println(c);
    }

    if (index >= 11){
      //Discard overly long inputs
      Serial.println("Command too long");
      delay(500);
      //Consume buffer
      while(Serial.available()){
        Serial.read();
      }
    }
    // Add next vertex into route
    switch (c){
      case '*':
        Serial.println("Deleting Commands");
        index = 0;
        memset(route,0,11);
        break;
      case '\n':
        //Ignore newlines
        break;
      case '.':
        route[index++] = -1;
        CommandComplete = true;
        break;
      default:
        int next_vertex = char_to_vertex_number(c);
        if (next_vertex == -1){
          Serial.println("Can't convert something that isnt 0x0 - 0xb");
          break;
        }
        else if (index != 0){
          //Ignore repeated commands
          if (next_vertex == route[index - 1]){
            Serial.println("Ignoring repeated command");
            break;
          }
        }
        route[index++] = next_vertex;
        break;
    }

  }
}

void setup() {
  Serial.begin(9600);
  delay(500);
  Serial.println("please input command");
  GetCommand(route);
  print_array(route, 11);
  button.waitForPress();
  delay(200);
  setup_state_machine();
  // test_mag_reading();
  spin = true;
}

void loop(){
  state_machine();
}
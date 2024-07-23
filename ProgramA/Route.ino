#include <Wire.h>
#include <ZumoShieldN.h>
#include "Route.h"

direction_t get_commands(int start, int end, direction_t starting_direction, char commands[13]);
char command_from_directions(direction_t current_direction, direction_t new_direction);

int route_index = 0;
int current_direction = UP;

void runme() {
  char commands[13] = { '0' };
  Serial.println("Testing");
  get_commands(1, 0xa, UP, commands);
  Serial.println(commands);
  button.waitForPress();
  delay(100);
}

void xy_from_vertex(int vertex, int* x, int* y) {
  *x = vertex % 4;
  *y = vertex / 4;
}

bool next_command(char commands[13]){
  memset(commands,0,13);
  int current = route[route_index];
  int end = route[route_index+1];
  if(end == -1){
    return false;
  }
  current_direction = get_commands(current, end, current_direction, commands);
  route_index++;
  return true;
}

direction_t get_commands(int start, int end, direction_t starting_direction, char commands[13]) {
  int x= 0; int y= 0; int end_x= 0; int end_y= 0; int i = 0;
  xy_from_vertex(start, &x, &y);
  xy_from_vertex(end, &end_x, &end_y);
  direction_t direction = starting_direction;
  while (x < end_x) {
    x++;
    commands[i++] = command_from_directions(direction, RIGHT);
    direction = RIGHT;
  }
  while (x > end_x) {
    x--;
    commands[i++] = command_from_directions(direction, LEFT);
    direction = LEFT;
  }
  while (y < end_y) {
    y++;
    commands[i++] = command_from_directions(direction, UP);
    direction = UP;
  }
  while (y > end_y) {
    y--;
    commands[i++] = command_from_directions(direction, DOWN);
    direction = DOWN;
  }
  commands[i] = '.';
  return direction;
}

char command_from_directions(direction_t current_direction, direction_t new_direction) {
  switch (current_direction) {
    case UP:
      switch (new_direction) {
        case UP:
          return 'f';
          break;
        case RIGHT:
          return 'r';
          break;
        case DOWN:
          return 'b';
          break;
        case LEFT:
          return 'l';
          break;
      }
      break;
    case RIGHT:
      switch (new_direction) {
        case UP:
          return 'l';
          break;
        case RIGHT:
          return 'f';
          break;
        case DOWN:
          return 'r';
          break;
        case LEFT:
          return 'b';
          break;
      }
      break;
    case DOWN:
      switch (new_direction) {
        case UP:
          return 'b';
          break;
        case RIGHT:
          return 'l';
          break;
        case DOWN:
          return 'f';
          break;
        case LEFT:
          return 'r';
          break;
      }
      break;
    case LEFT:
      switch (new_direction) {
        case UP:
          return 'r';
          break;
        case RIGHT:
          return 'b';
          break;
        case DOWN:
          return 'l';
          break;
        case LEFT:
          return 'f';
          break;
      }
      break;
  }
}

#include <Wire.h>
#include <ZumoShieldN.h>
#include "route.h"

void get_commands(int start, int end, direction_t starting_direction, char commands[13]);
char command_from_directions(direction_t current_direction, direction_t new_direction);


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

void get_commands(int start, int end, direction_t starting_direction, char commands[13]) {
  int x= 0; int y= 0; int end_x= 0; int end_y= 0;
  xy_from_vertex(start, &x, &y);
  int i = 0;
  xy_from_vertex(end, &end_x, &end_y);
  direction_t direction = starting_direction;
  Serial.println(x);
  Serial.println(y);
  Serial.println(end_x);
  Serial.println(end_y);
  while (x < end_x) {
    x++;
    commands[i++] = command_from_directions(direction, RIGHT);
  }
  while (x > end_x) {
    x--;
    commands[i++] = command_from_directions(direction, LEFT);
  }
  while (y < end_y) {
    y++;
    commands[i++] = command_from_directions(direction, UP);
  }
  while (y > end_y) {
    y--;
    commands[i++] = command_from_directions(direction, DOWN);
  }
  commands[i] = '.';
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

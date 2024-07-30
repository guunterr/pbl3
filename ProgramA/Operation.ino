#include <Wire.h>
#include <ZumoShieldN.h>

#define REF_THRESHOLD 400
#define TURN_R_90 1700
#define TURN_L_90 1600
#define ENTERING_SECOND 500

#define ENTERING_SPEED 100

#define TURN_P_MINIMUM_DELAY 20
#define TURN_P_COEFF 2.5
#define TURN_P_OFFSET -5
#define ANGLE_THRESHOLD 2
#define TURN_SPEED 100
#define TURN_ANGLE_READ_TIME 15

#define TRACE_BASE_SPEED 40
#define TRACE_P_COEFFICIENT 10

#define COMMAND_MAX 20

int state;
direction_t dir;
int command_index = 0;
bool DEBUG = true;
bool SHOW_DELTA = false;
bool SHOW_SENSORS = true;
float initial_heading;
float NORTH, EAST, SOUTH, WEST;

// enum direction_state{
//   UP, RIGHT, DOWN, LEFT,
// };

// コンパスのCalibration
void setup_compass() {
  imu.begin();
  imu.configureForCompassHeading();
  delay(200);

  calibrate_compass();
}

// 角度を0から360度に正規化
float normalizeAngle(float angle) {
  while (angle >= 360.0) {
    angle -= 360.0;
  }
  while (angle < 0.0) {
    angle += 360.0;
  }
  return angle;
}

// 角度の違いを入手する関数
float angleDifference(float angle1, float angle2) {
  float diff = fabs(angle1 - angle2);
  if (diff > 180.0) {
    diff = 360.0 - diff;
  }
  return diff;
}


//各センサーの重み　ほかの線を感知するのを防ぐため端は0
int weight[6] = { 0, -10, -10, 10, 10, 0 };

enum robot_state {
  FORWARD,
  BACK,
  REACHED_INTERSECTION,
  SETTLED_INTERSECTION,
  GET_INSTRUCTION,
  ROTATE_L,
  ROTATE_R,
  CONTINUE,
  FINISH,
};

int clip(int var, int bound) {
  if (var > 0) {
    if (abs(var) < bound) {
      return var;
    } else return bound;
  } else {
    if (abs(var) < bound) {
      return var;
    } else return -bound;
  }
}

//センサの値
int get_delta() {
  update_reflectances();
  int delta = 0;
  for (int i = 0; i < 6; i++) {
    int rfl = (int)reflectances.value(i + 1);
    delta += (rfl / 50) * weight[i];
  }
  return clip(delta, 60);
}

void update_reflectances() {
  reflectances.update();
  if (SHOW_SENSORS) {
    for (int i = 1; i < 7; i++) {
      Serial.print(reflectances.value(i));
      Serial.print(" ");
    }
    Serial.print("\n");
  }
}

//交差点に到達したかをboolで返す
bool is_cross() {
  update_reflectances();
  return ((reflectances.value(1) + reflectances.value(2) + reflectances.value(3)) / 3 > REF_THRESHOLD)
         || ((reflectances.value(4) + reflectances.value(5) + reflectances.value(6)) / 3 > REF_THRESHOLD);  // &&から||に変更
}

//前進し、交差点に到達したらstateの状態更新
void forward() {
  int delta = get_delta();
  if (SHOW_DELTA) Serial.println(delta);
  motors.setSpeeds(40 + delta, 40 - delta);

  if (is_cross()) {
    if (DEBUG) {
      Serial.println("Entering reached intersection state");
    }
    state = REACHED_INTERSECTION;
  }
}

//後方に進み、センサーが交差点を感知しなくなると停止。位置の調整用
void back_step() {
  for (int i = 10; i > 0; i--) {
    motors.setSpeeds(-i * 5, -i * 5);
    if (!is_cross()) {
      if (DEBUG) {
        Serial.println("Entering settled intersection state");
      }
      state = SETTLED_INTERSECTION;
      motors.setSpeeds(0, 0);
      break;
    }
  }
  delay(20);
}

//back_step後、交差点の中心まで移動
void settled_intersection() {
  motors.setSpeeds(ENTERING_SPEED, ENTERING_SPEED);
  delay(ENTERING_SECOND);
  motors.setSpeeds(0, 0);
  state = GET_INSTRUCTION;
}

void get_instruction(){
  switch (commands[command_index]) {
    case 'l':
      Serial.println("Rotating Left");
      state = ROTATE_L;
      break;
    case 'r':
      Serial.println("Rotating Right");
      state = ROTATE_R;
      break;
    case 'f':
      Serial.println("Entering forward state");
      state = FORWARD;
      break;
    case 'b':
      Serial.println("Entering back state");
      state = BACK;
      break;
    case '.':
      state = CONTINUE;
      break;
  }
  command_index++;
}

void turn(float angle, int direction){
  //direction = -1 -> left, 1 -> right
  int stage = 0;
  float degree = get_compass_heading(TURN_ANGLE_READ_TIME);
  float start_degree = degree;
  float target_degree = normalizeAngle(start_degree + ((float) direction) * angle);
  float angle_diff = 0;
  Serial.println(start_degree);
  Serial.println(target_degree);
  

  while(stage < 2){
    // motors.setSpeeds(0, 0);
    // delay(200);
    degree = get_compass_heading(TURN_ANGLE_READ_TIME);
    angle_diff = angleDifference(degree, target_degree);
    if (DEBUG){
      Serial.println(degree);
      Serial.println(angle_diff);
    }
    motors.setSpeeds(direction * TURN_SPEED, -direction * TURN_SPEED);
    if (stage == 0 && angle_diff < ANGLE_THRESHOLD) stage++;
    if (stage == 1 && angle_diff > ANGLE_THRESHOLD) break;
    // delay(max(TURN_P_MINIMUM_DELAY, (int) TURN_P_COEFF * angle_diff + TURN_P_OFFSET));
  }
  // motors.setSpeeds(-direction * TURN_SPEED, direction * TURN_SPEED);
  Serial.println(get_compass_heading(TURN_ANGLE_READ_TIME));
  motors.setSpeeds(0, 0);
  Serial.println("Entering forward state");
  state = FORWARD;
}

void setup_state_machine() {
  Serial.println("Initialising state machine");
  setup_compass();
  // 初期化
  button.waitForPress();
  buzzer.playOn();
  next_command(commands);
  Serial.println(commands);
  delay(1000);
  state = FORWARD;
}

void state_machine() {
  switch (state) {
    case FORWARD:
      forward();
      break;
    case REACHED_INTERSECTION:
      back_step();
      break;
    case SETTLED_INTERSECTION:
      settled_intersection();
      break;
    case GET_INSTRUCTION:
      get_instruction();
      break;
    case ROTATE_L:
      turn(90, -1);
      break;
    case ROTATE_R:
      turn(90,1);
      break;
    case BACK:
      turn(180,-1);
      break;
    case CONTINUE:
      if (next_command(commands)) {
        //動き続ける
        Serial.println(commands);
        delay(100);
        state = GET_INSTRUCTION;
        command_index = 0;
      } else {
        // Finish
        buzzer.playOn();
        state = FINISH;
      }
      break;
    case FINISH:
        motors.setSpeeds(0, 0);
        led.on();
        delay(100);
        led.off();
        delay(100);
      break;
  }
}

//前進　→　交差点到達、後退　→　交差点の中心へ前進　→　右回転or左回転→前進

#include <Wire.h>
#include <ZumoShieldN.h>

#define REF_THRESHOLD 400
#define TURN_R_90 1700
#define TURN_L_90 1600
#define ENTERING_SECOND 800

#define TURN_P_COEFF 2.5
#define TURN_P_OFFSET -5

#define COMMAND_MAX 20

int state;
direction_t dir;
int command_index = 0;
bool DEBUG = true;
bool SHOW_DELTA = false;
bool SHOW_SENSORS = false;
float initial_heading;
float NORTH, EAST, SOUTH, WEST;

// enum direction_state{
//   UP, RIGHT, DOWN, LEFT,
// };

// コンパスのCalibration
void setup_compass() {
  imu.begin();
  imu.configureForCompassHeading();
  button.waitForButton();
  Serial.println("starting calibration");

  calibrate_compass();

  button.waitForButton();
  initial_heading = get_compass_heading(100);
  define_direction();
  Serial.print("Initial Heading: ");
  Serial.println(initial_heading);
}

// 方角を定義
void define_direction() {
  NORTH = initial_heading;
  EAST = normalizeAngle(initial_heading + 90);
  SOUTH = normalizeAngle(initial_heading + 180);
  WEST = normalizeAngle(initial_heading + 270);
  Serial.print("NORTH:  ");
  Serial.println(NORTH);
  Serial.print("EAST:  ");
  Serial.println(EAST);
  Serial.print("SOUTH:  ");
  Serial.println(SOUTH);
  Serial.print("WEST:  ");
  Serial.println(WEST);
  delay(100);
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
  reflectances.update();
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
        Serial.println("Entering entering state");
      }
      state = SETTLED_INTERSECTION;
      motors.setSpeeds(0, 0);
      break;
    }
  }
  delay(20);
}

//back_step後、交差点の中心まで移動
void entering() {
  motors.setSpeeds(70, 70);
  delay(ENTERING_SECOND);
  motors.setSpeeds(0, 0);
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
      buzzer.playOn();
      state = FINISH;
      break;
  }
  command_index++;
}

//左に90度回転
void rotate_left_90(direction_t turn_dir) {
  // 例)30(30+360=390)から300に移動
  float goal_degree;
  switch (turn_dir) {
    case UP:
      goal_degree = WEST;
      dir = LEFT;
      break;
    case RIGHT:
      goal_degree = NORTH;
      dir = UP;
      break;
    case DOWN:
      goal_degree = EAST;
      dir = RIGHT;
      break;
    case LEFT:
      goal_degree = SOUTH;
      dir = DOWN;
      break;
  }

  float degree = get_compass_heading(10);  // 現在の角度を取得
  float static_degree = degree;            // 回転始めの角度を覚えておく

  while (1) {
    if (static_degree < 90) {
      motors.setSpeeds(0, 0);
      delay(50);
      degree = get_compass_heading(10);
      float degree_tmp = degree + 360;
      Serial.println(degree);
      if (angleDifference(degree_tmp, goal_degree) < 2) break;
      motors.setSpeeds(-100, 100);
      //P  control needed here
      //delay (20 @ 10 degree delta, 200 @ 90 delta)
      delay(max(20, (int) TURN_P_COEFF * angleDifference(degree, goal_degree) + TURN_P_OFFSET));

    } else {
      motors.setSpeeds(0, 0);
      delay(50);
      degree = get_compass_heading(10);
      Serial.println(degree);
      if (angleDifference(degree, goal_degree) < 2) break;
      motors.setSpeeds(-100, 100);
      delay(max(20, (int) TURN_P_COEFF * angleDifference(degree, goal_degree) + TURN_P_OFFSET));
    }
  }
  motors.setSpeeds(0, 0);
  Serial.println("Entering forward state");
  state = FORWARD;
}

//右に90度回転
void rotate_right_90() {
  float degree = get_compass_heading(10);  // 現在の角度を取得
  float goal_degree;
  switch (dir) {
    case UP:
      goal_degree = EAST;
      dir = RIGHT;
      break;
    case RIGHT:
      goal_degree = SOUTH;
      dir = DOWN;
      break;
    case DOWN:
      goal_degree = WEST;
      dir = LEFT;
      break;
    case LEFT:
      goal_degree = NORTH;
      dir = UP;
      break;
  }

  float static_degree = degree;  // 回転始めの角度を覚えておく

  while (1) {
    if (static_degree > 270) {
      degree = get_compass_heading(10);
      Serial.println(degree);
      float degree_tmp = degree + 360;
      if (angleDifference(degree_tmp, goal_degree) < 2) break;
      motors.setSpeeds(100, -100);
      delay(max(20, (int) TURN_P_COEFF * angleDifference(degree, goal_degree) + TURN_P_OFFSET));
    } else {
      degree = get_compass_heading(10);
      Serial.println(degree);
      if (angleDifference(degree, goal_degree) < 2) break;
      motors.setSpeeds(100, -100);
      delay(max(20, (int) TURN_P_COEFF * angleDifference(degree, goal_degree) + TURN_P_OFFSET));
    }
  }
  motors.setSpeeds(0, 0);
  Serial.println("Entering forward state");
  state = FORWARD;
}

void rotate_back() {
  float degree = get_compass_heading(10);  // 現在の角度を取得
  float goal_degree;
  switch (dir) {
    case UP:
      goal_degree = SOUTH;
      dir = DOWN;
      break;
    case RIGHT:
      goal_degree = WEST;
      dir = LEFT;
      break;
    case DOWN:
      goal_degree = NORTH;
      dir = UP;
      break;
    case LEFT:
      goal_degree = EAST;
      dir = RIGHT;
      break;
  }

  float static_degree = degree;  // 回転始めの角度を覚えておく

  while (1) {
    if (static_degree > 180) {
      degree = get_compass_heading(10);
      Serial.println(degree);
      float degree_tmp = degree + 360;
      if (angleDifference(degree_tmp, goal_degree) < 2) break;
      motors.setSpeeds(100, -100);
      delay(max(20, (int) TURN_P_COEFF * angleDifference(degree, goal_degree) + TURN_P_OFFSET));
    } else {
      degree = get_compass_heading(10);
      Serial.println(degree);
      if (angleDifference(degree, goal_degree) < 2) break;
      motors.setSpeeds(100, -100);
      delay(max(20, (int) TURN_P_COEFF * angleDifference(degree, goal_degree) + TURN_P_OFFSET));
    }
  }
  motors.setSpeeds(0, 0);
  Serial.println("Entering forward state");
  state = FORWARD;
}

void setup_state_machine() {
  Serial.println("start");
  setup_compass();
  delay(2000);
  // 初期化
  state = FORWARD;
  dir = UP;
}

void state_machine() {
  switch (state) {
    case FORWARD:
      forward();
      break;
    case BACK:
      rotate_back();
      break;
    case REACHED_INTERSECTION:
      back_step();
      break;
    case SETTLED_INTERSECTION:
      entering();
      break;
    case ROTATE_L:
      rotate_left_90(dir);
      break;
    case ROTATE_R:
      rotate_right_90();
      break;
    case FINISH:
      if (next_command(commands)) {
        //動き続ける
        state = SETTLED_INTERSECTION;
      } else {
        // Finish
        motors.setSpeeds(0, 0);
        led.on();
        delay(100);
        led.off();
        delay(100);
      }
      break;
  }
}

//前進　→　交差点到達、後退　→　交差点の中心へ前進　→　右回転or左回転→前進

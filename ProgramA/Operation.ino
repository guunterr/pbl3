#include <Wire.h>
#include <ZumoShieldN.h>

#define REF_THRESHOLD  400 
#define TURN_R_90 1700
#define TURN_L_90 1600
#define ENTERING_SECOND 800

#define COMMAND_MAX 20

int  state;
int command_index=0;
bool DEBUG = true;
bool SHOW_DELTA = false;
bool SHOW_SENSORS = false;


//各センサーの重み　ほかの線を感知するのを防ぐため端は0
int weight[6] = {0,-10,-10,10,10,0};

enum robot_state{
  FORWARD, REACHED_INTERSECTION, SETTLED_INTERSECTION,ROTATE_L,ROTATE_R,FINISH
};

int clip(int var, int bound){
  if (var > 0){
    if (abs(var) < bound){
      return var;
      
    } 
    else return bound;
  }
  else{
    if (abs(var) < bound){
      return var;
      
    } 
    else return -bound;
  }
}

//センサの値
int get_delta(){
  update_reflectances();
  int delta = 0;
  for (int i = 0; i<6; i++){
    int rfl = (int) reflectances.value(i+1);
    delta+=(rfl/50)*weight[i];
  }
  return clip(delta, 60);
}

void update_reflectances(){
  reflectances.update();
  if(SHOW_SENSORS){
    for(int i = 1; i < 7; i++){
      Serial.print(reflectances.value(i));
      Serial.print(" ");
    }
    Serial.print("\n");
  }
  
}

//交差点に到達したかをboolで返す
bool is_cross(){
  reflectances.update();
  return ((reflectances.value(1) + reflectances.value(2) + reflectances.value(3))/3> REF_THRESHOLD) && ((reflectances.value(4) + reflectances.value(5) + reflectances.value(6))/3 > REF_THRESHOLD);
}

bool is_line(){
  reflectances.update();
  return (reflectances.value(3) > REF_THRESHOLD) && (reflectances.value(4) > REF_THRESHOLD) && !is_cross();
}




//前進し、交差点に到達したらstateの状態更新
void forward(){
  

  int delta = get_delta();
  if(SHOW_DELTA) Serial.println(delta);
  motors.setSpeeds(40 + delta,40 - delta);
  
  if(is_cross()){
    if(DEBUG){
      Serial.println("Entering reached intersection state");
    }
    state = REACHED_INTERSECTION;
  }
}

//後方に進み、センサーが交差点を感知しなくなると停止。位置の調整用
void back_step(){
  for (int i = 10; i>0;i--){
    motors.setSpeeds(-i*5, -i*5);
    if (!is_cross()){
      if (DEBUG){
        Serial.println("Entering entering state");
      }
      state = SETTLED_INTERSECTION;
      motors.setSpeeds(0,0);
      break;
    }
  }
  delay(20);
}

//back_step後、交差点の中心まで移動
void entering(){
  
  motors.setSpeeds(50,50);
  delay(ENTERING_SECOND);
  motors.setSpeeds(0,0);
  switch (command[command_index]){
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
    case '.':
      buzzer.playOn();
      state = FINISH;
      break;
  }
  command_index++;
}

//左に90度回転
void rotate_left_90(){
  motors.setSpeeds(-100, 100);
  delay(TURN_L_90);
  motors.setSpeeds(0, 0);
  Serial.println("Entering forward state");
  state = FORWARD;
}

void rotate_left_10(){
  motors.setSpeeds(-100, 100);
  delay(TURN_L_90/9);
  motors.setSpeeds(0, 0);
  
}

//右に90度回転
void rotate_right_90(){
  motors.setSpeeds(100, -100);
  delay(TURN_R_90);
  motors.setSpeeds(0, 0);
  Serial.println("Entering forward state");
  state = FORWARD;
}

void rotate_right_10(){
  motors.setSpeeds(100, -100);
  delay(TURN_R_90/9);
  motors.setSpeeds(0, 0);
}


void setup_state_machine() {
  Serial.println("start");
  state = FORWARD;
  delay(2000);
}

void state_machine(){
  switch (state){
    case FORWARD:
      forward();
      break;
    case REACHED_INTERSECTION:
      back_step();
      break;
    case SETTLED_INTERSECTION:
      entering();
      break;
    case ROTATE_L:
      rotate_left_90();
      break;
    case ROTATE_R:
      rotate_right_90();
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


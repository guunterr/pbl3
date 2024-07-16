#include <Wire.h>
#include <ZumoShieldN.h>

char command[21]; // 終端記号の分だけ多く取る
bool CommandComplete = false; // コマンド入力が完了したかのフラグ
int x=0, y=0; // 初期地点
int xd = 0, yd = 1; // 初期の向き(上向き)

void setup() {
  Serial.begin(9600);
  delay(500);
  Serial.println("pleasse input command");
  delay(100);
  // コマンド入力が完了するまでループ
  while(!CommandComplete){
    GetCommand(command);
  }
  delay(1500);
  button.waitForPress();
  buzzer.playOn();
  delay(500);
}

void loop(){
  state_machine();
}

void GetCommand(char *command){
  int index = 0;
  // bool CommandComplete = false; // コマンド入力が完了したかのフラグ

  //入力された文字列の取得を試みる
  while (!CommandComplete){
    if(Serial.available()){
      char c = Serial.read(); // 1文字読み込む
      // '.'を読み込んだら終了
      if (c == '.'){
        command[index++] = '.';
        command[index++] = '\0'; // 終端記号
        CommandComplete = true;
        x=0; y=0; xd=0; yd=1; // 変数を初期化
        break;
      }
      else if(c == '\n') continue; // 改行コードは無視  
      else if (index > 18) { // '.'を除いた数の最大数
        Serial.println("Command exceeds 20 characters");
        delay(500);
        // 残りの入力を廃棄
        while(Serial.available()){
          Serial.read();
        }
        memset(command, 0, 21); // 配列全体を0でリセット
        index=0;
        delay(500); // 入れないと先頭に文字化け
      }
      // 入力できない文字
      else if (c != 'l' 
            && c != 'r'
            && c != 'f'
            && c != '.'){
        Serial.print("invalid command at position: ");
        Serial.println(index);
      }
      else{
        command[index++] = c; // コマンド配列に追加
      }
    }
  }
  
  // コマンド入力が終了後，コース内か確認
  if(isValidCommand(command)){
    // 入力したコマンドを表示
    Serial.print("input= ");
    Serial.println(command);
  }
  else{
    // コース外の場合
    // 入力したコマンドを表示
    Serial.print("input= ");
    Serial.print(command);
    Serial.println(" はコースの範囲外です");

    index = 0;  // インデックスをリセット
    memset(command, 0, 21); // 配列全体を0でリセット
    CommandComplete = false; // もう一度ループさせる
  }
}

bool isValidCommand(char *command){
  int i=0;
  while(command[i] != '.'){
    int tmp;
    switch(command[i]){
        case 'r':
          // 右回転
          tmp = yd;
          yd = -xd;
          xd = tmp;
          break;
        
        case 'l':
          // 左回転
          tmp = xd;
          xd = -yd;
          yd = tmp;
          break;
        
        case 'f':
          // 向きはそのまま
          break;
      }
      x += xd; 
      y += yd;
    // デバック用
    // Serial.print(x);
    // Serial.print(" ");
    // Serial.println(y); 

    // 座標が範囲内あるか確認
    if(!is_Valid_Coordinate(x, y)){
      return false;
    }
    i++; // 次の命令を確認
  }
  return true;
}

bool is_Valid_Coordinate(int x, int y){
  // 現在地が範囲内にあるか確認(Aコース)
  return (0<=x && x<=3 && 0<=y && y<=2);
}
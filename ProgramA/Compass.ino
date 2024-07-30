#include <Wire.h>
#include <ZumoShieldN.h>

#define COMPASS_CALIBRATION_SPEED 120

float get_compass_heading(int trials){
  //1ms/ read, trials = ms taken to read
  ZumoIMU::vector<int32_t> avg = {0, 0, 0};

  for(int i = 0; i < trials; i ++) {
    imu.readMag();
    avg.x += imu.m.x;
    avg.y += imu.m.y;
  }
  avg.x /= (float) trials;
  avg.y /= (float) trials;

  // avg is the average measure of the magnetic vector.
  return heading(avg, imu.m_max, imu.m_min);
}

void calibrate_compass() {
  ZumoIMU::vector<int16_t> running_min = { 32767, 32767, 32767 }, running_max = { -32767, -32767, -32767 };
  Serial.println("Beginning compass calibration");
  float reading_x, reading_y;

  //Calibrate longer and slower
  for (int index = 0; index < 200; index++) {
    motors.setSpeeds(0, 0);
    delay(30);
    // Take a reading of the magnetic vector and store it in compass.m
    reading_x = 0; reading_y = 0;
    for(int i = 0; i < 10; i++){
      imu.readMag();
      reading_x += imu.m.x;
      reading_y += imu.m.y;
    }
    reading_x /= 10;
    reading_y /= 10;
    
    running_min.x = min(running_min.x, reading_x);
    running_min.y = min(running_min.y, reading_y);

    running_max.x = max(running_max.x, reading_x);
    running_max.y = max(running_max.y, reading_y);

    // Serial.println(index);
    motors.setSpeeds(COMPASS_CALIBRATION_SPEED, -COMPASS_CALIBRATION_SPEED);
    delay(50);
    
  }

  motors.setLeftSpeed(0);
  motors.setRightSpeed(0);

  Serial.print("max.x, max.y, min.x, min.y = ");
  Serial.print(running_max.x);
  Serial.print(", ");
  Serial.print(running_max.y);
  Serial.print(", ");
  Serial.print(running_min.x);
  Serial.print(", ");
  Serial.print(running_min.y);
  Serial.println();

  // Store calibrated values in m_max and m_min
  imu.m_max.x = running_max.x;
  imu.m_max.y = running_max.y;
  imu.m_min.x = running_min.x;
  imu.m_min.y = running_min.y;
}
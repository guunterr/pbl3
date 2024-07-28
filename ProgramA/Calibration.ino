#include <Wire.h>
#include <ZumoShieldN.h>

void test_mag_reading(){
  Serial.println("Testing mag reading");
  
  for(int i = 0; i < 1000; i++){
    imu.readMag();
  }
  Serial.println("Done");
}

float get_compass_heading(int trials){
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
  Serial.println("Beginning sensor calibration");

  motors.setLeftSpeed(100);
  motors.setRightSpeed(-100);

  for (int index = 0; index < 1000; index++) {
    // Take a reading of the magnetic vector and store it in compass.m
    imu.readMag();

    running_min.x = min(running_min.x, imu.m.x);
    running_min.y = min(running_min.y, imu.m.y);

    running_max.x = max(running_max.x, imu.m.x);
    running_max.y = max(running_max.y, imu.m.y);

    // Serial.println(index);

    delay(10);
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
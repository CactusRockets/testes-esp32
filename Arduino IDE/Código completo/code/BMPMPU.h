/* CONFIGURAÇÕES BMP e MPU */

#include "Arduino.h"
#include "MPU.cpp"

#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>

#define BMP_ADRESS 0x76
#define MPU_ADRESS 0x68

Adafruit_BMP280 bmp;
MPU6050 mpu(MPU_ADRESS, 0.02, 10);

/* FUNÇÕES BMP280 */

double altitudeAtual = 0;
double altitudeAnterior = 0;

double maximumAltitudeValue = 0;

void setupBMP() {
  if(!bmp.begin(BMP_ADRESS)) {
    Serial.println("BMP not working ...");
    while(1);
  }
  Serial.println("BMP conectado!");

  // ANALISADO
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);
}

String testBMP() {
  float number = bmp.readAltitude(1013);
  Serial.println("---------------------------------");
  Serial.println(String("BMP:") + String(number));
  Serial.println("---------------------------------");

  return String(number);
}

// ANALISADO
void readAltitudeBMP() {
  // Fazer a leitura da altitude
  altitudeAtual = bmp.readAltitude(1013);

  // Atualizar maior altitude encontrada
  if(altitudeAtual > maximumAltitudeValue) {
    maximumAltitudeValue = altitudeAtual;
  }
}

/* FUNÇÕES MPU6050 */

double aceleracaoAtual = 0;
double aceleracaoAnterior = 0;

double velocidadeAtual = 0;
double velocidadeAnterior = 0;

void setupMPU() {
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(100);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.calibrate();
  Serial.println("MPU Calibrado!");
}

String testMPU() {
  
  mpu.update();
  Serial.println("---------------------------------");
  Serial.println("MPU:");
  Serial.println(mpu.accWorldFrameZ);
  Serial.println(mpu.accWorldFrameY);
  Serial.println(mpu.accWorldFrameX);
  Serial.println(mpu.velZ);
  Serial.println(mpu.pitch);
  Serial.println(mpu.roll);
  Serial.println("---------------------------------");

  return String(mpu.accWorldFrameX) + "," + String(mpu.accWorldFrameY) + "," + String(mpu.accWorldFrameZ);
}

// ANALISADO
void readVelocityMPU() {
  mpu.update();
  velocidadeAtual = mpu.velZ;
}

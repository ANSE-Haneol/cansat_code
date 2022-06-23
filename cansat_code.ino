#include <SPI.h>             //SD 카드, 통신과 GPS
#include <SD.h>
#include <Wire.h>
#include <TinyGPS.h>

#include <Adafruit_Sensor.h> //GY-87
#include <Adafruit_BMP280.h>
#include <HMC5883L_Simple.h>
#include "MPU6050.h"

#include <SoftwareSerial.h>  //기본 아두이노와 실시간 운영체제
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>         //세마포어

#define SCL 7     
#define SDA 8   
#define CSB 9   
#define SDO 10    

#define GPSBAUD 9600

SoftwareSerial xbee(17, 16);   

float p0 = 1013.8; 

TinyGPS gps;
String sTime;
float heading;
MPU6050 accelgyro;
HMC5883L_Simple Compass;
float latitude, longitude;
 
Adafruit_BMP280 bmp(CSB, SDA, SDO, SCL);  

const int MPU_ADDR = 0x68;   
int16_t AcX, AcY, AcZ, GyX, GyY, GyZ;   
String sBmpTemp, sBmpPress, sBmpAlt;       
String sLat, sLong;  
          
File myFile;             

String fname = "cansatData";   
String L1;  

int rainPin = A0;
boolean rainCheck = false;
int motorPin1 = 5;
int motorPin2 = 6;

SemaphoreHandle_t xSerialSemaphore;

void controllingMoment(void *pvParameters); //prototypes for scheduling the tasks
void dataCollecting(void *pvParameters);

void setup() 
{
  accelgyro.initialize();
  Serial.begin(9600); 
  Wire.begin();
  p0 = (bmp.readPressure()) / 100;
 
  Serial.println("Process started");
  Serial1.begin(GPSBAUD);
  Serial1.setTimeout(0);
  gps = TinyGPS();

  Serial.println("Starting Xbee...");
  xbee.begin(57600);

  Serial.println("Initializing SD card...");
  if(!SD.begin(SS)) {
    Serial.println(F("Initialization failed, or not present"));
  }
  else {
    Serial.println("initialization done.");
  }
  
  myFile = SD.open(fname, FILE_WRITE);       //파일에 쓰기
  pinMode(led, OUTPUT);
  if (myFile) { 
    Serial.print("Writing to " + fname); 
    myFile.close();
  }
  else {
    Serial.println("An error is occured while opening " + fname);
  }

  PinMode(motorPin1, OUTPUT);               //출력으로 설정
  PinMode(motorPin2, OUTPUT);

  Compass.SetDeclination(23, 35, 'E');      //HMC5338L 지자기 센서 설정
  Compass.SetSamplingMode(COMPASS_SINGLE);
  Compass.SetScale(COMPASS_SCALE_130);
  Compass.SetOrientation(COMPASS_HORIZONTAL_X_NORTH);
  
  if(xSerialSemaphore == NULL) {
    xSerialSemaphore = xSemaphoreCreateMutex();

    if(xSerialSemaphore != NULL) {
      xSemaphoreGive(xSerialSemaphore);
    }
  }

  xTaskCreate(dataCollecting, "Collect", 128, NULL, 1, NULL);
  xTaskCreate(controllingMoment, "Control", 128, NULL, 2, NULL);
}

void loop() 
{

}

void dataCollecting(void *pvParameters__attribute__((unused))) 
{
  for(;;) {
    if(xSemaphoreTake(xSerialSemaphore, (Ticktype_t)5) == pdTRUE) { //임계 구역 접근을 위한 허락 받기
      //process starts
      L1 = "Date: ";
      while(Serial1.available())     
      {
        char c = Serial1.read();    
        if(gps.encode(c))      
        { 
          getGPS(gps); 
        }   
      }
    
      accelgyro.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
      accelgyro.getMotion6(&AcX, &AcY, &AcZ, &GyX, &GyY, &GyZ);
      AcX = AcX / 1024.0;
      AcY = AcY / 1024.0;
      AcZ = AcZ / 1024.0;
      xSemaphoreGive(xSerialSemaphore); //자원을 나누어 줌으로써 다중 프로세싱
    
      getHMC();
      sBmpTemp = String(bmp.readTemperature()); 
      sBmpPress = String(bmp.readPressure() / 100.0); 
      sBmpAlt = String(bmp.readAltitude(p0)); 
    
      if(analogRead(A0) < 500) {
        raincheck = true;
      }
    
      L1 += sTime;
      L1 += "Lat: " + sLat + "°" + "\t\t" + "Temp: " + sBmpTemp + "°C\n";
      L1 += "Long: " + sLong + "°" + "\t\t" + "Press: " + sBmpPress + "hPa\n";
      L1 += "Alt: " + sAlt + " m" + "\t\t" + "bmpAlt: " + sBmpAlt + " m\n";
      L1 += "Rain: " + String(rainCheck) + "\n";
    
      rainCheck = false;
      
      Serial.print(L1); 
      Serial.println();
      xbee.print(L1);
    
      myFile = SD.open(fname, FILE_WRITE);
      if (myFile) { 
        myFile.print(L1); 
        myFile.close(); 
      } 
      else {
        Serial.println("error opening " + fname);
      }
      delay(2000);
    }
  }
}

void controllingMoment(void *pvParameters__attribute__((unused)))
{
  for(;;) {
    if(xSemaphoreTake(xSerialSemaphore, (Ticktype_t)5) == pdTRUE) {
//      if(x, y축 속도와 가속도의 값이 일정 이상) {
//        오른쪽으로 DC모터 회전;
//      }
//      else if(x, y축 속도와 가속도의 값이 일정 이상) {
//        왼쪽으로 DC모터 회전;
//      }
//      else {
//        DC모터 정지;
//      }
      xSemaphoreGive(xSerialSemaphore);
    }
    vTaskDelay(3);
  }
}

void getGPS(TinyGPS &gps)
{
  gps.f_get_position(&latitude, &longitude);
  sLat = String(latitude, 5);
  sLong = String(longitude, 5);
  
  int year;
  byte month, day, hour, minute, second, hundredths;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths);
  sTime += String(year, DEC) + "." + String(month, DEC) + "." + String(day, DEC);
  sTime += " / " + String(hour, DEC) + " : " + String(minute, DEC) + " : " + String(second, DEC) + "\n";
  
  unsigned long chars;
  unsigned short sentences, failed_checksum;
  gps.stats(&chars, &sentences, &failed_checksum);
  delay(10);
} 

void getHMC()
{
  heading = Compass.GetHeadingDegrees();
}

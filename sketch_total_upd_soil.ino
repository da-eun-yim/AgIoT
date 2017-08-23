/* Flame Sensor analog example.
Code by Reichenstein7 (thejamerson.com)

Flame Sensor Ordered from DX ( http://www.dx.com/p/arduino-flame-sensor-for-temperature-
detection-blue-dc-3-3-5v-118075?tc=USD&gclid=CPX6sYCRrMACFZJr7AodewsA-Q#.U_n5jWOrjfU )

To test view the output, point a serial monitor such as Putty at your arduino. 
*/
#include <DHT11.h>
// lowest and highest sensor readings:
const int sensorMin = 0;     // sensor minimum
const int sensorMax = 1024;  // sensor maximum


//analog A0 = humi and temp sensor
//analog A1 = flame sensor
//analog A2 = sunlight sensor
//analog A3 = soil moi

int HTSensor = A0;
int FlameSensor = A1;
int SunSensor = A2;
int SoilSensor = A3;

int soil=0;
int sunSensorValue = 0;
int soilSensorValue = 0;

DHT11 dht11(HTSensor); 
float temp, humi;

void setup() {
  // initialize serial communication @ 9600 baud:
  Serial.begin(9600);  
}
void loop() {
///////////////////////temp and humi//////////////
  if((dht11.read(humi, temp))==0)
  {
    Serial.print("temperature:");
    Serial.print(temp);
    Serial.print(" humidity:");
    Serial.print(humi);
    Serial.println();
  }
  delay(DHT11_RETRY_DELAY); //delay for reread
  
/////////////////////SunSensor//////////////////////////
  // read the sensor from sunlight
  sunSensorValue = analogRead (SunSensor);
  Serial.print("SunLight : ");
  Serial.println (sunSensorValue, DEC);

/////////////////////Flame Sensor///////////////////////
  // read the sensor from flame sensor
  int sensorReading = analogRead(FlameSensor);
  // map the sensor range (four options):
  // ex: 'long int map(long int, long int, long int, long int, long int)'
  int range = map(sensorReading, sensorMin, sensorMax, 0, 3);
  
  // range value:
  switch (range) {
  case 0:    // A fire closer than 1.5 feet away.
    Serial.println("** Close Fire **");
    break;
  case 1:    // A fire between 1-3 feet away.
    Serial.println("** Distant Fire **");
    break;
  case 2:    // No fire detected.
    Serial.println("No Fire");
    break;
  }
  delay(500);  // delay between reads

///////////////////////////Soil Sensor////////////////////////
  int soilSensorValue = analogRead(SoilSensor);
  soilSensorValue = constrain(soilSensorValue, 485, 1023);
  // print out the value you read:
  //Serial.println(soilSensorValue);

  //map the value to a percentage
  soil = map(soilSensorValue, 485, 1023, 100, 0);
  
  // print out the soil water percentage you calculated:
  Serial.print("Soil : ");
  Serial.print(soil);
  delay(500);        // delay in between reads for stability


}

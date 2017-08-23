#include <SPI.h>
#include <RH_RF95.h>
#include <String.h>
#include <DHT11.h>

#define dht_dpin A0 // Use A0 pin as Data pin for DHT11. 
char id = 1;
int SunSensor = A1;
int sunSensorValue = 0;
const int sensorMin = 0;     // sensor minimum
const int sensorMax = 1024;  // sensor maximum
byte bGlobalErr;
char dht_dat[5]; 
float frequency = 915.0 ;
int cnt = 0;

RH_RF95 rf95;
DHT11 dht11(dht_dpin);

void setup()
{
    Serial.begin(9600);
    if (!rf95.init())
        Serial.println("init failed");

    // SETTING !!!

    id = 8; // The first bit of Node ID
    rf95.setFrequency(frequency); // 915Mhz Frequency
    rf95.setSpreadingFactor(7); // SF 7
    rf95.setSignalBandwidth(250E3); // BW 125
    rf95.setCodingRate4(8); // Code Rate 5
    rf95.setTxPower(13); // TxPower 13
    cnt = 0;
    
    Serial.println("Start\n\n"); 
}

byte read_dht_dat()
{
    byte i = 0;
    byte result=0;
    
    for(i=0; i< 8; i++)
    {
        while(digitalRead(dht_dpin)==LOW);//wait 50us
        delayMicroseconds(30);//Check the high level time to see if the data is 0 or 1
        if (digitalRead(dht_dpin)==HIGH)
        result |=(1<<(7-i));
        while (digitalRead(dht_dpin)==HIGH);//Get High, Wait for next data sampleing. 
    }
    return result;
}

uint16_t calcByte(uint16_t crc, uint8_t b)
{
    uint32_t i;
    crc = crc ^ (uint32_t)b << 8;
    
    for ( i = 0; i < 8; i++)
    {
        if ((crc & 0x8000) == 0x8000)
            crc = crc << 1 ^ 0x1021;
        else
            crc = crc << 1;
    }
    return crc & 0xffff;
}

uint16_t CRC16(uint8_t *pBuffer,uint32_t length)
{
    uint16_t wCRC16=0;
    uint32_t i;
    if (( pBuffer==0 )||( length==0 ))
    {
      return 0;
    }
    for ( i = 0; i < length; i++)
    { 
      wCRC16 = calcByte(wCRC16, pBuffer[i]);
    }
    return wCRC16;
}

void loop()
{
    char data[50] = {0};
 
    /////////////////////DHT//////////////////////////
    float temp, humi;
    int err;
    if((err=dht11.read(humi, temp))!=0)
    {
      Serial.println("error");
    }

    delay(500);  // delay between reads
   
    /////////////////////SunSensor//////////////////////////
    // read the sensor from sunlight
    sunSensorValue = analogRead(SunSensor);
    int sun = map(sunSensorValue,0,1024,0,100);
    Serial.print("sun : ");
    Serial.println(sun);
    
    // Use data[0], data[1],data[2] as Node ID
    data[0] = id;
    data[1] = 1;
    data[2] = 1;
    
    data[3] = humi;
    data[4] = temp;
    data[5] = temp; // Soil
    data[6] = sun;
    data[7] = temp; // Flame or Something else.
    cnt ++;
    if(cnt > 255)
        cnt = 1;
    data[8] = cnt; // packet number
    // To make 8 bytes packets, padding it.
   
    Serial.print("Temperature:");
    Serial.println(temp);
    Serial.print("Humidity:");
    Serial.println(humi);
    Serial.print("SunSensor:");
    Serial.println(SunSensor);
    Serial.print("Flame:");
    Serial.println(data[7],DEC);
    Serial.print("Soil:");
    Serial.println(data[5],DEC);
    Serial.print("Packet Number :");
    Serial.println(cnt);
 
    int dataLength = strlen(data); //CRC length for LoRa Data
    uint16_t crcData = CRC16((unsigned char*)data,dataLength);//get CRC DATA
    unsigned char sendBuf[50]={0};
    
    for(int i = 0;i < 9;i++)
    {
        sendBuf[i] = data[i] ;
    }
    
    sendBuf[dataLength] = (unsigned char)crcData; // Add CRC to LoRa Data
    sendBuf[dataLength+1] = (unsigned char)(crcData>>8); // Add CRC to LoRa Data
    rf95.send(sendBuf, strlen((char*)sendBuf));//Send LoRa Data
     
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];//Reply data array
    uint8_t len = sizeof(buf);//reply data length

    if (rf95.waitAvailableTimeout(3000)) // Check If there is reply in 3 seconds.
    {  
        if (rf95.recv(buf, &len)) //check if reply message is correct
       {
            if(buf[0] == id&&buf[1] == 1) // Check if reply message has the our node ID
           {
               pinMode(4, OUTPUT);
               digitalWrite(4, HIGH);
               Serial.print("got reply: ");//print reply
               Serial.println((char*)buf);
               delay(400);
               digitalWrite(4, LOW); 
           }    
        }
        else
        {
           Serial.println("recv failed");
        }
    

    delay(3000); // Send sensor data every 3 seconds
  }
  else{
    Serial.println("No reply ");
  }
      for(int i=0; i<8; i++)
    {
      Serial.print("data[");
      Serial.print(i);
      Serial.print("]= ");
      Serial.print(data[i],BIN);
      Serial.print(" , ");
      Serial.println(data[i],DEC);
    }    
    Serial.print("\n");
}


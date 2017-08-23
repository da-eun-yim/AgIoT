/*
  Upload Data to IoT Server ThingSpeak (https://thingspeak.com/):
  Support Devices: LG01 
  
  Example sketch showing how to get data from remote LoRa node, 
  Then send the value to IoT Server

  It is designed to work with the other sketch dht11_client. 

  modified 24 11 2016
  by Edwin Chen <support@dragino.com>
  Dragino Technology Co., Limited
*/

/*Test dragino*/

#include <SPI.h>
#include <RH_RF95.h>
#include <Console.h>
#include "ThingSpeak.h"
#include "YunClient.h"
YunClient client;
RH_RF95 rf95;

//If you use Dragino IoT Mesh Firmware, uncomment below lines.
//For product: LG01. 
#define BAUDRATE 115200

unsigned long myChannelNumber = 298063;
const char * myWriteAPIKey = "QWF6LFFZF9AD1SA3";
uint16_t crcdata = 0;
uint16_t recCRCData = 0;
float frequency = 915.0;

void setup()
{
    Bridge.begin(BAUDRATE);
    //Console.begin();// Don't use Console here, since it is conflict with the ThinkSpeak library. 

    ThingSpeak.begin(client);
    
    if (!rf95.init())
        //Console.println("init failed");
    ;
    // Setup ISM frequency
    rf95.setFrequency(frequency);
    // Setup Power,dBm
    rf95.setTxPower(13);
    
    //Console.println("Start Listening ");
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

uint16_t CRC16(uint8_t *pBuffer, uint32_t length)
{
    uint16_t wCRC16 = 0;
    uint32_t i;
    if (( pBuffer == 0 ) || ( length == 0 ))
    {
        return 0;
    }
    for ( i = 0; i < length; i++)
    {
        wCRC16 = calcByte(wCRC16, pBuffer[i]);
    }
    return wCRC16;
}

uint16_t recdata( unsigned char* recbuf, int Length)
{
    crcdata = CRC16(recbuf, Length - 2); //Get CRC code
    recCRCData = recbuf[Length - 1]; //Calculate CRC Data
    recCRCData = recCRCData << 8; //
    recCRCData |= recbuf[Length - 2];
}
void loop()
{
    if (rf95.waitAvailableTimeout(2000))// Listen Data from LoRa Node
    {
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];//receive data buffer
        uint8_t len = sizeof(buf);//data buffer length
        if (rf95.recv(buf, &len))//Check if there is incoming data
        {
            recdata( buf, len);
            if(crcdata == recCRCData) //Check if CRC is correct
            { 
                //Console.println("Get Data from LoRa Node");
                if(buf[0] == 1||buf[1] == 1||buf[2] ==1) //Check if the ID match the LoRa Node ID
                {
                    uint8_t data[] = "   Server ACK";//Reply 
                    data[0] = buf[0];
                    data[1] = buf[1];
                    data[2] = buf[2];
                    rf95.send(data, sizeof(data));// Send Reply to LoRa Node
                    rf95.waitPacketSent();
                    int newData[4] = {0, 0, 0, 0}; //Store Sensor Data here
                    
                    //int sh = buf[7];
                    for (int i = 0; i < 4; i++)
                    {
                        newData[i] = buf[i + 3];
                    }
               
                    
                    int h = newData[0];
                    int t = newData[1];
                    int f = newData[2];
                    int s = newData[3];
//                    int sh = newData[4];


            
                    //int sh = newData[4];
//                    int h = buf[3];
//                    int t = buf[4];
//                    int f = buf[5];
//                    int s = buf[6];
//                    int sh = buf[7];
                    
                    ThingSpeak.setField(2,h); // 
                    ThingSpeak.setField(1,t);
                    ThingSpeak.setField(3,f);
                    ThingSpeak.setField(4,s);
                   // ThingSpeak.setField(5,7);
                   ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);   // Send Data to IoT Server.

//                    String tsData = "field1="+String(h)+"&field2="+String(t)+"&field3="+String(f)+"&field4="+String(s); 
 // sprintf(tsData, "field1=%d&field2=%0d&field3=%d&field4=%d&field5=%d", h,t,f,s,sh);
//  
//  client.print("POST /update HTTP/1.1\n");
//  client.print("Host: api.thingspeak.com\n");
//  client.print("Connection: close\n");
//  client.print("X-THINGSPEAKAPIKEY: " + String(myWriteAPIKey) + "\n");
//  client.print("Content-Type: application/x-www-form-urlencoded\n");
//  client.print("Content-Length: ");
//  client.print(tsData.length());
//  client.print("\n\n");
//  client.println(tsData);

                }
            }       
        }
         else
         {
              //Console.println("recv failed");
              ;
          }
     }
     //Console.println(
}


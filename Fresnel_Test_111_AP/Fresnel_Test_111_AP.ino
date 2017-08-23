/*
 * 2017_08_01_PM04:20 uploaded
 * removal of duplicate packet added
 */

#include <FileIO.h>
#include <SPI.h>
#include <RH_RF95.h>
#include <Console.h>
#include "YunClient.h"
#include "HttpClient.h"

#define BAUDRATE 115200

RH_RF95 rf95;

uint16_t crcdata = 0;
uint16_t recCRCData = 0;
float frequency = 915.0;
String null_data = "NULL";
int packetNumForCR5=0;
int packetNumForCR8=0;
int crcCountForCR5=0;
int crcCountForCR8=0;
int receivedPacketCountForCR5=0;
int receivedPacketCountForCR8=0;
int previousPacketNumForCR5=0;
int previousPacketNumForCR8=0;

void setup()
{
    Bridge.begin(BAUDRATE);
    Console.begin();// Don't use Console here, since it is conflict with the ThinkSpeak library. 
    
    if (!rf95.init())
        Console.println("init failed");
        
   /***************ADDITION************************/
   /*******************To DO**********************/
   rf95.setFrequency(frequency);
   rf95.setSignalBandwidth(125E3);
   rf95.setSpreadingFactor(7);
   rf95.setTxPower(13);

   crcCountForCR5=0;
   crcCountForCR8=0;

   Console.println("Start Listening ");
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
    recCRCData = recCRCData << 8; 
    recCRCData |= recbuf[Length - 2];
}

String getTimeStamp() {
  String result;
  Process time;
  // date is a command line utility to get the date and the time 
  // in different formats depending on the additional parameter 
  time.begin("date");
  time.addParameter("+%D-%T");  // parameters: D for the complete date mm/dd/yy
                                //             T for the time hh:mm:ss    
  time.run();  // run the command

  // read the output of the command
  while(time.available()>0) {
    char c = time.read();
    if(c != '\n')
      result += c;
  }
  
  return result;
}

void loop()
{       
    if (rf95.waitAvailableTimeout(3000))// Listen Data from LoRa Node
    {
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];//receive data buffer
        uint8_t len = sizeof(buf);//data buffer length
        uint8_t idBuf[3];
        if (rf95.recv(buf, &len))//Check if there is incoming data
        {
            recdata( buf, len);
            String dataString;
              if(crcdata == recCRCData) //Check if CRC is correct
              {
                      uint8_t data[] = "   Server ACK";//Reply 
                      data[0] = buf[0];
                      data[1] = buf[1];
                      data[2] = buf[2];
                      
                      rf95.send(data, sizeof(data));// Send Reply to LoRa Node
                      rf95.waitPacketSent();
                      int newData[5] = {0, 0, 0, 0, 0}; //Store Sensor Data here
  
                      //receive ID to idBuf
                      for(int i = 0; i < 3; i++)
                      {
                        idBuf[i] = buf[i];
                      }
  
                      //receive Sensor Data to newData
                      for (int i = 0; i < 6; i++)
                      {
                          newData[i] = buf[i + 3];
                      }
  
                      //label and store sensor data,
                      int humi = newData[0];
                      int temp = newData[1];
                      int soil = newData[2];
                      int sun = newData[3];
                      int flame = newData[4];  // flame or something else
                        
                      String id = String(data[0]) + String(data[1]) + String(data[2]);                    
                      String sensorsValues = "";
                      sensorsValues += "Id=";
                      sensorsValues += id;
                      sensorsValues += " Humi=";
                      sensorsValues += String(humi);
                      sensorsValues += " Temp=";
                      sensorsValues += String(temp);
                      sensorsValues += " Sun=";
                      sensorsValues += String(sun);
                      sensorsValues += " Soil=";
                      sensorsValues += String(soil);
                      sensorsValues += " flame=";
                      sensorsValues += String(flame);
                 
                      delay(100);
                     
                      dataString = "SensorsValues: ";
                      dataString += sensorsValues;
  
                      Console.println("in loop!");
  
                      //if packet num is 1, reset receivedPacketCount number
                      if((idBuf[0]==1) && ((buf[8] == 1) || (previousPacketNumForCR5 > buf[8])))
                      {
                        receivedPacketCountForCR5 = 0;
                        crcCountForCR5=0;
                      }
                      // make string for ID x11, coding rate 5
                      if(idBuf[0] == 1)
                      {
                        packetNumForCR5 = buf[8];
                        
                        dataString +=", Date: ";
                        dataString += getTimeStamp();
                        dataString += " Packet Num:";
                        dataString += packetNumForCR5;
                        dataString += ", CRC:  ";
                        dataString += crcCountForCR5;
                        receivedPacketCountForCR5++;
                        dataString += ", receivedPacketCount :";
                        dataString += receivedPacketCountForCR5;
                         /*************RSSI print******************/
                        dataString += ", RSSI: ";
                        dataString += rf95.lastRssi();
  
                        if(buf[8] > 100) //////////////testing PDR calculation automation  
                        {
                          dataString += "\nPDR :";
                          dataString += (String)(((float)receivedPacketCountForCR5/packetNumForCR5)*100);
                          dataString += "%, PCR :";
                          dataString += (String)((crcCountForCR5/packetNumForCR5)*100);
                          dataString += "%";
                        } 
                      }
             
                }
              else 
              {
              //CRC error for coding rate 5
                if(idBuf[0] == 1 && idBuf[1] == 1 && idBuf[2] ==1)
                {
                   packetNumForCR5 = buf[8];
                  
                   Console.println("—CRC ERROR FOR CR5—");
                   crcCountForCR5++; 
                   dataString = "CRC Error! packet corruption";
                   dataString +=", Date: ";
                   dataString += getTimeStamp();
                   dataString += " Packet Num:";
                   dataString += packetNumForCR5;
                   dataString += ", CRC:  ";
                   dataString += crcCountForCR5;
                   dataString += ", receivedPacketCount :";
                   dataString += receivedPacketCountForCR5;     
                   /*************RSSI print******************/
                   dataString += ", RSSI: ";
                   dataString += rf95.lastRssi();             
                }
      
              }
            
              //data received, no error in Packet Number
                if(idBuf[0] == 1)
                {
                  File dataFileFor111 = FileSystem.open("/mnt/data/111_Fresnel_Zone_test.txt", FILE_APPEND);
                   // if the file is available, write to it:
                        if (dataFileFor111) 
                        {
                          dataFileFor111.println(dataString);
                          dataFileFor111.close();
                          Console.println(dataString);
                        }  
                        // if the file isn't open, pop up an error:
                        else 
                        {
                          Console.println("error opening datalog.csv");
                        }

                        previousPacketNumForCR5 = buf[8];
                      
                }
                previousPacketNumForCR8 = buf[8];
          }
         else
         {
              Console.println("recv failed");
         }
     }
}

#include "LOGUARTClass.h"
#include <arduino.h>
#include "consoleNet.h"
#include "commandHandler.h"
#include "kshomeConfig.h"
#include "AmebaFatFS.h"


extern AmebaFatFS fs;
extern deviceConfigurationMgmt confiMgmt;

int consolNet:: consolNetInit()
{
  return udpServer.begin(localPort);  
}

int consolNet:: handleRebootCmd(void){
  uint16_t txDlen = 0;
  Serial.println("Reboot command received");  
  encodeHeader(CMD_Reboot_Ack,txDlen);
  sendResponse(sizeof(PROTOCOL_HEADER) + txDlen);
  return 0;
}
int consolNet:: handleGetConfCmd(void){
  Serial.println("Get conf command received");
  memset(buff, 0, MAX_RX_TX_BUFF_LEN);

  int len = confiMgmt.getConfFile(&buff[4], 508);
  encodeHeader(CMD_Get_Conf_Res, len);
  //append header to the buffer
  memcpy(buff, (const void*) &txHeader, sizeof(PROTOCOL_HEADER));
  sendResponse(len + sizeof(PROTOCOL_HEADER));
  return 0;
}

int consolNet:: handleSetConfCmd(void){
  Serial.println("set Conf command received");
  return 0;
}
// data Frame 2 byte for sequnce ID
int consolNet:: handleDiscoverReq(void){
  int retCode = -1;
  Serial.println("Discover req command received");
  if( rxDataLen == 2 ){
    Serial.print("Next data received : ");
    Serial.println(udpServer.read(buff, 2));

    uint16_t rxSeq =  ( (buff[0] << 8) | buff[1]);
    //if( rxSeq != discoverSeqID){
      discoverSeqID = rxSeq;
      memset(buff, 0, MAX_RX_TX_BUFF_LEN);
      DEVICE_IDENTIFICATION *devIdObj = confiMgmt.devIdObject.getDeviceIdentificationObject();
      //IP Address, DeviceName, deviceCode
      //1 byte len IP add 4 byte
      uint16_t startIndex = sizeof(PROTOCOL_HEADER);
      buff[startIndex++] =04;
      buff[startIndex++] = devIdObj->ip[3];
      buff[startIndex++] = devIdObj->ip[2];
      buff[startIndex++] = devIdObj->ip[1];
      buff[startIndex++] = devIdObj->ip[0];
      //1 byte DevicName len
      uint8_t len = strlen(devIdObj->deviceName);
      buff[startIndex++] = len;
      memcpy((void*) &buff[startIndex], (const void *) devIdObj->deviceName , len);
      startIndex+=len;

      len = strlen(devIdObj->deviceCode);
      buff[startIndex++] = len;
      memcpy((void*) &buff[startIndex], (const void *) devIdObj->deviceCode , len);
      startIndex+=len;

      encodeHeader(CMD_Discover_Res, startIndex);

      sendResponse(startIndex + sizeof(PROTOCOL_HEADER));

      retCode = 0;
   // }
   // else{
   //   Serial.print("received Seq : ");
   //   Serial.println(rxSeq);
   //   Serial.print("available Seq : ");
   //   Serial.println(discoverSeqID);
   //   Serial.println("Duplicate discover request");
    //}
  }
  return retCode;
}

int consolNet:: handleDiscoverResAck(void){
  Serial.println("Discover resAck command received");
  return 0;

}
int consolNet:: sendResponse(uint16_t nBytes){

  Serial.print( "Sending data : ");
  Serial.println(nBytes);

  udpServer.stop();
  udpServer.begin(localPort);

  if (udpServer.beginPacket(remoteIP, remotePort)){   
  //  if (udpServer.beginPacket("192.168.0.155", 62046)){   
                        
    udpServer.write(buff, nBytes);              
    if(udpServer.endPacket()){
      Serial.println("Reply sent successful");
    }else{
      Serial.println("Reply failed to sent successful");
    }
  }   
  return 0;      
}

int consolNet:: recieveHeader(void){
 
  int retCode = -1; 
  uint16_t packetSize; 

  memset((void*) &rxHeader, 0, sizeof(PROTOCOL_HEADER));

  packetSize = udpServer.parsePacket();
  if (packetSize) { 
    // read the packet into packetBufffer
    int len = udpServer.read( (char*) &rxHeader, sizeof(PROTOCOL_HEADER));
    if (len > 0) {
      Serial.print("Received packet of size ");
      Serial.println(packetSize);
      Serial.print("From ");
      remoteIP = udpServer.remoteIP();
      Serial.print(remoteIP);
      Serial.print(", port ");
      remotePort = udpServer.remotePort();
      Serial.println(remotePort);
      retCode = 0;
    } 
  }
  return retCode;
}

uint16_t consolNet:: encodeHeader(uint16_t cmd, uint16_t dLen){
  txHeader.code.Code=cmd;
  txHeader.dataLen.dataLen=dLen;
  // COPY THE HEADER INTO TX BUFF
  memcpy(buff,(const void*)&txHeader,sizeof(PROTOCOL_HEADER));
  return 0;
}

uint16_t consolNet::decodeHeader(){
  rxDataLen = ((rxHeader.dataLen.bLen[0] << 8) | rxHeader.dataLen.bLen[1]);
  rxCmd = ((rxHeader.code.bCode[0] << 8) | rxHeader.code.bCode[1]);
  Serial.print("code : ");
  Serial.print(rxCmd);
  Serial.print("      dataLen : ");
  Serial.println(rxDataLen);
  return 0;
}


void consolNet:: consolNetHandler(void)
{
  if(recieveHeader() == 0){
    
    decodeHeader();
    switch(rxCmd){
      case CMD_Discover_Req:
        handleDiscoverReq();
      break;
      case CMD_Discover_ResAck:
        handleDiscoverResAck();
      break;
      case CMD_Reboot_Req:
        handleRebootCmd();
      break;
      case CMD_Get_Conf_Req:
        handleGetConfCmd();
      break;
      case CMD_Set_Conf_Req:
        handleSetConfCmd();
      break;
      default:
      Serial.println("Wrong Command");
      break;   
    }  

  } 
}

#if 0

        if(int(packetBuffer[0]) == CMD_Get_Conf_Req)
        {            
          

        }
        if(int(packetBuffer[0]) == CMD_Set_Conf_Req)
        {
          //deviceReboot();
          Serial.println("SETCONF command received");
                      
          UdpServer.stop();
          UdpServer.begin(localPort);
          //App_Conf *devConf = getConfig();
          UdpServer.beginPacket(remoteIp, port);            
          //UdpServer.write((char*)devConf, sizeof(App_Conf));
          byte resCode= (byte) CMD_Set_Conf_Res;
          UdpServer.write(resCode);
          UdpServer.write("sucess", 7);
          if(UdpServer.endPacket())
          {
            Serial.println("Reply sent successful");
          }else{
            Serial.println("Reply failed to sent successful");
          }
        }
      }
      
      Serial.println("Contents:");
      Serial.println(packetBuffer);    
  }
 
}



void consolNetHandler(void)
{  
  PROTOCOL_HEADER header; 
  int packetSize = UdpServer.parsePacket();
  if (packetSize) { 
      // read the packet into packetBufffer
      int len = UdpServer.read( (char*) packetBuffer, 255);
      if (len > 0) {
        Serial.print("Received packet of size ");
        Serial.println(packetSize);
        Serial.print("From ");
        IPAddress remoteIp = UdpServer.remoteIP();
        Serial.print(remoteIp);
        Serial.print(", port ");
        uint16_t port = UdpServer.remotePort();
        Serial.println(port);
        packetBuffer[len] = 0;
        if(int(packetBuffer[0]) == CMD_Reboot_Req)
        { 
          //deviceReboot(); 
          //UdpServer.flush(); 
          UdpServer.stop();
          UdpServer.begin(localPort);          
          Serial.println("RESET command received");
          if (UdpServer.beginPacket(remoteIp, port)){   
            UdpServer.write((char)CMD_Reboot_Ack);                      
            UdpServer.write(ReplyBuffer, sizeof(ReplyBuffer));              
            if(UdpServer.endPacket()){
              Serial.println("Reply sent successful");
            }else{
              Serial.println("Reply failed to sent successful");
            }
          }            
        }

        if(int(packetBuffer[0]) == CMD_Get_Conf_Req)
        {            
          Serial.println("GETCONF command received");
          int len = getConfFile(buf, 512);
          UdpServer.stop();
          UdpServer.begin(localPort);
          //App_Conf *devConf = getConfig();
          UdpServer.beginPacket(remoteIp, port);            
          //UdpServer.write((char*)devConf, sizeof(App_Conf));
          byte resCode= (byte) CMD_Get_Conf_Res;
          UdpServer.write(resCode);           
          UdpServer.write(buf, len);
          if(UdpServer.endPacket())
          {
            Serial.println("Reply sent successful");
          }else{
            Serial.println("Reply failed to sent successful");
          }

        }
        if(int(packetBuffer[0]) == CMD_Set_Conf_Req)
        {
          //deviceReboot();
          Serial.println("SETCONF command received");
                      
          UdpServer.stop();
          UdpServer.begin(localPort);
          //App_Conf *devConf = getConfig();
          UdpServer.beginPacket(remoteIp, port);            
          //UdpServer.write((char*)devConf, sizeof(App_Conf));
          byte resCode= (byte) CMD_Set_Conf_Res;
          UdpServer.write(resCode);
          UdpServer.write("sucess", 7);
          if(UdpServer.endPacket())
          {
            Serial.println("Reply sent successful");
          }else{
            Serial.println("Reply failed to sent successful");
          }
        }
      }
      
      Serial.println("Contents:");
      Serial.println(packetBuffer);    
  }
 
}

#endif


//void deviceReboot();




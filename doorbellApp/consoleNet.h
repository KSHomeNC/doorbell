
#ifndef _CONSOL_NET_H___
#define _CONSOL_NET_H___

#include <WiFiUdp.h>
#include "IPAddress.h"
//#include "kshomeConfig.h"

typedef enum{
  CMD_Discover_Req=0,
  CMD_Discover_Res,
  CMD_Discover_ResAck,
  CMD_Reboot_Req,
  CMD_Reboot_Ack,
  CMD_Get_Conf_Req,
  CMD_Get_Conf_Res,
  CMD_Set_Conf_Req,
  CMD_Set_Conf_Res
}DEVICE_MGT_CMD;


#define CONSOL_DEFAULT_PORT 1234
#define MAX_RX_TX_BUFF_LEN 1024

typedef union{
uint16_t Code;
char bCode[2];
}PR_CODE;

typedef union{
uint16_t dataLen;
char bLen[2];
}PR_LEN;

typedef struct{
  PR_CODE code;
  PR_LEN dataLen;  
}PROTOCOL_HEADER;

class consolNet{
  private:
  WiFiUDP udpServer; 
  uint16_t localPort;
  PROTOCOL_HEADER rxHeader;
  PROTOCOL_HEADER txHeader;
  uint16_t rxCmd;
  uint16_t rxDataLen;
  IPAddress remoteIP;
  uint16_t remotePort; 
  uint16_t discoverSeqID; 
  char buff[MAX_RX_TX_BUFF_LEN];
  
  uint16_t decodeHeader( void);
  uint16_t encodeHeader( uint16_t cmd, uint16_t dLen);
  
  int handleRebootCmd(void);
  int handleGetConfCmd(void);
  int handleSetConfCmd(void);
  int handleDiscoverReq(void);  
  int handleDiscoverResAck(void);
  int sendResponse(uint16_t nByte);
  int recieveHeader(void);

  public:  
  consolNet( ){
    localPort = CONSOL_DEFAULT_PORT;
  }
  consolNet( int port){
    localPort = port;
  }
  
  void consolNetHandler(void);
  int consolNetInit(void);
};

#endif //_CONSOL_NET_H___

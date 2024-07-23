#ifndef __KSHOME_CONF_H__
#define __KSHOME_CONF_H__

#include "IPAddress.h"

#define confFilename "config.json"

typedef struct{
 IPAddress ip; 
} WIFiAddress;

typedef struct{
  char dName[64];
  char ssid[32];
  char pass[32];
  char cloudAdd[32];
  int cloudPort;
  char mqttServer[32];
  int mqttPort;  
  bool isMotionDetactionEnable;
}App_Conf;

//read config from local struct
App_Conf* getConfig();
//Write config into local struct
int setConfig(App_Conf *conf);
////read config from file and store into local struct
int loadConfig();
//Read config from local struct and write it into the local buffer
int storeConfig();
//print the configuration structure
void printConf();
// get confile contain in Json object
int getConfFile( char* buffer, int length);
#endif //__KSHOME_CONF_H__

#include "LOGUARTClass.h"
#include "kshomeConfig.h"
#include "AmebaFatFS.h"
#include <arduino.h>
#include <ArduinoJson.h>
extern AmebaFatFS fs;
JsonDocument doc;


int deviceConfigurationMgmt::getConfFile( char* buffer, int length){
  char path[128]; 
  int  len = 0;
  
  //printf("conf File \"%s\"  \r\n", confFilename);
  sprintf(path, "%s%s", fs.getRootPath(), confFilename);
  File file = fs.open(path);
  //Serial.println(file);
  if(file){ 
    if( file.size() > length)
    {
      Serial.println("buffer size is shorter");
      return 0;
    }

    len = file.readBytes(buffer,length);
    
  }//file
  file.close();   
  return len;    
}

int deviceConfigurationMgmt::loadConfig()
{
  char path[128]; 
  int retVal = -1;
  
  printf("conf File \"%s\"  \r\n", confFilename);
  sprintf(path, "%s%s", fs.getRootPath(), confFilename);
  File file = fs.open(path);
  //Serial.println(file);
  if(file){ 
      // Deserialize the JSON document
      DeserializationError error = deserializeJson(doc, file);
      if (error){
        Serial.println(F("deserializeJson() failed:"));    
      }
      else{
        printf("conf File \"%s\"   loaded successful\r\n" , confFilename);
        strlcpy(devConf.dName, doc["deviceName"],sizeof(devConf.dName));
        strlcpy(devConf.dCode, doc["deviceCode"],sizeof(devConf.dCode));
        strlcpy(devConf.ssid, doc["ssid"],sizeof(devConf.ssid));

        strlcpy(devConf.pass, doc["pass"],sizeof(devConf.pass));
        strlcpy(devConf.cloudAdd, doc["cloudAdd"],sizeof(devConf.cloudAdd));    
        devConf.cloudPort = doc["cloudPort"];
        strlcpy(devConf.mqttServer, doc["mqttServer"],sizeof(devConf.mqttServer));    
        devConf.mqttPort = doc["mqttPort"];
        devConf.isMotionDetactionEnable = doc["isMotionDetactionEnable"];  
        devIdObject.setDeviceName(devConf.dName);
        devIdObject.setDeviceCode(devConf.dCode);       
        retVal = 0;
      }      
    //}//len      
  }//file
  file.close();   
  return retVal;    
}

App_Conf* deviceConfigurationMgmt::getConfig(){  
  return &devConf;    
}

int deviceConfigurationMgmt::setConfig(App_Conf *conf)
{  
  strlcpy((char*)&devConf , (const char *) conf, sizeof(devConf));  
  return 0;
}

int deviceConfigurationMgmt::storeConfig(){

  int retVal = -1;
  char path[128]; 
  
  JsonDocument doc;

  doc["deviceName"] = devConf.dName;
  doc["deviceCode"] = devConf.dCode;
  doc["ssid"] = devConf.ssid;
  doc["pass"] = devConf.pass;
  doc["cloudAdd"] = devConf.cloudAdd;    
  doc["cloudPort"] = devConf.cloudPort;
  doc["mqttServer"] = devConf.mqttServer;    
  doc["mqttPort"] = devConf.mqttPort;
  doc["isMotionDetactionEnable"] = devConf.isMotionDetactionEnable;
  
    
  printf("conf File \"%s\"  \r\n", confFilename);
  sprintf(path, "%s%s", fs.getRootPath(), confFilename);
  File file = fs.open(path);
  //Serial.println(file);
  if(file){  
    // Serialize JSON to file
    Serial.println("Serializing the configuration");
    if (serializeJson(doc, file) == 0) {
      Serial.println(F("Failed to write to file"));
    }
    else{
      retVal = 0;
    }
  }
  else{
    Serial.println("file open failed ");
  } 
  file.close();

  return retVal;
}

void deviceConfigurationMgmt::printConf()
{
  Serial.println("Conf read ===>>");
  
  Serial.print("deviceName = ");
  Serial.println(devConf.dName);

  Serial.print("ssid = ");
  Serial.println(devConf.ssid);

  Serial.print("Password = ");
  Serial.println(devConf.pass);

  Serial.print("Cloud Address = ");
  Serial.println(devConf.cloudAdd);

  Serial.print("cloud port = ");
  Serial.println(devConf.cloudPort);

  Serial.print("mqtt Server = ");
  Serial.println(devConf.mqttServer);

  Serial.print("mqtt port = ");
  Serial.println(devConf.mqttPort);

  Serial.print("isMotionDetactionEnable = ");
  Serial.println(devConf.isMotionDetactionEnable);
}


#include "LOGUARTClass.h"
#include "kshomeConfig.h"
#include "AmebaFatFS.h"
#include <arduino.h>
#include <ArduinoJson.h>
extern AmebaFatFS fs;
JsonDocument doc;
App_Conf devConf;

int loadConfig()
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
        strlcpy(devConf.ssid, doc["ssid"],sizeof(devConf.ssid));

        strlcpy(devConf.pass, doc["pass"],sizeof(devConf.pass));
        strlcpy(devConf.cloudAdd, doc["cloudAdd"],sizeof(devConf.cloudAdd));    
        devConf.cloudPort = doc["cloudPort"];
        strlcpy(devConf.mqttServer, doc["mqttServer"],sizeof(devConf.mqttServer));    
        devConf.mqttPort = doc["mqttPort"];         
        retVal = 0;
      }      
    //}//len      
  }//file
  file.close(); 
  
  return retVal;    

}
App_Conf* getConfig(){
  
  return &devConf;    
}
int setConfig(App_Conf *conf)
{  
  strlcpy((char*)&devConf , (const char *) conf, sizeof(devConf));  
  return 0;
}

int storeConfig(){

  int retVal = -1;
  char path[128]; 
  
  JsonDocument doc;

  doc["deviceName"] = devConf.dName;
  doc["ssid"] = devConf.ssid;
  doc["pass"] = devConf.pass;
  doc["cloudAdd"] = devConf.cloudAdd;    
  doc["cloudPort"] = devConf.cloudPort;
  doc["mqttServer"] = devConf.mqttServer;    
  doc["mqttPort"] = devConf.mqttPort;
    
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

void printConf()
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
}


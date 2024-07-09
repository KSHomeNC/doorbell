#include "ff.h"
#include "LOGUARTClass.h"
#include "AmebaFatFS.h"
//#include "setDateNTime.h"
#include "cloudService.h"
#include <arduino.h>
#include "commandHandler.h"
//#include "kshomeConfig.h"

char* cmd;
char* arg1;
char* arg2;
char cmdBuf[128]= " ";
extern AmebaFatFS fs;
//extern kshomeDnT datenTime;

extern void stopAudioAndVideo();
extern void startAudioAndVideo();

void parseCommand(String cmdStr)
{
  cmdStr.trim();
  if(cmdStr.length() == 0)
    return;
    
  cmdStr.toCharArray(cmdBuf,128);
  char *tmp = strtok(cmdBuf, " ");
  Serial.print("Cmd String:  ");
  Serial.println(tmp);
  cmd = tmp;
  Serial.print("Cmd :  ");
  Serial.println(tmp);
  if(tmp != NULL){
    tmp = strtok(NULL, " ");
    arg1 = tmp;
    Serial.print("Arg1 :  ");
    Serial.println(tmp);
  }
  if(tmp != NULL){
    tmp = strtok(NULL, " ");
    arg2 = tmp;
    Serial.print("Arg2 :  ");
    Serial.println(tmp);
  }
  if(tmp != NULL){
   Serial.println("Invalid number of argument");
  }
}

void listDir(char* path)
{
  FRESULT res = FR_OK;
  FILINFO fileinfo;
  DIR dir;

  char *filename;
  unsigned int filelen;
  int bufidx = 0;
  if( fs.getRootPath() == NULL) {
    Serial.println(" Drive is not mounted");
    return;
  }

  res = f_opendir(&dir, path);
  if (res != FR_OK) {
    Serial.println(" Provided path doesn't found");
    return;
  }
  while (1) {
    // call f_readdir repeatedly until all files in directory have been processed.
    res = f_readdir(&dir, &fileinfo);
    if ((res != FR_OK) || (fileinfo.fname[0] == 0)) {
        break;
    }

    filename = fileinfo.fname;
    filelen = strlen(filename);      

    if (*filename == '.' || (filename[0] == '.' && filename[1] == '.')) {
        continue;
    }
    Serial.println(filename);
  }     
}

void cleanDir(char* path)
{
  FRESULT res = FR_OK;
  FILINFO fileinfo;
  DIR dir;
  char dirPath[128];

  char *filename;
  unsigned int filelen;
  int bufidx = 0;
  if( fs.getRootPath() == NULL) {
    Serial.println(" Drive is not mounted");
    return;
  }

  res = f_opendir(&dir, path);
  if (res != FR_OK) {
    Serial.println(" Provided path doesn't found");
    return;
  }
  while (1) {
    // call f_readdir repeatedly until all files in directory have been processed.
    res = f_readdir(&dir, &fileinfo);
    if ((res != FR_OK) || (fileinfo.fname[0] == 0)) {
        break;
    }

    filename = fileinfo.fname;
    filelen = strlen(filename);      

    if (*filename == '.' || (filename[0] == '.' && filename[1] == '.')) {
        continue;
    }
    sprintf(dirPath,"%s%s%s",path,"/",filename);
    f_rmdir(dirPath);
    Serial.println(dirPath);
  } 
}

void checkSerialCommand()
{  
  char tmpPath[64]="";
  char buf[5120];
  char *p;
  if(Serial.available())
  {
    String cmdString = Serial.readString();    
    parseCommand(cmdString);
   
    if(String (cmd) == "dir")
    { 
      
      if(arg1 !=NULL){
        sprintf(tmpPath,"%s%s%s",fs.getRootPath(),"/",arg1);
      }else{
        sprintf(tmpPath,"%s",fs.getRootPath());
      }      
      listDir(tmpPath);              
    }
    else if(String (cmd) == "dirClean")
    {     
      if(arg1 !=NULL){
        sprintf(tmpPath,"%s%s%s",fs.getRootPath(),"/",arg1);
      }else{
        sprintf(tmpPath,"%s",fs.getRootPath());
      }      
      cleanDir(tmpPath);
    }
    else if(String (cmd) == "backup"){
      char buf[512];
      char *p;  
      //stopAudioAndVideo();
      if (arg1 == NULL){      
        /* list root directory and put results in buf */
        memset(buf, 0, sizeof(buf));
        fs.readDir(fs.getRootPath(), buf, sizeof(buf));
  
        /* the filenames are separated with '\0', so we scan one by one */
        p = buf;
        while (strlen(p) > 0) { // send only one valid file at time
          p += strlen(p) + 1;
          String fName = String(p);
          if(fName.endsWith("mp4")){  
            Serial.println(p);                 
            //sendFileToCloud(p, &fs); 
            break;               
          }
        }
      } 
      else{
       // sendFileToCloud(arg1, &fs);              
      }
      //startAudioAndVideo();
    }
    else if(String (cmd) == "rm")
    {
      Serial.println(arg1);
      if( arg1 != NULL){
        fs.remove(arg1);  
      }
    }
    
    #if 0
    else if(String (cmd) == "NTPSync")
    {
      Serial.println("Device synching with NTP server ");
      //digitalWrite(GREEN_LED, HIGH);
      datenTime.synchRTCtoNtpServer();
      //delay(2000);      
      Serial.println(datenTime.getDateNTimeStr());
      //digitalWrite(GREEN_LED, LOW);
    }
    else if(String (cmd) == "getDnT"){      
      Serial.println(datenTime.getDateNTimeStr());
    } 
    #endif

    else if(String (cmd) == "getCnf"){
      #if 0
      App_Conf conf;
    
      //readConfig(&conf);
      Serial.println(conf.ssid);
      Serial.println(conf.pass);
      Serial.println(conf.cloudAdd);
      Serial.println(conf.cloudPort);
      Serial.println(conf.motionCount);
      Serial.println(conf.silenceTimeSec);
      Serial.println(conf.burzer);            
      #endif
      Serial.println("Not Impemented");            
    }
    else{
      Serial.println("Wrong command");
    }
  }
}

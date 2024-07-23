/*  This example uses the camera to capture JPEG images at regular intervals,
    and saves the images to SD Card.

 Example guide:
 https://www.amebaiot.com/en/amebapro2-arduino-video-jpeg-sdcard/
*/
#include "VideoStream.h"
#include "AmebaFatFS.h"
#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <commandHandler.h>
#include "MotionDetection.h"
#include "StreamIO.h"
#include "VideoStream.h"
#include <PubSubClient.h>
#include "kshomeConfig.h"
#include "liveCamera.h"
#include "bellButton.h"
#include "consoleNet.h"

#define CHANNEL 0
#define CHANNELMD  3  // RGB format video for motion detection only avaliable on channel 3
#define INTERVAL 10
// Pin Definition
#define GREEN_LED LED_G
#define BUZZER_PIN 8

App_Conf *conf;


char filename[32] = "";

WiFiUDP ntpUDP;

WIFiAddress ipAdd;
// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "sg.pool.ntp.org", 28800, 60000);

// Use a pre-defined resolution, or choose to configure your own resolution
VideoSetting config(VIDEO_FHD, CAM_FPS, VIDEO_JPEG, 1);
VideoSetting configMD(VIDEO_VGA, 10, VIDEO_RGB, 0);     // Low resolution RGB video for motion detection

uint32_t img_addr = 0;
uint32_t img_len = 0;
char path[128];

bool motionDetected = false;
int motionCount = 0;
int silenceTimeSec = 1000; // devide by of 10 mSec  1000 is 10Sec
int timelapseSec = silenceTimeSec;

bool isConsolNetStarted = false;

char clientId[]       = "amebaClient";
char publishTopic[]   = "objMotion";
char publishPayload[] = "Motion Detected on  ";
char subscribeTopic[] = "inTopic";
char pubtButton[] = "doorbell Pressed:==>> ";

MotionDetection MD;
StreamIO videoStreamerMD(1, 1); // 1 Input RGB Video -> 1 Output MD

AmebaFatFS fs;
extern int LIVECAMERA_PORT;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

consolNet consolnet;

void motionDetection(void);
void saveImageonSDCard(void);
void mqttInit(void);
void mqttReconnect(void);
void sendMsg(int mType);

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)(payload[i]));
    }
    Serial.println();
}

void sendMsg(int msgType){
  int rTry =0;
  char* topic;
  do{
    mqttReconnect();  
    rTry++;
  }while(!(client.connected()) && (rTry<=3) ); 

  if(client.connected()){
    char payLoad[64];
    if(msgType == 0 )// motion detected
    {
      sprintf(payLoad,"%s%s%s%s%d",publishPayload,"http://",ipAdd.ip.get_address(),":", LIVECAMERA_PORT);
      topic = "objMotion";
    }
    else
    {
      sprintf(payLoad,"%s%s%s%s%d",pubtButton,"http://",ipAdd.ip.get_address(),":", LIVECAMERA_PORT);
      topic = "button";
    }
    
    client.publish(topic, payLoad);
  }
  delay(1000);
  //once message sent close the connection for next message
  if(client.connected()){
    client.disconnect();
  }

}
void mqttReconnect(void){
   // Loop until we're reconnected
    while (!(client.connected())) {
        Serial.print("\r\nAttempting MQTT connection...");
        // Attempt to connect
        if (client.connect(clientId)) {
            Serial.println("connected");
            //Once connected, publish an announcement and resubscribe
            //client.publish(publishTopic, publishPayload);
            client.subscribe(subscribeTopic);
        } else {
            Serial.println("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            //Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void mqttInit(void){
  client.setServer(conf->mqttServer, conf->mqttPort);
  client.setCallback(mqttCallback);
}

void doorbellCameraInit()
{
  Camera.configVideoChannel(CHANNEL, config);
  Camera.configVideoChannel(CHANNELMD, configMD);
  Camera.videoInit();

  // Configure motion detection for low resolution RGB video stream
  MD.configVideo(configMD);
  MD.begin();
  MD.setDetectionMask(mask);

  // Configure StreamIO object to stream data from low res video channel to motion detection
  videoStreamerMD.registerInput(Camera.getStream(CHANNELMD));
  videoStreamerMD.setStackSize();
  videoStreamerMD.registerOutput(MD);
  if (videoStreamerMD.begin() != 0) {
    Serial.println("StreamIO link start failed");
  }
    
  // start video capture camera
  Camera.channelBegin(CHANNEL);

  // Start data stream from low resolution video channel
  Camera.channelBegin(CHANNELMD);
}

void setup() {
  Serial.begin(115200);
    // GPIO Initialization
  pinMode(GREEN_LED, OUTPUT);
  digitalWrite(GREEN_LED, true);
  // initialize the file system
  fs.begin();
  delay(1000);
  // load the device configuration
  loadConfig();
  //read the device configuration
  conf= getConfig();
  printConf();

  WiFi.begin(conf->ssid, conf->pass);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  // print your WiFi IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");

  ipAdd.ip = WiFi.localIP();
  timeClient.begin();

  doorbellCameraInit();

  initLiveCamera();

  Serial.println("Camera started!");
  delay(2000);

  initButton();
  Serial.println("BUtton started!");
  delay(2000);
  mqttInit();
  digitalWrite(GREEN_LED, false);

  if (consolnet.consolNetInit())
  {
    Serial.println("ConsoleNet Started");
    isConsolNetStarted = true;
  }
  else{
    Serial.println("ConsoleNet failed to Started");
  }
  
}

int cnt =0;
void loop() {

    checkSerialCommand();

    //motionDetection();
    //handleLiveCamera();
    if(isButtonPressed())
    {
      sendMsg(1);
    }
    if(isConsolNetStarted == true){
      consolnet.consolNetHandler();
    }
    //mqttTask();
    /*
    printConf();
    delay(INTERVAL);
    cnt++;
    if(cnt==3){
      App_Conf newConf;
      strlcpy(newConf.dName, "3580Lovage driver door", sizeof(newConf.dName));
      setConfig(&newConf);
      delay(5000);
      Serial.println("storing the new configuration");
      storeConfig();

      delay(5000);

      Serial.println("loading the new configuration");
      loadConfig();
    }
    */
  
    delay(INTERVAL);
}

void saveImageonSDCard(void)
{
  char dirName[32] = "";
  char dirPath[32] = "";
  timeClient.update();
  uint16_t year = (uint16_t)timeClient.getYear();
  uint16_t month = (uint16_t)timeClient.getMonth();
  uint16_t date = (uint16_t)timeClient.getMonthDay();
  uint16_t hour = (uint16_t)timeClient.getHours();
  uint16_t minute = (uint16_t)timeClient.getMinutes();
  uint16_t second = (uint16_t)timeClient.getSeconds();

  sprintf(dirName,"%d%d%d",month, date, year);
  sprintf(filename,"%d%d%d" ,hour ,minute, second);
  sprintf(dirPath, "%s%s", fs.getRootPath(), dirName);
  
  if(!fs.isDir(dirPath)) {
    fs.mkdir(dirPath);  
    Serial.print( "New Dir created  ==:");  
    Serial.println(dirPath);
  }  
  sprintf(path, "%s%s%s%s", dirPath, "/", filename, ".jpg");  
  Serial.print("File path =");
  Serial.println(path);
  File file = fs.open(path);
  delay(100);
  Camera.getImage(CHANNEL, &img_addr, &img_len);
  file.write((uint8_t *)img_addr, img_len);
  delay(100);
  printf("Saved %s\r\n", file.name());
  file.close();
  fs.setLastModTime(path, year, month, date, hour, minute, second);
 
}

void motionDetection(void)
{
  
   // Motion detection results is expressed as an array
    // With 0 or 1 in each element indicating presence of motion
    // Iterate through all elements to check for motion
    // and calculate largest rectangle containing motion
    if (motionDetected == false){
      std::vector<MotionDetectionResult> md_results = MD.getResult();
        
      if (MD.getResultCount() > 0) {
          digitalWrite(GREEN_LED, HIGH);
          motionCount++;
          Serial.print("Motion detection : ");
          Serial.println(motionCount);
          if(motionCount==3){  
            // tone(BUZZER_PIN, 1000, 500); 
            saveImageonSDCard();
            sendMsg(0);           
            int captureTimeSec = 4;
            while(captureTimeSec>0){
              saveImageonSDCard();
              captureTimeSec--;
              delay(500);// every Second
            }                      
            motionDetected = true;
            digitalWrite(GREEN_LED, HIGH);
            timelapseSec = silenceTimeSec;
            motionCount=0;
          }
      } 
      else{
        motionCount = 0;
        digitalWrite(GREEN_LED, LOW); // GREEN LED turn off when no motion detected
      }
    }
    else{
         timelapseSec--;
         if ( timelapseSec == 0)
         {
            digitalWrite(GREEN_LED, LOW); // GREEN LED turn off when no motion detected 
            motionDetected = false;
         }
    }   

}


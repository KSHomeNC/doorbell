#include "LOGUARTClass.h"
#include "WString.h"
#include "liveCamera.h"
#include "WiFi.h"
#include "videoStream.h"
#include "AudioStream.h"
#include "AudioEncoder.h"
#include "StreamIO.h"

#define CHANNEL 1
int LIVECAMERA_PORT = 80;

//VideoSetting configLive(VIDEO_FHD, CAM_FPS, VIDEO_JPEG, 1); // for full capture
VideoSetting configLive(1024, 576, CAM_FPS, VIDEO_JPEG, 1); //1024
WiFiServer server(LIVECAMERA_PORT,TCP_MODE, NON_BLOCKING_MODE);
//WiFiServer server(LIVECAMERA_PORT);
AudioSetting AudioConfig(0);
Audio audio;
AAC aac;

StreamIO audioStreamer(1,1);// audio input =>> Encoder output
StreamIO audioVidepMixer(2,1);// 1 encoder output & Video ouput

uint32_t liveimg_addr = 0;
uint32_t liveimg_len = 0;

#define PART_BOUNDARY "123456789000000000000987654321"
char* STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
char* IMG_HEADER = "Content-Type: image/jpeg\r\nContent-Length: %lu\r\n\r\n";

void sendHeader(WiFiClient& client) {
    client.print("HTTP/1.1 200 OK\r\nContent-type: multipart/x-mixed-replace; boundary=");
    client.println(PART_BOUNDARY);
    client.print("Transfer-Encoding: chunked\r\n");
    client.print("\r\n");
}

void sendChunk(WiFiClient& client, uint8_t* buf, uint32_t len) {
    uint8_t chunk_buf[64] = {0};
    uint8_t chunk_len = snprintf((char*)chunk_buf, 64, "%lX\r\n", len);
    client.write(chunk_buf, chunk_len);
    client.write(buf, len);
    client.print("\r\n");
}


int initLiveCamera()
{
  Camera.configVideoChannel(CHANNEL, configLive);
  Camera.videoInit();
  
  audio.configAudio(AudioConfig);
  audio.begin();

  aac.configAudio(AudioConfig);
  aac.begin();

  audioStreamer.registerInput(audio);
  audioStreamer.registerOutput(aac);
  if(audioStreamer.begin()!=0){
    Serial.println("Audio Streamer link failed");
  }




  Camera.channelBegin(CHANNEL);

  server.begin();

  return 0;
  
}


void handleLiveCamera()
{
  WiFiClient client = server.available();
  client.setTimeout(5000);

    if (client) {       
        while (client.connected()) {
          //Serial.println("new client connected");
          String currentLine = "";
          if (client.available()) {
              char c = client.read();
              Serial.write(c);
              if (c == '\n') {
                Serial.println(" live streaming started");
                  if (currentLine.length() == 0) {
                      sendHeader(client);
                      while (client.connected()) {
                          Camera.getImage(CHANNEL, &liveimg_addr, &liveimg_len);
                          uint8_t chunk_buf[64] = {0};
                          uint8_t chunk_len = snprintf((char*)chunk_buf, 64, IMG_HEADER, liveimg_len);
                          sendChunk(client, chunk_buf, chunk_len);
                          sendChunk(client, (uint8_t*)liveimg_addr, liveimg_len);
                          sendChunk(client, (uint8_t*)STREAM_BOUNDARY, strlen(STREAM_BOUNDARY));
                          delay(5);   // Increase this delay for higher resolutions to get a more consistent, but lower frame rate
                      }
                      Serial.println("connection closed from Client");
                      client.stop();
                      break;
                  } else {
                      currentLine = "";
                  }
              } else if (c != '\r') {
                  currentLine += c;
              }
          }
        }        
        client.stop();
        //Serial.println("client disonnected");
    } else {
        Serial.println("waiting for client connection");
        delay(1000);
    }

}

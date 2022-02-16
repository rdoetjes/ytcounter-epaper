/*
YT Counter ePaper
*/


#define LILYGO_T5_V213
#define uS_TO_S_FACTOR 1000000 

#include <boards.h>
#include <GxEPD.h>
#include <GxDEPG0213BN/GxDEPG0213BN.h>    // 2.13" b/w  form DKE GROUP
#include <U8g2_for_Adafruit_GFX.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

static HTTPClient http;
static int httpError;

static const char *id = "your channle id";
static const char *key = "your yt api key"; 
static const char *ssid = "you ssid";
static const char *wifipass = "your wifi password";

static WiFiUDP ntpUDP;
static NTPClient timeClient(ntpUDP);

struct Stats {
  int viewCount;
  int subCount;
  int videoCount;
};

static Stats myStats;

static const char* rootca="-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"-----END CERTIFICATE-----\n";

String getDateTime(NTPClient *ntp){
  ntp->begin();
  ntp->setTimeOffset(3600);
  
  while(!ntp->update()) {
    ntp->forceUpdate();
  }

  String dateTime = ntp->getFormattedDate();
  dateTime.replace("T", " ");
  dateTime.replace("Z", "");
  return dateTime;
}

void displayValue(U8G2_FOR_ADAFRUIT_GFX *screen, const char *text, int value, int x, int y){
  u8g2Fonts.setCursor(x, y);
  String print = String(text);
  print+=String(value);
  u8g2Fonts.print(print);
}

void displayValue(U8G2_FOR_ADAFRUIT_GFX *screen, const char *text, float value, int x, int y){
  u8g2Fonts.setCursor(x, y);
  String print = String(text);
  print+=String(value);
  u8g2Fonts.print(print);
}

void getNrSubscriptions(HTTPClient *http, int *httpError, Stats *stats, const char* id, const char *key, const char *rootca){
  
  static int count;
  String request = "https://www.googleapis.com/youtube/v3/channels?part=statistics&id="+String(id)+"&key="+String(key);
  http->setTimeout(15000);
  http->begin(request.c_str(), rootca);
  (*httpError) = http->GET();
  
  if ((*httpError) > 0){ 
    StaticJsonDocument<1024> doc;

    String res = http->getString();
    DeserializationError error = deserializeJson(doc, res.c_str());

    if (!error) {
       stats->subCount = doc["items"][0]["statistics"]["subscriberCount"].as<int>();
       stats->viewCount = doc["items"][0]["statistics"]["viewCount"].as<int>();
       stats->videoCount = doc["items"][0]["statistics"]["videoCount"].as<int>();
    }
  }
}

void print_wakeup_reason()
{
  u8g2Fonts.setFontMode(1);                           // use u8g2 transparent mode (this is default)
  u8g2Fonts.setFontDirection(1);                      // left to right (this is default)
  u8g2Fonts.setForegroundColor(GxEPD_BLACK);          // apply Adafruit GFX color
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);          // apply Adafruit GFX color
  u8g2Fonts.setFont(u8g2_font_VCR_OSD_tu);            // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
  display.fillScreen(GxEPD_WHITE);
  u8g2Fonts.setCursor(50, 20);
  u8g2Fonts.print("QUERING API...");   
  display.update();
 
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, wifipass);
  }

  //Wait for Wifi to be reconnected
  int retry=0;
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    retry++;
    if (retry>20) esp_deep_sleep_start();
  }  
  
  getNrSubscriptions(&http, &httpError, &myStats, id, key, rootca);

  display.fillScreen(GxEPD_WHITE);
  
  u8g2Fonts.setCursor(100, 50);
  u8g2Fonts.print("LAST UPDATE:");
  
  u8g2Fonts.setCursor(80,10);
  u8g2Fonts.print(getDateTime(&timeClient));
  
  displayValue(&u8g2Fonts, "SUBS  : ", myStats.subCount, 55, 40);
  displayValue(&u8g2Fonts, "VIEWS : ", myStats.viewCount, 35, 40);
  displayValue(&u8g2Fonts, "VIDEOS: ", myStats.videoCount, 15, 40);
  //Draw thick border
  for(int i=0; i<5; i++) display.drawRect(i, i, display.width()-(i*2), display.height()-(i*2), GxEPD_BLACK);
  
  display.update();
    
}

void setup(void)
{
    SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
    display.init(); // enable diagnostic output on Serial
    u8g2Fonts.begin(display);    
    
    esp_sleep_enable_timer_wakeup(600 * uS_TO_S_FACTOR);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)39, LOW);
    print_wakeup_reason();
    esp_deep_sleep_start();
}

void loop(){

}

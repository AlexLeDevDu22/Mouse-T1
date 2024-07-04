#include "esp_camera.h"
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_http_server.h"
#include "SPIFFS.h"
#include <ArduinoJson.h>
#include <time.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <iostream>
#include <vector>
#include <string>
#include <String.h>
#include <sstream>
#include <driver/adc.h>

// Commands
#define button1 14
#define button2 12
#define SDA_PIN 13
#define SCL_PIN 15

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, SCL_PIN, SDA_PIN);

const char* ssid = "Galaxy A20e de Alex";
const char* password = "Alexdu22";


#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define MIN(a, b) ((a) < (b) ? (a) : (b)) // Define the MIN macro

httpd_handle_t server = NULL;

camera_fb_t *captured_image = NULL; // Variable globale pour stocker l'image capturée
String currentTime = ""; // Variable globale pour stocker l'heure actuelle

void capture_image() {
  if (captured_image != NULL) {
    esp_camera_fb_return(captured_image); // Libère l'ancienne image si elle existe
  }

  captured_image = esp_camera_fb_get(); // Capture une nouvelle image
  if (!captured_image) {
    Serial.println("Camera capture failed");
    return;
  }

  // Affiche la taille de l'image capturée
  Serial.print("Captured image size: ");
  Serial.println(captured_image->len);
}

static esp_err_t capture_handler(httpd_req_t *req) {
  if (captured_image == NULL) {
    Serial.println("No image captured");
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_send(req, (const char *)captured_image->buf, captured_image->len);

  return ESP_OK;
}

DynamicJsonDocument jsonRequest(200);


// Fonction pour mettre à jour l'heure locale
void updateLocalTime(void *pvParameters) {
    while (true) {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) {
        currentTime = "Failed to obtain time";
        } else {
        char buffer[64];
        struct timeval tv;
        gettimeofday(&tv, NULL);
        snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d.%03ld", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, tv.tv_usec / 1000);
        currentTime = String(buffer);
        }
        delay(1);  // Met à jour toutes les millisecondes
    }
}

static esp_err_t json_get_handler(httpd_req_t *req) {  
    if (!jsonRequest.containsKey("type")){
        jsonRequest["type"] = "no request";
    }else{
        jsonRequest["post url"] = "http://" + WiFi.localIP().toString() + "/json-post"; // request info
    }
  String jsonStr;
  serializeJson(jsonRequest, jsonStr);
  httpd_resp_set_type(req, "application/json");
  httpd_resp_send(req, jsonStr.c_str(), jsonStr.length());
  return ESP_OK;
}

// Get Android response
String lastResponseString = "";
String lastResponseDid = "";
DynamicJsonDocument lastResponseDict(200);
#include <vector>
std::vector<String> chunks;
int expectedChunks = -1;
String pageName="menu";
std::vector<String> OverviewMenuList={"{\"text\":\"chatGPT asking\",\"redirect\":\"askingType(chatGPT asking)\"}",
                                    "{\"text\":\"lessons\",\"redirect\":\"overview matiere\"}",
                                    "{\"text\":\"chatGPT history\",\"redirect\":\"askingType(chatGPT history)\"}"
                                    };
std::vector<String> overviewMatieresList={"{\"text\":\"back to menu\"}",
                                        "{\"text\":\"Français\"}",
                                        "{\"text\":\"Histoire\"}",
                                        "{\"text\":\"Géographie\"}",
                                        "{\"text\":\"Anglais\"}",
                                        "{\"text\":\"Espagnol\"}",
                                        "{\"text\":\"EMC\"}",
                                        "{\"text\":\"Sciences\"}",
                                        "{\"text\":\"Mathématiques\"}",
                                        "{\"text\":\"NSI\"}",
                                        "{\"text\":\"Physique-chimie\"}"
                                        };
std::vector<String> overviewAskingTypeList={"{\"text\":\"back to menu\"}",
                                            "{\"text\":\"Réponse(courte) à la question\"}",
                                            "{\"text\":\"Réponse(longue) à la question\"}",
                                            "{\"text\":\"Rédaction\"}",
                                            "{\"text\":\"Traduction(->Français)\"}",
                                            "{\"text\":\"Traduction(->Anglais)\"}",
                                            "{\"text\":\"Traduction(->Espagnol)\"}",
                                            "{\"text\":\"Résolution d'équation\"}",
                                            "{\"text\":\"Ortographe d'un mot\"}",
                                            "{\"text\":\"Définition d'un mot\"}",
                                            "{\"text\":\"Trouver la date\"}",
                                            "{\"text\":\"Description d'image\"}",
                                            };
String simpleTextViewText="";
std::vector<String> OverviewDatasList;
String askingTypeSelected;
int IndexFocused=0;
int lastViewFocused=0;
int lastIndexFocused=0;
int overviewXMoving=0;
int streakLeftButtonTouched=0;
int streakRightButtonTouched=0;
bool veilleMode=false;

void updateIndexFocused(void *pvParameters) {
    while(true){
        if (touchRead(button1) > 35  || touchRead(button2) > 35){
            while (touchRead(button1) < 35 && touchRead(button2) > 35) {
                if (IndexFocused>0){
                    IndexFocused-=1;
                    overviewXMoving=0;

                    if (pageName=="simple text view"){
                        delay(500-streakLeftButtonTouched*30);
                    }else{
                        delay(600);
                    }
                    
                    if (streakLeftButtonTouched<9){
                        streakLeftButtonTouched+=1;
                    }
                }
            }
            while(touchRead(button1)> 35 && touchRead(button2) < 35){
                overviewXMoving=0;
                IndexFocused+=1;
                if (pageName=="simple text view"){
                    delay(500-streakRightButtonTouched*30);
                }else{
                    delay(600);
                }
                if (streakRightButtonTouched<9){
                    streakRightButtonTouched+=1;
                }
            }
            streakLeftButtonTouched=0;
            streakRightButtonTouched=0;
            delay(2);
        }
    }
}


static esp_err_t json_post_handler(httpd_req_t *req) {
    Serial.println("Handling POST request...");

    char content[2048];
    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }

    content[recv_size] = '\0';

    char query[32];
    char chunkStr[8];
    char totalStr[8];
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK) {
        Serial.println("Error getting query string");
        return ESP_FAIL;
    }

    if (httpd_query_key_value(query, "chunk", chunkStr, sizeof(chunkStr)) != ESP_OK ||
        httpd_query_key_value(query, "total", totalStr, sizeof(totalStr)) != ESP_OK) {
        Serial.println("Error parsing query parameters");
        return ESP_FAIL;
    }

    int chunkIndex = atoi(chunkStr);
    int totalChunks = atoi(totalStr);

    if (expectedChunks == -1) {
        expectedChunks = totalChunks;
        chunks.clear();
        chunks.resize(totalChunks);
        Serial.printf("Expecting %d chunks\n", totalChunks);
    }

    Serial.printf("Received chunk %d\n", chunkIndex);
    chunks[chunkIndex] = String(content);

    if (chunkIndex == totalChunks - 1) {
        String completeJson;
        for (const String &chunk : chunks) {
            completeJson += chunk;
        }
        lastResponseString = completeJson;
        expectedChunks = -1; // Réinitialiser pour la prochaine requête
        Serial.println("Received complete JSON:");
        Serial.println(lastResponseString);
    }

    httpd_resp_send(req, "Chunk received", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

void startServer() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Augmenter la taille maximale des tampons de réception
    config.recv_wait_timeout = 30; // Augmente le timeout de réception si nécessaire
    config.send_wait_timeout = 30; // Augmente le timeout d'envoi si nécessaire
    config.max_uri_handlers = 16; // Augmente le nombre de gestionnaires d'URI
    config.max_resp_headers = 16; // Augmente le nombre de headers de réponse
    config.lru_purge_enable = true; // Active la purge LRU pour les connexions inactives
    config.uri_match_fn = httpd_uri_match_wildcard;

    httpd_uri_t capture_uri = {
        .uri       = "/capture",
        .method    = HTTP_GET,
        .handler   = capture_handler,
        .user_ctx  = NULL
    };
    httpd_uri_t jsonGET_uri = {
        .uri       = "/json-get",
        .method    = HTTP_GET,
        .handler   = json_get_handler,
        .user_ctx  = NULL
    };
    httpd_uri_t jsonPOST_uri = {
        .uri       = "/json-post",
        .method    = HTTP_POST,
        .handler   = json_post_handler,
        .user_ctx  = NULL
    };

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &capture_uri);
        httpd_register_uri_handler(server, &jsonGET_uri);
        httpd_register_uri_handler(server, &jsonPOST_uri);
        Serial.println("Server started successfully");
    } else {
        Serial.println("Failed to start server");
    }
}
void showPageText(String text);

void setup() {
    Wire.begin(SDA_PIN, SCL_PIN);  
    u8g2.begin();
    u8g2.setFont(u8g2_font_profont17_tr);

    Serial.begin(115200);

    touchAttachInterrupt(button1, NULL, 40); 
    touchAttachInterrupt(button2, NULL, 40);

    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Disable brownout detector

    // Init SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
      config.frame_size = FRAMESIZE_SVGA;
      config.jpeg_quality = 12;
      config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
      Serial.printf("Camera init failed with error 0x%x", err);
      return;
  }

  WiFi.begin(ssid, password);

    showPageText("Device not connected...");
  
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  u8g2.clearBuffer();
  Serial.println("");
  Serial.println("WiFi connected");

  // Init and get the time
  configTime(0, 0, "pool.ntp.org");

  Serial.print("Go to: http://");
  Serial.println(WiFi.localIP().toString());

  startServer();

  // Crée une tâche pour mettre à jour l'heure locale en arrière-plan
  xTaskCreate(updateLocalTime, "updateLocalTime", 2048, NULL, 1, NULL);
  xTaskCreate(updateIndexFocused, "updateIndexFocused", 2048, NULL, 1, NULL);

  randomSeed(millis());
}


std::vector<String> stringToVector(String input) {
    std::vector<String> result;

    const char* inputChar=input.c_str();
    String word = "";

    for (int i = 0; i < input.length(); i++) {

        char chara = inputChar[i];

        // Comparaison des caractères individuels
        if (chara != '[' && chara != ']') {
            if (!(word.isEmpty() && chara==',')){
                word += input[i];
            }
        }
        if (!word.isEmpty() && chara== '}' ) {
            result.push_back(word);
            word = "";
        }
    }
    return result;
}


std::vector<String> splitText(const char* text, int maxWidth) {
    std::vector<String> lines;
    String line = "";
    String word = "";
    const char* p = text;

    while (*p) {
        if (*p == ' ' || *p == '\n') {
            // Si un espace ou un saut de ligne est rencontré, on ajoute le mot en cours à la ligne
            if (!word.isEmpty()) {
                if (u8g2.getStrWidth((line + word).c_str()) > maxWidth) {
                    if (!line.isEmpty()) {
                        lines.push_back(line);
                    }
                    line = word;
                } else {
                    if (!line.isEmpty()) {
                        line += " ";
                    }
                    line += word;
                }
                word = "";
            }

            if (*p == '\n') {
                lines.push_back(line);
                line = "";
            }
        } else {
            word += *p;
        }

        p++;
    }

    // Ajouter le dernier mot et la dernière ligne
    if (!word.isEmpty()) {
        if (u8g2.getStrWidth((line + word).c_str()) > maxWidth) {
            if (!line.isEmpty()) {
                lines.push_back(line);
            }
            line = word;
        } else {
            if (!line.isEmpty()) {
                line += " ";
            }
            line += word;
        }
    }
    if (!line.isEmpty()) {
        lines.push_back(line);
    }

    return lines;
}

// Graphic interface
void showPageText(String text) {
    u8g2.clearBuffer();

    int indexFocused=IndexFocused;

    std::vector<String> lines=splitText(text.c_str(),128);
    int lineHeight = u8g2.getMaxCharHeight();
    if (indexFocused>lines.size()-3){
        indexFocused=lines.size()-3;
        if (indexFocused<0){
            indexFocused=0;
        }
        IndexFocused=indexFocused;
    }

    if (indexFocused==lastIndexFocused){
        for (int i = 0; i < lines.size(); i++) {
            int y =  -indexFocused*lineHeight + i * lineHeight;
            u8g2.drawStr(0,lineHeight + y -2, lines[i].c_str());

            if (i==lines.size()-1 && lines.size()>3){
                for (int xEndLine = 1; xEndLine < 128; xEndLine+=16) {
                    u8g2.drawLine(xEndLine,y+lineHeight+10,xEndLine+8,y+lineHeight+10);
                }
            }
        }
        u8g2.sendBuffer();

    }else{
        for (int offset = 1; offset < lineHeight; offset++){
            int offset_ = offset;
            if (indexFocused>lastIndexFocused){
                offset_ = -offset;
            }
            u8g2.clearBuffer();
            for (int i = 0; i < lines.size(); i++) {
                int y =  -lastIndexFocused*lineHeight + i * lineHeight  + offset_;
                u8g2.drawStr(0,lineHeight + y -2, lines[i].c_str());

                if (i==lines.size()-1 && lines.size()>3){
                    for (int xEndLine = 1; xEndLine < 128; xEndLine+=16) {
                        u8g2.drawLine(xEndLine,y+lineHeight+10,xEndLine+8,y+lineHeight+10);
                    }
                }
            }
            u8g2.sendBuffer();
        }
    }
    lastIndexFocused=indexFocused;
}

void showPageOverview(std::vector<String>& OverviewDatasList) {
    int lineHeight = u8g2.getMaxCharHeight();

    int indexFocused=IndexFocused;
    
    int viewFocused = -indexFocused;
    if (0 < indexFocused && indexFocused < OverviewDatasList.size()-1){
        viewFocused = -indexFocused+1;
    }else if(indexFocused>=OverviewDatasList.size()-1 && OverviewDatasList.size()>2) {
        indexFocused=OverviewDatasList.size()-1;
        IndexFocused=indexFocused;
        viewFocused = -indexFocused+2;
    }


    int x;
    if (viewFocused==lastViewFocused){
        u8g2.clearBuffer();
        for (int i = 0; i < OverviewDatasList.size(); i++) {
            int y =  viewFocused*lineHeight + i * lineHeight;
            DynamicJsonDocument bloc(200);
            deserializeJson(bloc, OverviewDatasList[i]);
            char oneInt[10];
            itoa(i+1, oneInt, 10);
            x = strlen(oneInt)*8.5+4;
            if (indexFocused==i){
                u8g2.drawBox(x, y+1 , 128-x, lineHeight-1);
                u8g2.setDrawColor(0);
                if (u8g2.getStrWidth(bloc["text"])>128-x){
                    if (overviewXMoving>=u8g2.getStrWidth(bloc["text"])+40){
                        overviewXMoving=0;
                    }else{
                        overviewXMoving++;
                    }
                    u8g2.drawStr(x-overviewXMoving+u8g2.getStrWidth(bloc["text"])+40,lineHeight + y -2, bloc["text"]);
                }
                u8g2.drawStr(x-overviewXMoving,lineHeight + y -2, bloc["text"]); 
            }else{
                u8g2.drawStr(x,y+lineHeight, bloc["text"]);            
            }
            u8g2.setDrawColor(0);    
            u8g2.drawBox(0, y , u8g2.getStrWidth(oneInt)+4, lineHeight+1);
            u8g2.setDrawColor(1);    
            u8g2.drawStr(0,y+lineHeight , oneInt);

            if (i==OverviewDatasList.size()-1 && OverviewDatasList.size()>3){
                for (int xEndLine = 1; xEndLine < 128; xEndLine+=16) {
                    u8g2.drawLine(xEndLine,y+lineHeight+10,xEndLine+8,y+lineHeight+10);
                }
            }
        }
        u8g2.sendBuffer();
    }else{
        for (int offset = 0; offset < lineHeight; offset++){
            int offset_ = offset;
            if (viewFocused<lastViewFocused){
                offset_ = -offset;
            }
            u8g2.clearBuffer();
            for (int i = 0; i < OverviewDatasList.size(); i++) {
                char oneInt[10]; 
                itoa(i+1, oneInt, 10);
                int y =  lastViewFocused*lineHeight + i * lineHeight  + offset_;
            
                DynamicJsonDocument bloc(200);
                deserializeJson(bloc, OverviewDatasList[i]);
                u8g2.drawStr(0,lineHeight + y -2, oneInt);
                int x = strlen(oneInt)*8.5+4;
                if (indexFocused==i){
                    u8g2.drawBox(x, y , 128-x, lineHeight);
                    u8g2.setDrawColor(0);
                }
                u8g2.drawStr(x,lineHeight + y -2, bloc["text"]);
                u8g2.setDrawColor(1); 

                if (i==OverviewDatasList.size()-1 && OverviewDatasList.size()>3){
                    for (int xEndLine = 1; xEndLine < 128; xEndLine+=16) {
                        u8g2.drawLine(xEndLine,y+lineHeight+10,xEndLine+8,y+lineHeight+10);
                    }
                }
            }
            u8g2.sendBuffer();
        }
    }
    lastViewFocused=viewFocused;
    lastIndexFocused=indexFocused;

}

int centerButtonStreak=0;
int leftButtonStreak=0;
void loop() {
    if (!veilleMode){
        if (WiFi.status() == WL_CONNECTED){
            if (pageName==""){
                pageName="menu";
            }
            // Response part
            if (lastResponseString != "" && lastResponseString != lastResponseDid) { // If Android response
                deserializeJson(lastResponseDict, lastResponseString); // getting response to dict

                if (lastResponseDict["type"] == "text") {
                    pageName = "simple text view";
                    simpleTextViewText=lastResponseDict["text"].as<String>();
                } else if (lastResponseDict["type"] == "overview") {

                    OverviewDatasList=stringToVector(lastResponseDict["overview"].as<String>());
                    OverviewDatasList.insert(OverviewDatasList.begin(),"{\"text\":\"back to menu\"}");

                    pageName = "overview";
                    IndexFocused = 0;
                }
                lastResponseDid = lastResponseString;

            }else if (pageName == "menu") {
                deserializeJson(jsonRequest, "{}");
                jsonRequest["type"] = "no request";
                showPageOverview(OverviewMenuList);
                if (touchRead(button1) < 35 && touchRead(button2) < 35) {// select button clicked

                    DynamicJsonDocument page(200);
                    deserializeJson(page, OverviewMenuList[IndexFocused]);
                    pageName=page["redirect"].as<const char*>();
                    IndexFocused = 0;
                }

            }else if (pageName=="askingType(chatGPT asking)"){
                showPageOverview(overviewAskingTypeList);
                if (touchRead(button1) < 35 && touchRead(button2) < 35) {
                    if (IndexFocused!=0){

                        deserializeJson(jsonRequest, "{}"); // reset request
                        DynamicJsonDocument askingType(200);
                        deserializeJson(askingType, overviewAskingTypeList[IndexFocused]);
                        askingTypeSelected=askingType["text"].as<String>();

                        pageName = "camForChatGPT";
                        IndexFocused=0;
                    }else{
                        pageName="menu";
                    }
                }
            }else if (pageName == "camForChatGPT") {
                showPageText("click for send photo");
                if (touchRead(button2) < 35) {
                    capture_image(); // Enregistrer la nouvelle image qui sera mise en ligne
                    jsonRequest["type"] = "chatGPT asking"; // request info
                    jsonRequest["time"] = currentTime;
                    jsonRequest["id"] = random(1000000);
                    jsonRequest["image source"] = "http://"+WiFi.localIP().toString()+"/capture";
                    jsonRequest["asking type"] = askingTypeSelected;

                    pageName = "simple text view";
                    IndexFocused = 0;
                    simpleTextViewText="sending request to android, waiting response...";
                }else if(touchRead(button1) < 35){
                    IndexFocused = 0;
                    pageName="menu";

                }
            }else if (pageName == "overview matiere") {
                showPageOverview(overviewMatieresList);
                if (touchRead(button1) < 35 && touchRead(button2) < 35) {
                    if (IndexFocused!=0){
                        deserializeJson(jsonRequest, "{}"); // reset request
                        jsonRequest["time"] = currentTime;
                        jsonRequest["id"] = random(1000000);
                        DynamicJsonDocument matiere(200);
                        deserializeJson(matiere, overviewMatieresList[IndexFocused]);
                        jsonRequest["matiere"] = matiere["text"];
                        jsonRequest["type"] = "overview lessons"; // request info

                        pageName = "simple text view";
                        IndexFocused = 0;
                        simpleTextViewText="sending request to android, waiting response...";
                    }else{
                        pageName="menu";
                    }
                }
                
            }else if (pageName=="askingType(chatGPT history)"){
                showPageOverview(overviewAskingTypeList);
                if (touchRead(button1) < 35 && touchRead(button2) < 35) {
                    if (IndexFocused!=0){

                        deserializeJson(jsonRequest, "{}"); // reset request
                        DynamicJsonDocument askingType(200);
                        deserializeJson(askingType, overviewAskingTypeList[IndexFocused]);
                        jsonRequest["type"]="chatGPT history overview";
                        jsonRequest["asking type"]=askingType["text"];

                        pageName = "simple text view";
                        IndexFocused=0;
                        simpleTextViewText="sending request to android, waiting response...";
                    }else{
                        pageName="menu";
                    }
                }
            }else if (pageName == "simple text view") {
                showPageText(simpleTextViewText);
                if (touchRead(button1)< 35 && touchRead(button2) < 35) {
                    pageName = "menu";
                    IndexFocused = 0;
                }
            } if (pageName == "overview") {
                showPageOverview(OverviewDatasList);
                if (touchRead(button1)< 35 && touchRead(button2) < 35) {
                    if (IndexFocused!=0){
                        if (lastResponseDict["response"]=="chatGPT history overview"){
                            deserializeJson(jsonRequest, "{}"); // reset request
                            jsonRequest["type"]="one chatGPT history";
                            DynamicJsonDocument overviewElement(200);
                            deserializeJson(overviewElement, OverviewDatasList[IndexFocused]);
                            jsonRequest["date"]=overviewElement["date"];

                            simpleTextViewText="sending request to android, waiting response...";
                            pageName = "simple text view";
                        }else if (lastResponseDict["response"]=="overview lessons"){

                            deserializeJson(jsonRequest, "{}"); // reset request
                            jsonRequest["type"]="one lesson";
                            DynamicJsonDocument overviewElement(200);
                            deserializeJson(overviewElement, OverviewDatasList[IndexFocused]);
                            jsonRequest["lesson title"]=overviewElement["text"];

                            simpleTextViewText="sending request to android, waiting response...";
                            pageName = "simple text view";
                        }
                    }else{
                        pageName = "menu";
                    }
                }
            }
        }else{
            u8g2.clearBuffer();
            showPageText("Device not connected...");
            u8g2.sendBuffer();
        }
    }else{
        u8g2.clearBuffer();
        u8g2.sendBuffer();
        delay(1000);
    }


    if (!(touchRead(button1) < 35 && touchRead(button2))){
        centerButtonStreak=0;
    }
    while (touchRead(button1) < 35 && touchRead(button2) < 35){
        centerButtonStreak+=1;
        if (centerButtonStreak>=250000){
            veilleMode=!veilleMode;
            centerButtonStreak=0;
            break;
        }
    }
}
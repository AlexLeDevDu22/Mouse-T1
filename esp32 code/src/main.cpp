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
#include <cmath>
#include <random>

// Commands
#define button1 14
#define button2 12
#define SDA_PIN 13
#define SCL_PIN 15

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

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
bool needWifi = false;
int expectedChunks = -1;
String pageName="menu";
String lastPageName="menu";
String gameChoiced="";
std::vector<String> OverviewMenuList={"{\"text\":\"chatGPT asking\",\"redirect\":\"askingType(chatGPT asking)\"}",
                                    "{\"text\":\"lessons\",\"redirect\":\"overview matiere\"}",
                                    "{\"text\":\"chatGPT history\",\"redirect\":\"askingType(chatGPT history)\"}",
                                    "{\"text\":\"Games\",\"redirect\":\"Games overview\"}"
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

                                            
std::vector<String> OverviewGamesList={ "{\"text\":\"back to menu\"}",
                                        "{\"text\":\"Morpion\"}",
                                        "{\"text\":\"Pong\"}",
                                        "{\"text\":\"Test Game\"}",
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
int centerButtonStreak=0;
bool veilleMode=false;

void morpionGame();
void pongGame();
void testGame();

void updateIndexFocused(void *pvParameters) {
    while(true){
        if (!veilleMode){
            if (touchRead(button1) > 35  || touchRead(button2) > 35){
                while (touchRead(button1) < 35 && touchRead(button2) > 35) {
                    if (IndexFocused>0){
                        IndexFocused-=1;
                        overviewXMoving=0;

                        if (pageName=="simple text view"){
                            delay(500-streakLeftButtonTouched*50);
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
}


void veilleModeManager(void *pvParameters){
    while (true){
        while (touchRead(button1) < 35 && touchRead(button2) < 35){
            centerButtonStreak+=1;
            if (centerButtonStreak>=250000){
                veilleMode=!veilleMode;
                centerButtonStreak=0;
                break;
            }
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

void startServer(void *pvParameters) {
    while (WiFi.status() != WL_CONNECTED){
        delay(100);
    }

    Serial.println("");
    Serial.println("WiFi connected");

    Serial.print("Go to: http://");
    Serial.println(WiFi.localIP().toString());

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
    while (true) {delay(100);}
}

void showPageText(String text);

void setup() {
    Wire.begin(SDA_PIN, SCL_PIN);  
    u8g2.begin();
    u8g2.enableUTF8Print();
    u8g2.setFont(u8g2_font_ncenB08_tr);

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

  configTime(0, 0, "pool.ntp.org");

  // Crée une tâche pour mettre à jour l'heure locale en arrière-plan
  xTaskCreate(startServer, "startServer", 2048, NULL, 1, NULL);
  xTaskCreate(updateIndexFocused, "updateIndexFocused", 2048, NULL, 1, NULL);
  xTaskCreate(veilleModeManager, "veilleModeManager", 2048, NULL, 1, NULL);

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

    int indexFocused=IndexFocused;

    std::vector<String> lines=splitText(text.c_str(),128);
    int lineHeight = u8g2.getMaxCharHeight();
    if (indexFocused>lines.size()-3){
        indexFocused=lines.size()-3;
        if (indexFocused<0){
            indexFocused=0;
        }
    }
    if (indexFocused>0 && lines.size()<4){
        indexFocused=0;
    }
    IndexFocused=indexFocused;

    u8g2.clearBuffer();
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

    if (lastViewFocused==-1 && indexFocused<1){
        lastViewFocused=viewFocused;
    }

    if (indexFocused>0 && OverviewDatasList.size()<4){
        viewFocused=0;
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
                u8g2.drawBox(x, y , u8g2.getStrWidth(bloc["text"])+2, lineHeight);
                u8g2.setDrawColor(0);
                if (u8g2.getStrWidth(bloc["text"])>128-x){
                    if (overviewXMoving>=u8g2.getStrWidth(bloc["text"])+40){
                        overviewXMoving=0;
                    }else{
                        overviewXMoving++;
                    }
                    u8g2.drawStr(x+1-overviewXMoving+u8g2.getStrWidth(bloc["text"])+40,lineHeight + y - 2, bloc["text"]);
                }
                u8g2.drawStr(x+1-overviewXMoving,lineHeight + y - 2, bloc["text"]); 
            }else{
                u8g2.drawStr(x+1,y+lineHeight - 2, bloc["text"]);            
            }
            u8g2.setDrawColor(0);
            u8g2.drawBox(0, y , x, lineHeight);
            u8g2.setDrawColor(1);
            u8g2.drawStr(0,y+lineHeight , oneInt);
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
                u8g2.drawStr(0,lineHeight + y, oneInt);
                int x = strlen(oneInt)*8.5+4;
                if (indexFocused==i){
                    u8g2.drawBox(x, y , u8g2.getStrWidth(bloc["text"])+2, lineHeight);
                    u8g2.setDrawColor(0);
                }
                u8g2.drawStr(x+1,lineHeight + y - 2, bloc["text"]);
                u8g2.setDrawColor(1); 
            }
            u8g2.sendBuffer();
        }
    }
    lastViewFocused=viewFocused;
}

void loop() {
    if (!veilleMode){
        u8g2.setFont(u8g2_font_ncenB08_tr);
        if (WiFi.status() == WL_CONNECTED && needWifi || !needWifi){
            if(pageName!=lastPageName){
                IndexFocused=0;
                overviewXMoving=0;
                lastViewFocused=-1;
                lastPageName=pageName;
            }
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
                    jsonRequest["time"] = millis();
                    jsonRequest["id"] = random(1000000);
                    jsonRequest["image source"] = "http://"+WiFi.localIP().toString()+"/capture";
                    jsonRequest["asking type"] = askingTypeSelected;

                    pageName = "simple text view";
                    
                    simpleTextViewText="sending request to android, waiting response...";

                    needWifi=true;
                }else if(touchRead(button1) < 35){
                    
                    pageName="menu";

                }
            }else if (pageName == "overview matiere") {
                showPageOverview(overviewMatieresList);
                if (touchRead(button1) < 35 && touchRead(button2) < 35) {
                    if (IndexFocused!=0){
                        deserializeJson(jsonRequest, "{}"); // reset request
                        jsonRequest["time"] = millis();
                        jsonRequest["id"] = random(1000000);
                        DynamicJsonDocument matiere(200);
                        deserializeJson(matiere, overviewMatieresList[IndexFocused]);
                        jsonRequest["matiere"] = matiere["text"];
                        jsonRequest["type"] = "overview lessons"; // request info

                        pageName = "simple text view";
                        
                        simpleTextViewText="sending request to android, waiting response...";

                        needWifi=true;
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

                        needWifi=true;
                    }else{
                        pageName="menu";
                    }
                }
            }else if (pageName=="Games overview"){
                showPageOverview(OverviewGamesList);
                if (touchRead(button1)< 35 && touchRead(button2) < 35) {
                    if (IndexFocused!=0){
                        pageName = "In Game";
                        DynamicJsonDocument games(200);
                        deserializeJson(games, OverviewGamesList[IndexFocused]);
                        gameChoiced=games["text"].as<const char*>();
                    }else{
                        pageName="menu";
                    }
                }
            }else if (pageName=="In Game"){
                if (gameChoiced=="Morpion"){
                    morpionGame();
                }else if (gameChoiced=="Pong") {
                    pongGame();
                }else if (gameChoiced=="Test Game") {
                    testGame();
                }
                pageName="Games overview";

            }else if (pageName == "simple text view") {
                showPageText(simpleTextViewText);
                if (touchRead(button1)< 35 && touchRead(button2) < 35) {
                    pageName = "menu";
                    
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

                        needWifi=true;
                    }else{
                        pageName = "menu";
                    }
                }
            }
        }else{
            u8g2.clearBuffer();
            showPageText("Device not connected...");
            if (touchRead(button1)< 35 && touchRead(button1)< 35){
                pageName = "menu";
                needWifi=false;
            }
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

    while (touchRead(button1) < 35 || touchRead(button2) < 35 && lastPageName!=pageName){}
}











void drawTable(int xOffset, std::vector<String> tableValues,bool showIndex){
    u8g2.clearBuffer();

    for (int collumnBorder = 0; collumnBorder<4; collumnBorder++){
        u8g2.drawBox(xOffset+collumnBorder*10, 0, 1, 30);
    }

    for (int rowBorder = 0; rowBorder<4; rowBorder++){
        
        u8g2.drawBox(xOffset, rowBorder * 10 , 30, 1);
        
    }

    for (int x = 0; x<3; x++){
        for (int y = 0; y<3; y++){

        if (IndexFocused == x+y*3 && showIndex){
            u8g2.drawBox(xOffset + x*10, y*10,10,10);
            u8g2.setDrawColor(0);
        }

        if (tableValues[x+y*3].c_str()!=" "){
            u8g2.drawStr(xOffset + x*10 + 3, y*10+9,tableValues[x+y*3].c_str());
        }
        
        u8g2.setDrawColor(1);
        }
    }
    u8g2.sendBuffer();


}

bool checkLine(const std::vector<String>& tableValues, int startX, int startY, int dx, int dy, const String& player, int length,                std::vector<int>& winningIndices) {
    int count = 0;
    int x = startX, y = startY;
    winningIndices.clear();
    for (int i = 0; i < length; ++i) {
        if (x < 0 || x >= 3 || y < 0 || y >= 3 || tableValues[y * 3 + x] != player) {
            return false;
        }
        winningIndices.push_back(y * 3 + x);
        count++;
        x += dx;
        y += dy;
    }
    return count == length;
}

// Convertit un index du plateau en coordonnées x, y
void indexToCoordinates(int index, int& x, int& y) {
    x = index % 3;
    y = index / 3;
}

bool checkForWin(const std::vector<String>& tableValues, int lastMoveIndex, std::vector<int>& winningIndices) {
    int lastMoveX, lastMoveY;
    indexToCoordinates(lastMoveIndex, lastMoveX, lastMoveY);
    String player = tableValues[lastMoveIndex];
    
    // Vérifier la ligne horizontale
    if (checkLine(tableValues, 0, lastMoveY, 1, 0, player, 3, winningIndices)) {
        return true;
    }
    
    // Vérifier la colonne verticale
    if (checkLine(tableValues, lastMoveX, 0, 0, 1, player, 3, winningIndices)) {
        return true;
    }
    
    // Vérifier la diagonale (haut-gauche à bas-droit)
    if (lastMoveX == lastMoveY && checkLine(tableValues, 0, 0, 1, 1, player, 3, winningIndices)) {
        return true;
    }
    
    // Vérifier la diagonale (haut-droit à bas-gauche)
    if (lastMoveX + lastMoveY == 2 && checkLine(tableValues, 2, 0, -1, 1, player, 3, winningIndices)) {
        return true;
    }
    
    return false;
}








void morpionGame(){
    u8g2.setFont(u8g2_font_6x12_tr);


    bool player1Play = true;
 
    std::vector<String> tableValues = {" ", " ", " ",
                                       " ", " ", " ",
                                       " ", " ", " "};
    
    while (true){
        u8g2.clearBuffer();
        if (!veilleMode){
            
             int xOffset;
            if (player1Play){
              xOffset=0;
            }else {
              xOffset=95;
            }
          
            drawTable(xOffset,tableValues,true);
           
            
            if (IndexFocused>8){
               IndexFocused=8;
            }
            
            if (touchRead(button1) < 35 && touchRead(button2) < 35) {
                if (tableValues[IndexFocused]==" "){
                   if (player1Play){
                     tableValues[IndexFocused]="X";
                   }else {
                     tableValues[IndexFocused]="O";
                   }
                   

                    //Check for win

                    std::vector<int> winningIndices;
                    if (checkForWin(tableValues, IndexFocused, winningIndices)) {

                        //Anim for move table to the center
                        for (int i = 1; i<=48; i++){
                            if (player1Play){
                                drawTable(xOffset+i,tableValues,false);

                            }else{
                                drawTable(xOffset-i,tableValues,false);
                            }
                            delay(10);

                        }

                        std::vector<String> noWinningTableValues = tableValues;
                                                                  
                        for (int i=0;i<3;i++){
                            noWinningTableValues[winningIndices[i]]=" ";
                        }

                        for (int t=300;t>0;t-=40){
                            drawTable(48,tableValues,false);
                            delay(t);
                            drawTable(48,noWinningTableValues,false);
                            delay(t);
                        }
                        drawTable(48,tableValues,false);

                        while (true){
                            if (touchRead(button1) < 35 || touchRead(button2) < 35){
                                return;
                            }
                        }

                    } else{

                        int casesEmpty=0;
                        for (int i = 0; i<9;i++){
                            if (tableValues[i] == " "){
                                casesEmpty++;
                            }
                        }

                        if (casesEmpty==0){
                            while (true){

                                for (int t=300;t>0;t-=40){
                                    drawTable(48,tableValues,false);
                                    delay(t);
                                    u8g2.clearBuffer();
                                    delay(t);
                                }
                                drawTable(48,tableValues,false);
                                
                                if (touchRead(button1) < 35 || touchRead(button2) < 35){
                                    return;
                                }
                            }
                        }


                        //animation for change side of table
                        for (int i = 1; i<96; i++){
                            if (player1Play){
                                drawTable(xOffset+i,tableValues,true);

                            }else{
                                drawTable(xOffset-i,tableValues,true);
                            }
                        }
                        player1Play=!player1Play;
                    }
               }
            }
        }else{
            u8g2.clearBuffer();
            u8g2.sendBuffer();
        }
        
    }
}


void updateBallPosition(float& balPosX, float& balPosY, int balAngle, float balSpeed, bool isSmash, int UltiTargetTo,int yPlayer1Pos, int yPlayer2Pos) {

    // Convert angle to radians
    float radians = balAngle * 3.14159265 / 180.0;
    
    // Calculate the change in position
    float deltaX = cos(radians);
    float deltaY = sin(radians);

    if (isSmash){
        balSpeed += 0.5;
    }
        
    // Update ball position
    balPosX += deltaX*balSpeed;
    balPosY += deltaY*balSpeed;

    if(UltiTargetTo==1 && balPosX>64 ) {
        balPosY=yPlayer1Pos+22;
        if (balPosY>31){
            balPosY=balPosY-32;
        }
    }else if(UltiTargetTo==2 && balPosX<64 ){
        balPosY=yPlayer2Pos+22;
        if (balPosY>31){
            balPosY=balPosY-32;
        }
    }
}

std::string getRandomUlti() {

    const std::vector<std::string> vec = {"Smash", "Slow", "Target", "Narrowing"};

    Serial.println(vec[millis()%vec.size()].c_str());
    return vec[millis()%vec.size()];
}

void pongGame(){
    
    
    int player1Score=0;
    int player2Score=0;

    float player1Ulti=0;
    std::string namePlayer1Ulti="";

    float player2Ulti=0;
    std::string namePlayer2Ulti="";

    int roundsWinForFinish;

    while (true){
        if (!veilleMode){
            u8g2.clearBuffer();

            u8g2.drawStr(20,u8g2.getMaxCharHeight(), "nombre de manche a gagner");

            char roundsWinForFinishStr[10];
            itoa(IndexFocused, roundsWinForFinishStr, 10);
            u8g2.drawStr(20,20, roundsWinForFinishStr);

            if (touchRead(button1) < 35 && touchRead(button1) < 35 && IndexFocused>0){
                roundsWinForFinish=IndexFocused;
                while (touchRead(button1) < 35 || touchRead(button2) < 35){}
                break;
            }

            u8g2.sendBuffer();
        }else{
            u8g2.clearBuffer();
            u8g2.sendBuffer();
        }
    }

	while (true){//rounds
        float yPlayer1Pos=0;
	    float yPlayer2Pos=0;

        int balAngle = std::to_string(millis())[0] - '0';
        float balSpeed=1;
        float balPosX = 64;
        float balPosY = 16;

        int lastPlayerTouchBal=0;

        bool roundFinished=false;

        //Ulti vars
        bool isSmash=false;

        int ultiPlayerSlowed=0;

        int UltiTargetTo=0;

        int player1Height=12;
        int player2Height=12;

        while (true){
            u8g2.clearBuffer();
            //if (!veilleMode){


                if (player1Ulti>=32-u8g2.getMaxCharHeight()-3){
                    if (namePlayer1Ulti==""){
                        namePlayer1Ulti=getRandomUlti();
                    }

                    if (namePlayer1Ulti=="Smash"){
                        isSmash=true;
                        u8g2.setFont(u8g2_font_twelvedings_t_all);
                        u8g2.drawGlyph(10, 20, 41);
                    }else if (namePlayer1Ulti=="Slow"){
                        ultiPlayerSlowed=2;
                        u8g2.setFont(u8g2_font_twelvedings_t_all);
                        u8g2.drawGlyph(10, 20, 0x0067);
                    }else if (namePlayer1Ulti=="Target"){
                        UltiTargetTo=2;
                        u8g2.setFont(u8g2_font_unifont_t_86);
                        u8g2.drawGlyph(10, 20, 0x2b59);
                    }else if (namePlayer1Ulti=="Narrowing"){
                        player2Height=7;
                        u8g2.setFont(u8g2_font_unifont_t_symbols);
                        u8g2.drawGlyph(10, 20, 0x21d4);
                    }
                    u8g2.setFont(u8g2_font_ncenB08_tr);

                    
                    if (namePlayer1Ulti!="Smash"){
                        isSmash=false;
                    }
                    if (namePlayer1Ulti!="Slow"){
                        ultiPlayerSlowed=0;
                    }
                    if (namePlayer1Ulti!="Target"){
                        UltiTargetTo=0;
                    }
                    if (namePlayer1Ulti!="Narrowing"){
                        player2Height=12;
                    }
                }

                if (player2Ulti>=32-u8g2.getMaxCharHeight()-3){

                    if (namePlayer2Ulti==""){
                        namePlayer2Ulti=getRandomUlti();
                    }
                    
                    if (namePlayer2Ulti=="Smash"){
                        isSmash=true;
                        u8g2.setFont(u8g2_font_twelvedings_t_all);
                        u8g2.drawGlyph(103, 20, 40);
                    }else if (namePlayer2Ulti=="Slow"){
                        ultiPlayerSlowed=1;
                        u8g2.setFont(u8g2_font_twelvedings_t_all);
                        u8g2.drawGlyph(103, 20, 0x0067);
                    }else if (namePlayer2Ulti=="Target"){
                        UltiTargetTo=1;
                        u8g2.setFont(u8g2_font_unifont_t_86);
                        u8g2.drawGlyph(103, 20, 0x2b59);
                    }else if (namePlayer2Ulti=="Narrowing"){
                        player1Height=7;
                        u8g2.setFont(u8g2_font_8x13_t_symbols);
                        u8g2.drawGlyph(103, 20, 0x2195);
                    }

                    u8g2.setFont(u8g2_font_ncenB08_tr);

                    
                    if (namePlayer2Ulti!="Smash"){
                        isSmash=false;
                    }
                    if (namePlayer2Ulti!="Slow"){
                        ultiPlayerSlowed=0;
                    }
                    if (namePlayer2Ulti!="Target"){
                        UltiTargetTo=0;
                    }
                    if (namePlayer1Ulti!="Narrowing"){
                        player1Height=12;
                    }
                }
                        
                        
                if (touchRead(button1)<35){
                    if (ultiPlayerSlowed!=1){
                        yPlayer1Pos+=1;
                    }else{
                        yPlayer1Pos+=0.5;
                    }
                    if (yPlayer1Pos>=32){
                        yPlayer1Pos=0;
                    }
                }
                if (touchRead(button2)<35){
                    if (ultiPlayerSlowed!=2){
                        yPlayer2Pos+=1;
                    }else{
                        yPlayer2Pos+=0.5;
                    }
                    if (yPlayer2Pos>=32){
                        yPlayer2Pos=0;
                    }
                }

                char player1ScoreStr[10];
                itoa(player1Score, player1ScoreStr, 10);
                int player1ScoreStrWidth = u8g2.getStrWidth(player1ScoreStr);

                char player2ScoreStr[10];
                itoa(player2Score, player2ScoreStr, 10);
                int player2ScoreStrWidth = u8g2.getStrWidth(player2ScoreStr);

                if (roundsWinForFinish>1){

                    u8g2.drawStr(0,u8g2.getMaxCharHeight(),player1ScoreStr);
                    u8g2.drawStr(128-player2ScoreStrWidth,u8g2.getMaxCharHeight(),player2ScoreStr);

                }
                u8g2.drawBox(player1ScoreStrWidth+2,(int)yPlayer1Pos,2,player1Height);
                if (yPlayer1Pos>32-player1Height){
                    u8g2.drawBox(player1ScoreStrWidth+2,0,2,player1Height-(32-(int)yPlayer1Pos));
                }

                u8g2.drawBox(128-player2ScoreStrWidth-4,(int)yPlayer2Pos,2,player2Height);
                if (yPlayer2Pos>32-player2Height){
                    u8g2.drawBox(128-player2ScoreStrWidth-4,0,2,player2Height-(32-(int)yPlayer2Pos));
                }

                for (int i=0;i<16;i++){
                    u8g2.drawLine(i*8+player1ScoreStrWidth,0,i*8+4+player1ScoreStrWidth,0);
                    u8g2.drawLine(4+i*8+player1ScoreStrWidth,31,i*8+8+player1ScoreStrWidth,31);
                }
            
                u8g2.drawLine(0,u8g2.getMaxCharHeight()+1,player1ScoreStrWidth,u8g2.getMaxCharHeight()+1);
                if (player1Ulti<32-u8g2.getMaxCharHeight()-3){
                    u8g2.drawBox(0,31-player1Ulti,3,32);
                    u8g2.drawBox(1,31-player1Ulti-1,1,1);
                }

                u8g2.drawLine(128-player2ScoreStrWidth,u8g2.getMaxCharHeight()+1,128,u8g2.getMaxCharHeight()+1);
                if (player2Ulti<32-u8g2.getMaxCharHeight()-3){
                    u8g2.drawBox(125,31-player2Ulti,3,32);
                    u8g2.drawBox(126,31-player2Ulti-1,1,1);
                }

                if  ((int)balPosX==5+player1ScoreStrWidth && (((int)yPlayer1Pos-3<=(int)balPosY && (int)balPosY<=(int)yPlayer1Pos+player1Height+3) ||  balPosY<=(int)yPlayer1Pos-(32-player1Height) )){

                    balAngle=270+270-balAngle;
                    balAngle+=std::to_string(millis())[0] - '0';
                    balPosX=6+player1ScoreStrWidth;

                    lastPlayerTouchBal=1;

                    if (player1Ulti<32-u8g2.getMaxCharHeight()-2){
                        player1Ulti+=2;
                    }

                    if (player2Ulti>=32-u8g2.getMaxCharHeight()-3){
                        player2Ulti=0;
                        namePlayer2Ulti="";
                    }

                }else if ((int)balPosX==123-player2ScoreStrWidth && (((int)yPlayer2Pos-3<=(int)balPosY && (int)balPosY<=(int)yPlayer2Pos+player2Height+3) || balPosY<=(int)yPlayer2Pos-(32-player1Height))){
                    balAngle=90+90-balAngle;
                    balAngle+=std::to_string(millis())[0] - '0';
                    lastPlayerTouchBal=2;
                    balPosX=122-player2ScoreStrWidth;

                    if (player2Ulti<32-u8g2.getMaxCharHeight()-2){
                        player2Ulti+=2;
                    }

                    if (player1Ulti>=32-u8g2.getMaxCharHeight()-3){
                        player1Ulti=0;
                        namePlayer1Ulti="";
                    }

                }else if ((int)balPosX==5+player1ScoreStrWidth){
                    player2Score++;
                    roundFinished=true;
                }else if ((int)balPosX==123-player2ScoreStrWidth){
                    player1Score++;
                    roundFinished=true;
                }

                if ((int)balPosY==3 || (int)balPosY==29){
                    balAngle=-balAngle;
                    balAngle+=std::to_string(millis())[0] - '0';
                    balSpeed+=0.005;

                    if (lastPlayerTouchBal==1){
                        player1Ulti++;
                    }else if (lastPlayerTouchBal==2){
                        player2Ulti++;
                    }
                    lastPlayerTouchBal=0;

                    if ((int)balPosY==3){
                        balPosY=4;
                    }else{
                        balPosY=28;
                    }
                }

                updateBallPosition(balPosX, balPosY, balAngle, balSpeed, isSmash, UltiTargetTo, (int)yPlayer1Pos, (int)yPlayer2Pos);

                if (balPosX<5+player1ScoreStrWidth){
                    balPosX=5+player1ScoreStrWidth;
                }else if (balPosX>123-player2ScoreStrWidth){
                    balPosX=123-player2ScoreStrWidth;
                }
                if (balPosY<3){
                    balPosY=3;
                }else if (balPosY>29){
                    balPosY=29;
                }

                u8g2.drawCircle(balPosX,balPosY,3);


                if (touchRead(button1) < 35 && touchRead(button2) < 35){
                    delay(40);
                    if (touchRead(button1) > 35 && touchRead(button2) > 35){
                        return;
                    }
                }
            //}
            if (roundFinished){
                break;
            }
        u8g2.sendBuffer();
        }
        if (player1Score==roundsWinForFinish){
            return;//player 1 win
        }else if (player2Score==roundsWinForFinish){
            return;//player 2 win
        }
        delay(500);
    }
}










void testGame(){
    while (true){
        u8g2.clearBuffer();

        u8g2.setFont(u8g2_font_twelvedings_t_all);
        u8g2.drawGlyph(105, 20, 40);
        u8g2.sendBuffer();

        delay(1000);

        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_twelvedings_t_all);
        u8g2.drawGlyph(105, 20, 0x0067);
        u8g2.sendBuffer();

        delay(1000);

        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_unifont_t_86);
        u8g2.drawGlyph(105, 20, 0x2b59);
        u8g2.sendBuffer();

        delay(1000);
    }
}
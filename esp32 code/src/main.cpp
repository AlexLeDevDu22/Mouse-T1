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
#include <stdlib.h>

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
                                        "{\"text\":\"Dino Jump\"}",
                                        "{\"text\":\"Geometry Dash\"}",
                                        "{\"text\":\"sudoku generator\"}",
                                        "{\"text\":\"Test Game\"}"
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
void dinoGame();
void geometryDash();
void sudokuGenerator();
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
                }else if (gameChoiced=="Dino Jump") {
                    dinoGame();
                }else if (gameChoiced=="Geometry Dash") {
                    geometryDash();
                }else if (gameChoiced=="sudoku generator") {
                    sudokuGenerator();
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


void updateBallPosition(std::vector<float>& balPos, int balAngle, float balSpeed, std::vector<bool> isSmash, std::vector<bool> UltiPlayersTarget,std::vector<float> yPlayerPos, std::vector<bool> UltiAllDirections) {

    if (UltiAllDirections[0]){
        balAngle=millis()%90+135;
    }else if (UltiAllDirections[1]){
        balAngle=millis()%180-45;
    }

    // Convert angle to radians
    float radians = balAngle * 3.14159265 / 180.0;
    
    // Calculate the change in position
    float deltaX = cos(radians);
    float deltaY = sin(radians);

    if (isSmash[0] || isSmash[1]){
        balSpeed += 0.7;
    }
        
    // Update ball position
    balPos[0] += deltaX*balSpeed;
    balPos[1] += deltaY*balSpeed;

    if(UltiPlayersTarget[0] && balPos[0]>40 ) {
        balPos[1]=(int)yPlayerPos[0]+22;
        if (balPos[1]>31){
            balPos[1]=balPos[1]-32;
        }
    }else if(UltiPlayersTarget[1] && balPos[0]<88 ){
        balPos[1]=(int)yPlayerPos[1]+22;
        if (balPos[1]>31){
            balPos[1]=balPos[1]-32;
        }
    }
}

std::string getRandomUlti() {

    const std::vector<std::string> vec = {"Smash", "Slow", "Target", "Narrowing", "All Directions"};

    Serial.println(vec[millis()%vec.size()].c_str());
    return vec[millis()%vec.size()];
}

void pongGame(){
    
    std::vector<int> playersScore={0,0};
    
    std::vector<float> playersUlti={16,16};
    std::vector<std::string> namePlayersUlti={"",""};

    int roundsWinForFinish;

    while (true){
        if (!veilleMode){
            u8g2.clearBuffer();

            u8g2.drawStr(20,u8g2.getMaxCharHeight(), "nombres de manches gagnantes");

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
        std::vector<float> yPlayerPos={0,0};

        int balAngle = std::to_string(millis())[0] - '0';
        float balSpeed=1;
        std::vector<float> balPos = {64,16};

        int lastPlayerTouchBal=0;

        bool roundFinished=false;

        //Ulti vars
        std::vector<bool> ultiPlayerSmash={false,false};

        std::vector<bool> ultiPlayerSlowed={false,false};

        std::vector<bool>  UltiPlayersTarget={false,false};

        std::vector<int> playersHeight={12,12};

        std::vector<bool> UltiAllDirections={false, false};

        while (true){
            u8g2.clearBuffer();
            //if (!veilleMode){


                if (playersUlti[0]>=32-u8g2.getMaxCharHeight()-3){
                    if (namePlayersUlti[0]==""){
                        namePlayersUlti[0]=getRandomUlti();
                    }

                    if (namePlayersUlti[0]=="Smash"){
                        ultiPlayerSmash[0]=true;
                        u8g2.setFont(u8g2_font_twelvedings_t_all);
                        u8g2.drawGlyph(10, 20, 41);
                    }else if (namePlayersUlti[0]=="Slow"){
                        ultiPlayerSlowed[1]=true;
                        u8g2.setFont(u8g2_font_twelvedings_t_all);
                        u8g2.drawGlyph(10, 20, 0x0067);
                    }else if (namePlayersUlti[0]=="Target"){
                        UltiPlayersTarget[1]=true;
                        u8g2.setFont(u8g2_font_unifont_t_86);
                        u8g2.drawGlyph(10, 20, 0x2b59);
                    }else if (namePlayersUlti[0]=="Narrowing"){
                        playersHeight[1]=7;
                        u8g2.setFont(u8g2_font_unifont_t_symbols);
                        u8g2.drawGlyph(10, 20, 0x21d4);
                    }else if (namePlayersUlti[0]=="All Directions"){
                        UltiAllDirections[0]=true;
                        u8g2.setFont(u8g2_font_cu12_t_symbols);
                        u8g2.drawGlyph(103, 20, 0x219b);
                    }

                    u8g2.setFont(u8g2_font_ncenB08_tr);

                }else{
                    ultiPlayerSmash[0]=false;
                    ultiPlayerSlowed[1]=false;
                    UltiPlayersTarget[1]=false;
                    playersHeight[1]=12;
                    UltiAllDirections[0]=false;

                }

                if (playersUlti[1]>=32-u8g2.getMaxCharHeight()-3){

                    if (namePlayersUlti[1]==""){
                        namePlayersUlti[1]=getRandomUlti();
                    }
                    
                    if (namePlayersUlti[1]=="Smash"){
                        ultiPlayerSmash[1]=true;
                        u8g2.setFont(u8g2_font_twelvedings_t_all);
                        u8g2.drawGlyph(103, 20, 40);
                    }else if (namePlayersUlti[1]=="Slow"){
                        ultiPlayerSlowed[0]=true;
                        u8g2.setFont(u8g2_font_twelvedings_t_all);
                        u8g2.drawGlyph(103, 20, 0x0067);
                    }else if (namePlayersUlti[1]=="Target"){
                        UltiPlayersTarget[0]=true;
                        u8g2.setFont(u8g2_font_unifont_t_86);
                        u8g2.drawGlyph(103, 20, 0x2b59);
                    }else if (namePlayersUlti[1]=="Narrowing"){
                        playersHeight[0]=7;
                        u8g2.setFont(u8g2_font_8x13_t_symbols);
                        u8g2.drawGlyph(103, 20, 0x2195);
                    }else if (namePlayersUlti[1]=="All Directions"){
                        UltiAllDirections[1]=true;
                        u8g2.setFont(u8g2_font_cu12_t_symbols);
                        u8g2.drawGlyph(103, 20, 0x219c);
                    }

                    u8g2.setFont(u8g2_font_ncenB08_tr);
                
                }else{
                        ultiPlayerSmash[0]=false;
                        ultiPlayerSlowed[0]=false;
                        UltiPlayersTarget[0]=false;
                        playersHeight[0]=12;
                        UltiAllDirections[1]=false;

                }
                        
                        
                if (touchRead(button1)<35){
                    if (ultiPlayerSlowed[0]){
                        yPlayerPos[0]+=0.4;
                    }else{
                        yPlayerPos[0]+=1;
                    }
                    if (yPlayerPos[0]>=32){
                        yPlayerPos[0]=0;
                    }
                }
                if (touchRead(button2)<35){
                    if (ultiPlayerSlowed[1]){
                        yPlayerPos[1]+=0.4;
                    }else{
                        yPlayerPos[1]+=1;
                    }
                    if (yPlayerPos[1]>=32){
                        yPlayerPos[1]=0;
                    }else{
                        yPlayerPos[1]+=0.5;
                    }
                }

                std::vector<std::string> playersScoreStr = {"", ""};
                std::vector<int> playersScoreStrWidth = {0, 0};

                // Conversion des scores en chaînes de caractères
                playersScoreStr[0] = std::to_string(playersScore[0]);
                playersScoreStrWidth[0] = u8g2.getStrWidth(playersScoreStr[0].c_str());

                playersScoreStr[1] = std::to_string(playersScore[1]);
                playersScoreStrWidth[1] = u8g2.getStrWidth(playersScoreStr[1].c_str());

                if (roundsWinForFinish > 1) {
                    u8g2.drawStr(0, u8g2.getMaxCharHeight(), playersScoreStr[0].c_str());
                    
                    u8g2.drawStr(128 - playersScoreStrWidth[1], u8g2.getMaxCharHeight(), playersScoreStr[1].c_str());
                }

                u8g2.drawBox(playersScoreStrWidth[0]+2,(int)yPlayerPos[0],2,playersHeight[0]);
                if (yPlayerPos[0]>32-playersHeight[0]){
                    u8g2.drawBox(playersScoreStrWidth[0]+2,0,2,playersHeight[0]-(32-(int)yPlayerPos[0]));
                }

                u8g2.drawBox(128-playersScoreStrWidth[1]-4,(int)yPlayerPos[1],2,playersHeight[1]);
                if (yPlayerPos[1]>32-playersHeight[1]){
                    u8g2.drawBox(128-playersScoreStrWidth[1]-4,0,2,playersHeight[1]-(32-(int)yPlayerPos[1]));
                }

                for (int i=0;i<16;i++){
                    u8g2.drawLine(i*8+playersScoreStrWidth[0],0,i*8+4+playersScoreStrWidth[0],0);
                    u8g2.drawLine(4+i*8+playersScoreStrWidth[0],31,i*8+8+playersScoreStrWidth[0],31);
                }
            
                u8g2.drawLine(0,u8g2.getMaxCharHeight()+1,playersScoreStrWidth[0],u8g2.getMaxCharHeight()+1);
                if (playersUlti[0]<32-u8g2.getMaxCharHeight()-3){
                    u8g2.drawBox(0,31-playersUlti[0],3,32);
                    u8g2.drawBox(1,31-playersUlti[0]-1,1,1);
                }

                u8g2.drawLine(128-playersScoreStrWidth[1],u8g2.getMaxCharHeight()+1,128,u8g2.getMaxCharHeight()+1);
                if (playersUlti[1]<32-u8g2.getMaxCharHeight()-3){
                    u8g2.drawBox(125,31-playersUlti[1],3,32);
                    u8g2.drawBox(126,31-playersUlti[1]-1,1,1);
                }

                if  ((int)balPos[0]==5+playersScoreStrWidth[0] && (((int)yPlayerPos[0]-3<=(int)balPos[1] && (int)balPos[1]<=(int)yPlayerPos[0]+playersHeight[0]+3) ||  balPos[1]<=(int)yPlayerPos[0]-(32-playersHeight[0]) )){

                    balAngle=270+270-balAngle;
                    balAngle+=std::to_string(millis())[0] - '0';
                    balPos[0]=6+playersScoreStrWidth[0];

                    lastPlayerTouchBal=1;

                    if (playersUlti[0]<32-u8g2.getMaxCharHeight()-2){
                        playersUlti[0]+=2;
                    }

                    if (playersUlti[1]>=32-u8g2.getMaxCharHeight()-3){
                        playersUlti[1]=0;
                        namePlayersUlti[1]="";
                    }

                }else if ((int)balPos[0]==123-playersScoreStrWidth[1] && (((int)yPlayerPos[1]-3<=(int)balPos[1] && (int)balPos[1]<=(int)yPlayerPos[1]+playersHeight[1]+3) || balPos[1]<=(int)yPlayerPos[1]-(32-playersHeight[1]))){
                    balAngle=90+90-balAngle;
                    balAngle+=std::to_string(millis())[0] - '0';
                    lastPlayerTouchBal=2;
                    balPos[0]=122-playersScoreStrWidth[1];

                    if (playersUlti[1]<32-u8g2.getMaxCharHeight()-2){
                        playersUlti[1]+=2;
                    }

                    if (playersUlti[0]>=32-u8g2.getMaxCharHeight()-3){
                        playersUlti[0]=0;
                        namePlayersUlti[0]="";
                    }

                }else if ((int)balPos[0]==5+playersScoreStrWidth[0]){
                    playersScore[1]++;
                    roundFinished=true;
                }else if ((int)balPos[0]==123-playersScoreStrWidth[1]){
                    playersScore[0]++;
                    roundFinished=true;
                }

                if ((int)balPos[1]==3 || (int)balPos[1]==29){
                    balAngle=-balAngle;
                    balAngle+=std::to_string(millis())[0] - '0';
                    balSpeed+=0.005;

                    if (lastPlayerTouchBal==1){
                        playersUlti[0]++;
                    }else if (lastPlayerTouchBal==2){
                        playersUlti[1]++;
                    }
                    lastPlayerTouchBal=0;

                    if ((int)balPos[1]==3){
                        balPos[1]=4;
                    }else{
                        balPos[1]=28;
                    }
                }

                updateBallPosition(balPos, balAngle, balSpeed, ultiPlayerSmash, UltiPlayersTarget, yPlayerPos, UltiAllDirections);

                if (balPos[0]<5+playersScoreStrWidth[0]){
                    balPos[0]=5+playersScoreStrWidth[0];
                }else if (balPos[0]>123-playersScoreStrWidth[1]){
                    balPos[0]=123-playersScoreStrWidth[1];
                }
                if (balPos[1]<3){
                    balPos[1]=3;
                }else if (balPos[1]>29){
                    balPos[1]=29;
                }

                u8g2.drawCircle(balPos[0],balPos[1],3);


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
        if (playersScore[0]==roundsWinForFinish){
            return;//player 1 win
        }else if (playersScore[1]==roundsWinForFinish){
            return;//player 2 win
        }
        delay(500);
    }
}






struct Obstacle {
    int x;
    int y;
    int type;
};

// Fonction pour dessiner le joueur
void drawPlayer(int x, int y) {
    // Exemple de dessin d'un joueur avec un dinosaure stylisé
    u8g2.drawBox(x, y, 6, 10); // Corps
    u8g2.drawBox(x - 2, y + 2, 2, 2); // Tête
    u8g2.drawBox(x + 6, y + 4, 2, 2); // Queue
}

// Fonction pour dessiner les obstacles
void drawObstacle(int x, int y, int type) {
    switch (type) {
        case 0:
            // Exemple de dessin d'un obstacle de type 0
            u8g2.drawBox(x, y, 10, 10);
            u8g2.drawLine(x, y, x + 10, y + 10);
            u8g2.drawLine(x, y + 10, x + 10, y);
            break;
        case 1:
            // Exemple de dessin d'un obstacle de type 1
            u8g2.drawCircle(x + 5, y + 5, 5, U8G2_DRAW_ALL);
            break;
        case 2:
            // Exemple de dessin d'un obstacle de type 2
            u8g2.drawTriangle(x, y + 10, x + 5, y, x + 10, y + 10);
            break;
    }
}

void dinoGame() {
    int dinoPos = 10;
    String dinoDirection = "";
    unsigned long lastSpawnTime = 0;
    unsigned long lastMoveTime = 0;
    int speed = 20; // Vitesse initiale
    int score = 0;
    std::vector<Obstacle> obstacles; // Utilisation de std::vector pour les obstacles

    while (true) {
        unsigned long currentTime = millis();

        if (currentTime - lastMoveTime > speed) {
            lastMoveTime = currentTime;

            // Move dinosaur
            if (dinoPos != 0 && dinoPos != 10 && dinoPos != 20) {
                if (dinoDirection == "left") {
                    dinoPos -= 1;
                } else if (dinoDirection == "right") {
                    dinoPos += 1;
                }
            } else {
                dinoDirection = "";
            }

            if (touchRead(button1) < 35) {
                if (dinoPos != 0) {
                    dinoDirection = "left";
                    dinoPos -= 1;
                }
            } else if (touchRead(button2) < 35) {
                if (dinoPos != 20) {
                    dinoDirection = "right";
                    dinoPos += 1;
                }
            }

            // Move obstacles
            for (size_t i = 0; i < obstacles.size(); i++) {
                obstacles[i].x -= 1;
                if (obstacles[i].x < 0) {
                    obstacles.erase(obstacles.begin() + i);
                    i--;
                    score++;
                }
            }

            // Check collision
            for (size_t i = 0; i < obstacles.size(); i++) {
                if (obstacles[i].x < 21 && obstacles[i].x > 4 && obstacles[i].y == dinoPos) {
                    return; // Game over
                }
            }

            // Draw everything
            u8g2.clearBuffer();
            drawPlayer(15, dinoPos);
            for (size_t i = 0; i < obstacles.size(); i++) {
                drawObstacle(obstacles[i].x, obstacles[i].y, obstacles[i].type);
            }
            u8g2.setFont(u8g2_font_ncenB08_tr);
            u8g2.drawStr(0, 10, String(score).c_str());
            u8g2.sendBuffer();
        }

        delay(10);

        // Increase difficulty over time by spawning more obstacles more frequently
        if (currentTime - lastSpawnTime > 1000 - (score * 10)) { // Increase spawn rate with score
            lastSpawnTime = currentTime;
            int newY = (rand() % 3) * 10;

            // Ensure there's always a way to dodge
            bool canSpawn = true;
            for (size_t i = 0; i < obstacles.size(); i++) {
                if (obstacles[i].x > 100 && obstacles[i].x < 128 && obstacles[i].y == newY) {
                    canSpawn = false;
                    break;
                }
            }

            if (canSpawn) {
                int obstacleType = rand() % 3; // Trois types d'obstacles différents
                obstacles.push_back({128, newY, obstacleType});
                if (speed > 2) {
                    speed -= 2; // Increase difficulty by reducing the speed delay
                }
            }
        }
    }
}




void geometryDash(){
    const int screenWidth = 128;
    const int screenHeight = 32;

    // Paramètres du jeu
    const int groundLevel = screenHeight - 5;
    const int cubeSize = 5;
    int cubeX = 10;
    int cubeY = groundLevel - cubeSize;
    int cubeVelocityY = 0;
    const int gravity = 1;
    const int jumpStrength = -7;
    bool isJumping = false;
    int cubeRotation = 0;

    const int obstacleWidth = 5;
    const int spikeHeight = 5;
    const int maxObstacles = 5;
    int obstacleX[maxObstacles];
    int obstacleY[maxObstacles];
    bool isSpike[maxObstacles];

    // Variables de jeu
    bool gameRunning = true;

    for (int i = 0; i < maxObstacles; i++) {
        obstacleX[i] = screenWidth + i * (screenWidth / maxObstacles);
        obstacleY[i] = groundLevel - obstacleWidth;
        isSpike[i] = random(2) == 0; // Aléatoirement spike ou bloc
    }

    while (true){
        if (gameRunning) {
            // Logique du cube
            if (isJumping) {
            cubeVelocityY += gravity;
            cubeY += cubeVelocityY;

            // Arrêter de sauter et remettre le cube au niveau du sol
            if (cubeY >= groundLevel - cubeSize) {
                cubeY = groundLevel - cubeSize;
                isJumping = false;
                cubeVelocityY = 0;
                cubeRotation = 0;
            } else {
                cubeRotation = (cubeRotation + 1) % 4; // Rotation du cube
            }
            }

            // Déplacement des obstacles
            for (int i = 0; i < maxObstacles; i++) {
            obstacleX[i] -= 2; // Vitesse de l'obstacle
            if (obstacleX[i] < -obstacleWidth) {
                obstacleX[i] = screenWidth;
                obstacleY[i] = groundLevel - (random(3) + 1) * obstacleWidth; // Hauteur aléatoire
                isSpike[i] = random(2) == 0; // Aléatoirement spike ou bloc
            }

            // Vérification des collisions
            if (cubeX < obstacleX[i] + obstacleWidth && cubeX + cubeSize > obstacleX[i] &&
                cubeY + cubeSize > obstacleY[i] && cubeY < obstacleY[i] + (isSpike[i] ? spikeHeight : obstacleWidth)) {
                gameRunning = false; // Collision détectée, fin du jeu
            }
            }

            // Affichage
            u8g2.clearBuffer();
            u8g2.drawFrame(0, 0, screenWidth, screenHeight); // Cadre du jeu

            // Dessiner le cube
            u8g2.drawBox(cubeX, cubeY, cubeSize, cubeSize); 

            // Dessiner les obstacles
            for (int i = 0; i < maxObstacles; i++) {
            if (isSpike[i]) {
                // Dessiner un pic
                u8g2.drawTriangle(obstacleX[i], groundLevel, obstacleX[i] + obstacleWidth / 2, groundLevel - spikeHeight, obstacleX[i] + obstacleWidth, groundLevel);
            } else {
                // Dessiner un bloc
                u8g2.drawBox(obstacleX[i], obstacleY[i], obstacleWidth, obstacleWidth);
            }
            }

            u8g2.sendBuffer();
        } else {
            // Affichage de l'écran de fin
            u8g2.clearBuffer();
            u8g2.drawStr(10, 16, "Game Over");
            u8g2.sendBuffer();
        }

        // Gérer les entrées
        if ((touchRead(button1) < 35 || touchRead(button2) < 35) == HIGH && !isJumping) { // Assurez-vous de configurer correctement le bouton
            isJumping = true;
            cubeVelocityY = jumpStrength;
        }
    }
}










const int screenWidth = 128;
const int screenHeight = 32;


bool isSafe(int sudoku[][9], int row, int col, int num, int size) {
  for (int x = 0; x < size; x++) {
    if (sudoku[row][x] == num || sudoku[x][col] == num) {
      return false;
    }
  }
  int startRow = row - row % 3, startCol = col - col % 3;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      if (sudoku[i + startRow][j + startCol] == num) {
        return false;
      }
    }
  }
  return true;
}

bool solveSudoku(int sudoku[][9], int row, int col, int size) {
  if (row == size - 1 && col == size) {
    return true;
  }
  if (col == size) {
    row++;
    col = 0;
  }
  if (sudoku[row][col] != 0) {
    return solveSudoku(sudoku, row, col + 1, size);
  }
  for (int num = 1; num <= size; num++) {
    if (isSafe(sudoku, row, col, num, size)) {
      sudoku[row][col] = num;
      if (solveSudoku(sudoku, row, col + 1, size)) {
        return true;
      }
    }
    sudoku[row][col] = 0;
  }
  return false;
}

void fillSudoku(int sudoku[][9], int size) {
  solveSudoku(sudoku, 0, 0, size);
}

void removeNumbers(int sudoku[][9], int size, int count) {
  while (count != 0) {
    int cellId = random(size * size);
    int row = cellId / size;
    int col = cellId % size;
    if (sudoku[row][col] != 0) {
      sudoku[row][col] = 0;
      count--;
    }
  }
}

void generateSudoku(int size, int difficulty, int sudoku[][9]) {
  // Initialiser la grille avec des zéros
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      sudoku[i][j] = 0;
    }
  }

  // Remplir la grille avec une solution complète
  fillSudoku(sudoku, size);

  // Appliquer la difficulté en enlevant certains nombres
  int cellsToRemove = (difficulty == 1) ? (size * size * 2 / 5) : (difficulty == 2) ? (size * size * 3 / 5) : (size * size * 4 / 5);
  removeNumbers(sudoku, size, cellsToRemove);
}


void displaySudoku(int sudoku[][9], int size) {
    if (IndexFocused>size){
        IndexFocused = size;
    }
    if (lastIndexFocused!=IndexFocused){

        int cellSize=14;

        int startY = -IndexFocused * cellSize; // Calculer la position de départ en fonction de l'index de défilement

        for (int transition=0; transition<cellSize; transition++){   
            int transition_=transition;
            if (IndexFocused<lastIndexFocused){
                transition_=-transition;
            }

            u8g2.clearBuffer();

            for (int i = 0; i < size; i++) {
                for (int j = 0; j < size; j++) {
                int x = j * cellSize;
                int y = startY + i * cellSize;
                    
                if (sudoku[i][j] != 0) {
                    char buffer[2];
                    sprintf(buffer, "%d", sudoku[i][j]);
                    u8g2.drawStr(x + 5, y + cellSize - 3+transition, buffer); // Ajuster l'espacement du texte dans la cellule
                }
                u8g2.drawFrame(x, y+transition, cellSize, cellSize); // Dessiner la bordure de la cellule
                }
                
            }
            u8g2.sendBuffer();
        }
        
        u8g2.sendBuffer();
    }
    lastIndexFocused = IndexFocused;
}


// Variables de défilement
int scrollX = 0;
int scrollY = 0;

void sudokuGenerator(){
    int sudoku[9][9] = {0};
    int size, difficulty;
    while (true){
        std::vector<String> OverviewDifficultyList={ "{\"text\":\"4x4 Easy\"}",
                                        "{\"text\":\"9x9 Easy\"}",
                                        "{\"text\":\"9x9 Medium\"}",
                                        "{\"text\":\"9x9 Hard\"}"
                                        };
        showPageOverview(OverviewDifficultyList);

        if (touchRead(button1) < 35 && touchRead(button2) < 35){
            size = (IndexFocused == 0) ? 4 : 9;
            difficulty = (IndexFocused == 1) ? 1 : (IndexFocused == 2) ? 2 : 3;
            break;
        }
    }
    while(touchRead(button1) < 35 || touchRead(button2) < 35){}
    delay(100); // Simule un délai pour la sélection

    generateSudoku(size, difficulty, sudoku);


    IndexFocused=0;
    // Affichage du Sudoku
    while (true){
        displaySudoku(sudoku, size);

        if (touchRead(button1) < 35 && touchRead(button2) < 35){
            return;
        }
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

        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_cu12_t_symbols);
        u8g2.drawGlyph(103, 20, 0x219c);
        u8g2.sendBuffer();

        delay(1000);

    }
}
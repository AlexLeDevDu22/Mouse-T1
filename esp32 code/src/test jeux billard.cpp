#include <Arduino.h>
#include <U8g2lib.h>

// Initialisation de l'écran OLED
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Variables du jeu
int playerCount = 1; // 1 ou 2 joueurs
int difficulty = 0; // Niveau de difficulté du bot (0: facile, 1: difficile)
int angle = 0; // Angle de visée de la queue de billard

const int screenWidth = 128;
const int screenHeight = 32;

// Structure pour représenter une boule
struct Ball {
  int x, y;
  bool isWhite;
  bool isEight;
  bool inPocket;
};

Ball balls[16]; // 15 boules + la blanche
Ball whiteBall;

// Initialisation des boules
void initBalls() {
  // Initialiser les positions des boules (exemple simple, peut être amélioré)
  int startX = screenWidth / 2;
  int startY = screenHeight / 2;
  for (int i = 0; i < 15; i++) {
    balls[i] = {startX + (i % 5) * 5, startY + (i / 5) * 5, false, (i == 7), false};
  }
  whiteBall = {screenWidth / 4, screenHeight / 2, true, false, false};
}

// Affichage de la table et des boules
void displayTable() {
  u8g2.clearBuffer();

  // Dessiner les bandes
  u8g2.drawFrame(0, 0, screenWidth, screenHeight);

  // Dessiner les trous
  u8g2.drawDisc(0, 0, 3);
  u8g2.drawDisc(screenWidth / 2, 0, 3);
  u8g2.drawDisc(screenWidth - 1, 0, 3);
  u8g2.drawDisc(0, screenHeight - 1, 3);
  u8g2.drawDisc(screenWidth / 2, screenHeight - 1, 3);
  u8g2.drawDisc(screenWidth - 1, screenHeight - 1, 3);

  // Dessiner les boules
  for (int i = 0; i < 15; i++) {
    if (!balls[i].inPocket) {
      u8g2.drawDisc(balls[i].x, balls[i].y, 2);
      if (balls[i].isEight) {
        u8g2.drawStr(balls[i].x - 1, balls[i].y + 1, "8");
      }
    }
  }
  if (!whiteBall.inPocket) {
    u8g2.drawDisc(whiteBall.x, whiteBall.y, 2);
  }

  // Dessiner la queue de billard
  int cueX = whiteBall.x + cos(angle * DEG_TO_RAD) * 10;
  int cueY = whiteBall.y + sin(angle * DEG_TO_RAD) * 10;
  u8g2.drawLine(whiteBall.x, whiteBall.y, cueX, cueY);

  u8g2.sendBuffer();
}

// Fonction de tir
void shoot() {
  // Calculer les nouvelles positions des boules (simplification)
  whiteBall.x += cos(angle * DEG_TO_RAD) * 2;
  whiteBall.y += sin(angle * DEG_TO_RAD) * 2;
  
  // Gestion des collisions (simplifiée)
  for (int i = 0; i < 15; i++) {
    if (!balls[i].inPocket && sqrt(sq(whiteBall.x - balls[i].x) + sq(whiteBall.y - balls[i].y)) < 4) {
      balls[i].inPocket = true;
    }
  }

  // Vérifier les collisions avec les bandes
  if (whiteBall.x <= 2 || whiteBall.x >= screenWidth - 2) {
    angle = 180 - angle;
  }
  if (whiteBall.y <= 2 || whiteBall.y >= screenHeight - 2) {
    angle = -angle;
  }
}

void setup() {
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  initBalls();

  // Affichage initial
  displayTable();
}

void loop() {
  // Exemple d'interaction avec les boutons
  if (digitalRead(2) == HIGH) { // Bouton 1 pour augmenter l'angle
    angle += 5;
    if (angle >= 360) angle -= 360;
  }
  if (digitalRead(3) == HIGH) { // Bouton 2 pour diminuer l'angle
    angle -= 5;
    if (angle < 0) angle += 360;
  }
  if (digitalRead(4) == HIGH) { // Bouton 3 pour tirer
    shoot();
  }

  // Afficher la table mise à jour
  displayTable();
  delay(100);
}

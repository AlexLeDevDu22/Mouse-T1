/*#define valueinclude <Arduino.h>
#include <U8g2lib.h>

#define button1 14
#define button2 12
#define SDA_PIN 13
#define SCL_PIN 15

// Initialisation de l'écran OLED
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Variables du jeu
int playerCount = 1; // 1 ou 2 joueurs
int difficulty = 0;  // Niveau de difficulté du bot (0: facile, 1: difficile)
int cueAngle = 0;       // Angle de visée de la queue de billard

const int screenWidth = 128;
const int screenHeight = 32;

// Structure pour représenter une boule
struct Ball
{
  float x, y;
  String type;
  float velocity;
  int angle;
  bool inPocket;
};

Ball balls[16]; // 15 boules + la blanche

void displayTable(float charging, bool showCue);
// Initialisation des boules
void initBalls()
{
  int ballIndex = 0;
  for (int collumn = 1; collumn <= 5; collumn++)
  {
    int step = (collumn - 1) * 2;
    for (int row = 0; row < collumn; row++)
    {
      String type;
      if (ballIndex == 4)
      {
        type = "eight";
      }
      else if (ballIndex % 2 == 0)
      {
        type = "full";
      }
      else
      {
        type = "empty";
      }
      balls[ballIndex] = {collumn * 4 + screenWidth / 3, screenHeight / 2 + (row * 4 - step), type, 0, 0, false};
      ballIndex++;
    }
  }

  balls[15] = {16, 16, "white", 0, 0, false};

  displayTable(0, true);
}

// Affichage de la table et des boules
void displayTable(float cueGap, bool showCue)
{
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
  for (int i = 0; i < 15; i++)
  {
    if (!balls[i].inPocket)
    {
      if (balls[i].type == "full") {
        u8g2.drawDisc((int)balls[i].x, (int)balls[i].y, 1);

      } else if (balls[i].type == "empty") {
        u8g2.drawCircle((int)balls[i].x, (int)balls[i].y, 1);
      }
      else if (balls[i].type == "eight")
      {
        u8g2.drawStr((int)balls[i].x - 1, (int)balls[i].y + 3, "8");
      }
    }
  }
  if (!balls[15].inPocket)
  {
    u8g2.drawDisc((int)balls[15].x, (int)balls[15].y, 2);
  }

  // Dessiner la queue de billard
  float cueX = balls[15].x + cos((cueAngle + 180 % 360) * DEG_TO_RAD) * 15;
  float cueY = balls[15].y + sin((cueAngle + 180 % 360) * DEG_TO_RAD) * 15;

  if (showCue) {
    float chargingX = 1;
    if (cueX < balls[15].x){
      chargingX = (10 - cueGap) / 10;
    }else if(cueX > balls[15].x){
      chargingX = (10 + cueGap) / 10;
    }

    float chargingY = 1;
    if (cueY < balls[15].y)
      chargingY = (10 - cueGap) / 10;
    else if (cueY > balls[15].y){
      chargingY = (10 + cueGap) / 10;
    }
    u8g2.drawLine((int)balls[15].x * chargingX, (int)balls[15].y * chargingY, cueX * chargingX, cueY * chargingY);

  }

  u8g2.sendBuffer();
}

bool areTouching(int O1X,int O1Y,int O2X, int O2Y, int diameter) {
  int distance = (int)std::sqrt(std::pow(O2X- O1X, 2) + std::pow(O2Y - O1Y, 2));
  return distance <= diameter;
}

void handleCollision(Ball &b1, Ball &b2) {
    // Calcul de la nouvelle direction après collision
    float dx = b2.x - b1.x;
    float dy = b2.y - b1.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    // Normalisation du vecteur de direction
    float nx = dx / distance;
    float ny = dy / distance;

    // Produit scalaire entre les vitesses et la normale au point de contact
    float p = 2 * (b1.velocity * nx * std::cos(b1.angle * PI / 180.0) +
                   b1.velocity * ny * std::sin(b1.angle * PI / 180.0) -
                   b2.velocity * nx * std::cos(b2.angle * PI / 180.0) -
                   b2.velocity * ny * std::sin(b2.angle * PI / 180.0));

    // Calcul des nouvelles vitesses basées sur l'échange d'impulsion
    float new_velocity_b1 = b1.velocity - p * nx;
    float new_velocity_b2 = b2.velocity + p * nx;

    // Vérification pour éviter l'augmentation de la vitesse totale
    float total_velocity_before = b1.velocity + b2.velocity;
    float total_velocity_after = std::abs(new_velocity_b1) + std::abs(new_velocity_b2);

    if (total_velocity_after > total_velocity_before) {
        float scale = total_velocity_before / total_velocity_after;
        new_velocity_b1 *= scale;
        new_velocity_b2 *= scale;
    }

    // Mise à jour des vitesses et angles
    b1.velocity = std::max(0.0f, new_velocity_b1);
    b2.velocity = std::max(0.0f, new_velocity_b2);
    
    // Mise à jour des angles avec une légère variation aléatoire
    b1.angle = std::atan2(ny, nx) * 180.0 / PI + (std::rand() % 3 - 1);
    b2.angle = std::atan2(-ny, -nx) * 180.0 / PI + (std::rand() % 3 - 1);
}
void updateBallPosition(Ball &ball) {
  if (ball.velocity > 0) {
    float radAngle = ball.angle * (PI / 180.0); // Conversion en radians
    ball.x += ball.velocity * std::cos(radAngle);
    ball.y += ball.velocity * std::sin(radAngle);

    // Simuler une légère réduction de la vitesse due à la friction
    ball.velocity *= 0.99f;
  }
}

// Fonction de tir
void shoot()
{
  balls[15].velocity = 1;
  while (touchRead(button1)<35 && touchRead(button2)<35)
  {
    if (balls[15].velocity < 8) balls[15].velocity++;
    displayTable(balls[15].velocity, true);
    delay(80);
  }
  for (int i; i <= balls[15].velocity; i++) {
    displayTable(balls[15].velocity - i, true);
    delay(80-i*5);
  }
  balls[15].angle = cueAngle;
  while (true)
  {
    for (int ball = 0; ball <= 15; ball++)
    {
      if (balls[ball].velocity > 0 && !balls[ball].inPocket)
      {

        for (size_t j = 0; j <= 15; ++j) {
          if (ball != j && areTouching((int)balls[ball].x, (int)balls[ball].y, balls[j].x, balls[j].y, 3)) {
            handleCollision(balls[ball], balls[j]);
          }
        }
        updateBallPosition(balls[ball]);

        // voir si il est tomber dans un trou
        struct Position
        {
          int x;
          int y;
        };
        Position holes[6] = {
          {0, 0},   // Coin supérieur gauche
          {64, 0},  // Milieu supérieur
          {128, 0}, // Coin supérieur droit
          {0, 32},  // Coin inférieur gauche
          {64, 32}, // Milieu inférieur
          {128, 32} // Coin inférieur droit
        };

        for (int i = 0; i < 6; ++i)
        {
          int dx = (int)balls[ball].x - holes[i].x;
          int dy = (int)balls[ball].y - holes[i].y;
          if (areTouching(balls[ball].x, balls[ball].y, holes[i].x, holes[i].y, 3))
          {
            Serial.println("fell");
            balls[ball].velocity=0;
  
            if (ball == 15) {
              balls[ball].x = 16;
              balls[ball].y = 16;
              balls[ball].angle = 0;
            }else balls[ball].inPocket = true;
          }
        }
        // Vérifier les collisions avec les bandes

        if ((int)balls[ball].x <= 2 && !balls[ball].inPocket)
        {
          balls[ball].angle = 180 - balls[ball].angle;
          balls[ball].x = 3;
          balls[ball].velocity*=0.9;
          
        } else if ((int)balls[ball].x >= screenWidth - 2) {
          balls[ball].angle = 180 - balls[ball].angle;
          balls[ball].x = screenHeight - 3;
          balls[ball].velocity*=0.9;
        }
        if ((int)balls[ball].y <= 2 && !balls[ball].inPocket)
        {
          balls[ball].angle = -balls[ball].angle;
          balls[ball].y = 3;
          balls[ball].velocity*=0.9;
        } else if ((int)balls[ball].y >= screenHeight - 2) {
          balls[ball].angle = -balls[ball].angle;
          balls[ball].y = screenHeight - 3;
          balls[ball].velocity*=0.9;
        }
      };
    }
    displayTable(0, false);

    int ballsStoped = 0;
    for (int i = 0; i < 16; i++)
    {
      if (balls[i].velocity <= 0)
      {
        ballsStoped++;
      }
      else
      {
        balls[i].velocity -= 0.2;
      }
    }
    Serial.println(ballsStoped );
    if (ballsStoped == 16)
      break;
  }
}

#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib> // Pour la génération d'aléatoire
#include <Wire.h>


//const float PI = 3.14159265;
//const float DIAMETER = 3.0;
//const float RADIUS = DIAMETER / 2.0;




void setup()
{
  Wire.begin(SDA_PIN, SCL_PIN);  
  Serial.begin(115200);

  u8g2.begin();
  u8g2.setFont(u8g2_font_tiny_simon_tr);
}

void loop()
{
  initBalls();
  while (true)
  {

    // Exemple d'interaction avec les boutons
    if (touchRead(button1)<35 && touchRead(button2) >35)
    { // Bouton 1 pour augmenter l'angle
      cueAngle = (cueAngle + 5) % 360;

      displayTable(0, true);
    }
    if (touchRead(button1)>35 && touchRead(button2) <35)
    { // Bouton 2 pour diminuer l'angle
      cueAngle = (cueAngle - 5) % 360;

      displayTable(0, true);
    }
    if (touchRead(button1)<35 && touchRead(button2) <35)
    { // Bouton 3 pour tirer
      shoot();
    }
  }
}
*/
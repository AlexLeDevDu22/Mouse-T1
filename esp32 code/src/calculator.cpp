/*#include <U8g2lib.h>
#include <Wire.h>

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0,  U8X8_PIN_NONE);

int IndexFocused = 0; // Indice de sélection
char result[16] = "0"; // Résultat ou expression en cours
bool calculating = false;

const char *buttons[4][4] = {
  {"7", "8", "9", "/"},
  {"4", "5", "6", "*"},
  {"1", "2", "3", "-"},
  {"0", "C", "=", "+"}
};

void setup() {
  Serial.begin(115200);
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr); // Police pour les chiffres
  pinMode(5, INPUT_PULLUP); // Bouton numérique
  pinMode(4, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
}

void loop() {
  u8g2.clearBuffer();  // Efface le tampon

  // Affiche le résultat
  u8g2.setCursor(0, 10);
  u8g2.print(result);

  // Dessine la grille des boutons
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      int x = j * 32;
      int y = i * 8 + 16;
      u8g2.setCursor(x + 10, y + 7);

      if (IndexFocused == i * 4 + j) {
        u8g2.drawBox(x, y, 32, 8); // Highlight
        u8g2.setDrawColor(0); // Inverse la couleur pour le focus
      }

      u8g2.print(buttons[i][j]);
      u8g2.setDrawColor(1); // Remet la couleur à normal
    }
  }

  u8g2.sendBuffer();  // Affiche le contenu du tampon

  if (digitalRead(5) == LOW) {
    delay(200); // Debounce

    handleButtonPress(IndexFocused);
  }

  // Déplace le focus (simulation du scrolling avec boutons)
  if (digitalRead(4)==LOW){
  IndexFocused = (IndexFocused+1)%16;
  }else if (digitalRead(6)==LOW)
  {
    IndexFocused = (IndexFocused-1)%16;
  }
  delay(100);
}

void handleButtonPress(int index) {
  char selected = buttons[index / 4][index % 4][0];

  if (selected >= '0' && selected <= '9') {
    appendToResult(selected);
  } else if (selected == 'C') {
    resetResult();
  } else if (selected == '=') {
    calculateResult();
  } else {
    appendToResult(selected);
  }
}

void appendToResult(char c) {
  if (calculating) {
    strcpy(result, "0");
    calculating = false;
  }
  if (strlen(result) < 15) {
    if (result[0] == '0' && result[1] == '\0') {
      result[0] = c;
      result[1] = '\0';
    } else {
      char str[2] = {c, '\0'};
      strcat(result, str);
    }
  }
}

void resetResult() {
  strcpy(result, "0");
}

void calculateResult() {
  // Implémentez votre propre parser simple pour l'expression
  // Ici, c'est une opération simple (vous pouvez ajouter plus de logique)
  int res = atoi(result); // Conversion de la chaîne en entier
  sprintf(result, "%d", res);
  calculating = true;
}

*/
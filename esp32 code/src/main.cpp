#include <U8g2lib.h>
#include <Wire.h>
#include <vector>
#include <iostream>
#include <cstring>
#include <cctype>
#include <stack>
#include <string>
#include <sstream>
#include <cmath>

#define button1 14
#define button2 12
#define SDA_PIN 13
#define SCL_PIN 15

int IndexFocused=0;

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

void updateIndexFocused(void * pvParameters) {
    while (true) {
        if (touchRead(button1) > 35 || touchRead(button2) > 35) {
            while (touchRead(button1) < 35 && touchRead(button2) > 35) {
                if (IndexFocused > 0) {
                    IndexFocused -= 1;
                    delay(500);
                }
            }
            while (touchRead(button1) > 35 && touchRead(button2) < 35) {
                IndexFocused += 1;
                delay(500);
            }
            delay(2);
        }
    }
}

char result[16] = "0"; 

std::vector<std::vector<char>> buttons= {
  {'0', '1', '2', '3','4', '5', '6','7', '8', '9'},
  {'+', '-', '*', '/',',','(',')','D', 'C', '='},
};
bool operators=false;

void appendToResult(char c) {
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

bool isOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/';
}

// Function to check operator precedence
int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

// Function to apply an operator to two operands
float applyOperator(float a, float b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return a / b;
    }
    return 0;
}

void calculateResult(const std::string& expression) {

    std::vector<std::string> output; // Output list for RPN
    std::stack<char> operators;      // Stack for operators
    std::string number;              // To accumulate multi-digit numbers

    // Tokenize the input expression
    for (size_t i = 0; i < expression.length(); ++i) {
        char c = expression[i];

        if (isdigit(c) || c == '.') {
            number += c; // Accumulate digits or decimal point
        } else {
            if (!number.empty()) {
                output.push_back(number); // Push the complete number to output
                number.clear();
            }

            if (c == '(') {
                operators.push(c); // Push '(' to the operator stack
            } else if (c == ')') {
                // Pop operators until '(' is encountered
                while (!operators.empty() && operators.top() != '(') {
                    output.push_back(std::string(1, operators.top()));
                    operators.pop();
                }
                operators.pop(); // Pop the '('
            } else if (isOperator(c)) {
                // Pop operators with higher or equal precedence
                while (!operators.empty() && precedence(operators.top()) >= precedence(c)) {
                    output.push_back(std::string(1, operators.top()));
                    operators.pop();
                }
                operators.push(c); // Push the current operator
            }
        }
    }

    // Push the remaining number if any
    if (!number.empty()) {
        output.push_back(number);
    }

    // Pop the remaining operators
    while (!operators.empty()) {
        output.push_back(std::string(1, operators.top()));
        operators.pop();
    }

    // Evaluate the RPN expression
    std::stack<float> values;
    for (const std::string& token : output) {
        if (isdigit(token[0]) || (token.length() > 1 && token[0] == '-')) {
            values.push(std::stof(token)); // Push numbers to the stack
        } else if (isOperator(token[0])) {
            float b = values.top(); values.pop();
            float a = values.top(); values.pop();
            values.push(applyOperator(a, b, token[0])); // Apply the operator
        }
    }

  // Implémentez votre propre parser simple pour l'expression
  // Ici, c'est une opération simple (vous pouvez ajouter plus de logique)
  float res = values.top(); // Conversion de la chaîne en entier
  sprintf(result, "%d", res);

  Serial.println(result);

}

void removeLastChar() {
    size_t len = strlen(result);
    if (len > 0) {
        result[len - 1] = '\0'; // Remove the last character
    }
}

void handleButtonPress(bool operators, int index) {
  char selected = buttons[operators ? 1 : 0][index];

  if (selected == 'D') {
    removeLastChar();
  } else if (selected == 'C') {
    strcpy(result, "0");
  } else if (selected == '=') {
    calculateResult(std::string(result));
  } else {
    appendToResult(selected);
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr); // Police pour les chiffres
  xTaskCreate(updateIndexFocused, "updateIndexFocused", 2048, NULL, 1, NULL);

}

void loop() {
  u8g2.clearBuffer();  // Efface le tampon

  // Affiche le résultat
  u8g2.setCursor(0, 10);
  u8g2.print(result);

  if (IndexFocused>buttons[operators ? 1 : 0].size()-1) IndexFocused = buttons[operators ? 1 : 0].size()-1;

  // Dessine la grille des boutons
  for (int i = 0; i < 2; i++) {
    u8g2.drawLine(0,14+i*8,128,14+i*8);
    int xScale = 0;
    if (buttons[i].size()>8 && i == operators ? 1 : 0) {

        if (IndexFocused>4){
            xScale  = IndexFocused-4;
            if (xScale+8>buttons[i].size()) xScale = buttons[i].size()-8;
        }
    }
    for (int j = 0; j < buttons[i].size(); j++) {
      int x = (j-xScale) * 16;
      int y = i * 8 + 15;
      u8g2.drawLine(x,y,x,y+8);

      u8g2.setCursor(x + 5, y + 8);

      if (IndexFocused == j && i==(operators ? 1 : 0)) {
        u8g2.drawBox(x, y, 16, 8); // Highlight
        u8g2.setDrawColor(0); // Inverse la couleur pour le focus
      }

      u8g2.print(buttons[i][j]);
      u8g2.setDrawColor(1); // Remet la couleur à normal
    }
  }
  u8g2.drawLine(0,31,128,31);


  u8g2.sendBuffer();  // Affiche le contenu du tampon

    int timeClicked=0;
    while (touchRead(button1) < 35 && touchRead(button2) < 35){
        timeClicked++;
        delay(1);
    }
    if (timeClicked>400) operators=!operators;
    else if (timeClicked>0)handleButtonPress(operators, IndexFocused);
  
  delay(100);
}
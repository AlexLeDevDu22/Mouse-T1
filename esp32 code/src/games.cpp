void morpionGame(){
    bool player1Play = true;
    int xTransition = 0;
    std::vector<char*> tableValues = {" ", " ", " "
                                                              " ", " ", " "
                                                              " ", " ", " "};
    
    while (true){
        if (!veilleMode){
            u8g2.clearBuffer()
            
            if (player1Play){
              int xOffset=0;
            }else {
              int xOffset=96;
            }
            
            for (int collumnBorder = 0, collumnBorder<4, collumnBorder++){
              
              u8g2.drawBox(xOffset+xTransition+collumnBorder*8, 0, 1, 32);
              
            }
            
            for (int rowBorder = 0, rowBorder<4, rowBorder++){
              
              u8g2.drawBox(xOffset+xTransition, rowBorder * 8, 32, 1);
              
            }
            
            for (int x = 0, x<3,x++){
              for (int y = 0, y<3, y++){
 
               if (IndexFocused == x+y*3){
                   u8g2.drawBox(xOffset + xTransition + x*8, y*8,8,8);
                   u8g2.setDrawColor(0)
               }
               
               u8g2.drawStr(xOffset + xTransition + x*8, y*8,tableValues[x+y*3]);
               
               u8g2.setDrawColor(1)
              }
            }
            u8g2.sendBuffer();
            
            if (xTransition!=0){
                if (player1Play){
                    xTransition--:
                }else{
                    xTransition++;
                }
                if (xTransition == 96 | xTransition == -96){
                  xTransition=0;
                }else{
                  delay(20);
                }
            }
            
             if (IndexFocused>8){
               IndexFocused=8;
             }
            
            if (touchRead(button1) < 35 && touchRead(button2) < 35     &&      xTransition==0) {
               if (tableValue[IndexFocused]==" "){
                   if (player1Play){
                     tableValue[IndexFocused]="X";
                     xTransition=1;
                   }else {
                     tableValue[IndexFocused]="O":
                   }
                   player1Play= !player1Play;
               }
            }
            
        
        }else{
           u8g2.clearBuffer();
           u8g2.sendBuffer();
        }
        
    }
}



























#include <iostream>
#include <vector>
#include <string>

using namespace std;

// Vérifie si une ligne ou colonne ou diagonale contient une séquence continue de symboles du joueur
bool checkLine(const vector<string>& board, int startX, int startY, int dx, int dy, const string& player, int length, int& startXOut, int& startYOut, int& endXOut, int& endYOut) {
    int count = 0;
    int x = startX, y = startY;
    for (int i = 0; i < length; ++i) {
        if (x < 0 || x >= 3 || y < 0 || y >= 3 || board[y * 3 + x] != player) {
            return false;
        }
        if (count == 0) {
            startXOut = x;
            startYOut = y;
        }
        endXOut = x;
        endYOut = y;
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

// Vérifie s'il y a une victoire en regardant uniquement les lignes qui contiennent le dernier coup
bool checkForWin(const vector<string>& board, int lastMoveIndex, int& startXOut, int& startYOut, int& endXOut, int& endYOut) {
    int lastMoveX, lastMoveY;
    indexToCoordinates(lastMoveIndex, lastMoveX, lastMoveY);
    string player = board[lastMoveIndex];
    
    // Vérifier la ligne horizontale
    if (checkLine(board, 0, lastMoveY, 1, 0, player, 3, startXOut, startYOut, endXOut, endYOut) ||
        checkLine(board, 0, lastMoveY, -1, 0, player, 3, startXOut, startYOut, endXOut, endYOut)) {
        return true;
    }
    
    // Vérifier la colonne verticale
    if (checkLine(board, lastMoveX, 0, 0, 1, player, 3, startXOut, startYOut, endXOut, endYOut) ||
        checkLine(board, lastMoveX, 0, 0, -1, player, 3, startXOut, startYOut, endXOut, endYOut)) {
        return true;
    }
    
    // Vérifier la diagonale (haut-gauche à bas-droit)
    if (lastMoveX == lastMoveY && checkLine(board, 0, 0, 1, 1, player, 3, startXOut, startYOut, endXOut, endYOut)) {
        return true;
    }
    
    // Vérifier la diagonale (haut-droit à bas-gauche)
    if (lastMoveX + lastMoveY == 2 && checkLine(board, 2, 0, -1, 1, player, 3, startXOut, startYOut, endXOut, endYOut)) {
        return true;
    }
    
    return false;
}

int main() {
    vector<string> board = { " ", " ", " ", "X", "X", "X", " ", " ", " " }; // Exemple de plateau
    int lastMoveIndex = 4; // Exemple d'index du dernier coup
    int startX, startY, endX, endY;
    
    if (checkForWin(board, lastMoveIndex, startX, startY, endX, endY)) {
        cout << "Win detected from (" << startX << "," << startY << ") to (" << endX << "," << endY << ")" << endl;
    } else {
        cout << "No win detected." << endl;
    }
    
    return 0;
}
void morpionGame(){



  void drawTable(int xOffset, vector<string> tableValues){

            for (int collumnBorder = 0, collumnBorder<4, collumnBorder++){
           	u8g2.drawBox(xOffset+collumnBorder*8, 0, 1, 32);
    	    }
            
            for (int rowBorder = 0, rowBorder<4, rowBorder++){
              
              u8g2.drawBox(xOffset, rowBorder * 8, 32, 1);
              
            }
            
            for (int x = 0, x<3,x++){
              for (int y = 0, y<3, y++){
 
               if (IndexFocused == x+y*3){
                   u8g2.drawBox(xOffset + x*8, y*8,8,8);
                   u8g2.setDrawColor(0)
               }
               
               u8g2.drawStr(xOffset + x*8, y*8,tableValues[x+y*3]);
               
               u8g2.setDrawColor(1)
              }
            }
            u8g2.sendBuffer();


  }


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





    bool player1Play = true;
 
    vector<string> tableValues = {" ", " ", " ",
                                  " ", " ", " ",
                                  " ", " ", " "};
    
    while (true){
        u8g2.clearBuffer()
        if (!veilleMode){
            
            
            if (player1Play){
              int xOffset=0;
            }else {
              int xOffset=96;
            }
          
           drawTable(xOffset,tableValues);
           
            
             if (IndexFocused>8){
               IndexFocused=8;
             }
            
            if (touchRead(button1) < 35 && touchRead(button2) < 35) {
              while (touchRead(button1) < 35 || touchRead(button2) < 35){}
               if (tableValue[IndexFocused]==" "){
                   if (player1Play){
                     tableValue[IndexFocused]="X";
                   }else {
                     tableValue[IndexFocused]="O";
                   }
                   player1Play= !player1Play;

                 //Check for win

                 int startX, startY, endX, endY;
                 if (checkForWin(tableValues, IndexFocused, startX, startY, endX, endY)) {

                  //Anim for move table to the center
                  for (int i = 1, i<=48, i++){
                    if (!player1Play){
                       i=-i;
                    }

                    u8g2.clearBuffer();
                    drawTable(xOffset+i,tableValues);
                    delay(20);

                  }
                  
                  if (startX==endX || startY == endY){
                    int animLinePixels = 24;
                  }else{
                    int animLinePixels = 34;
                  }

                  for (int i = 1, i<=animLinePixels,i++){
                    int coefLonguer = i/animLinePixels;

                    startX=48+startX*8+4;
                    startY=startY*8+4;
                    startX=48+endX*8+4;
                    endY=endY*8+4;

                    u8g2.drawLine(startX,startY,(endX-startX)*coefLonguer+startX, (endY, startY)*coefLonguer+startX);
                    delay(20);
                  }
                  
                  delay(500);
                  
                  while (true){
                    u8g2.clearBuffer();
                    drawTable(48, tableValues);
                    delay(100);
                    u8g2.drawLine(startX,startY,endX,endY);
                    u8g2.sendBuffer();
                    delay(100)
                    if (touchRead(button1) < 35 || touchRead(button2) < 35){
                      return;
                    }
                  }

                 } else {
                   //animation for change side of table
                   for (int i = 1, i<=96, i++){
                     if (!player1Play){
                        i=-i;
                      }

                      drawTable(xOffset+1,tableValues);
                   }
                   }
               }
            }
            
        
        }else{
          u8g2.sendBuffer();
        }
        
    }
}




void pongGame(){
	int yPlayer1Pos=0;
	int yPlayer2Pos=0;

  int balPosX = 64;
  int balPosY = 32;

	while (true){
  u8g2.clearBuffer();
		if (!veilleMode){
			
			
      if (touchRead(button1)<35){
        if (yPlayer1Pos<22){
          yPlayer1Pos++;
        }
      }else if (yPlayer1Pos>1){
        yPlayer1Pos--;
      }
      
      if (touchRead(button2)<35){
        if (yPlayer2Pos<22){
          yPlayer2Pos++;
        }
      }else if (yPlayer2Pos>1){
        yPlayer2Pos--;
      }


      u8g2.drawBox(0,yPlayer1Pos,2,10);
		  u8g2.drawBox(127,yPlayer2Pos,2,10);
      
			u8g2.drawCircle(balPosX,balPosY,3);



		}
    u8g2.sendBuffer();
	}
}
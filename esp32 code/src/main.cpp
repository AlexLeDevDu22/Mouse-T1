/*#include <U8g2lib.h>
#include <vector>

#define button1 14
#define button2 12
#define SDA_PIN 13
#define SCL_PIN 15

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);


int IndexFocused = 0;

class Card {
  public:
    char colors;
    int values;

    Card(std::string c, int v) : coulor(c), value(v) {}
};

std::vector<Card> generateRandomGame() {
  std::vector<Card> jeu;
  std::vector<char> colors = {"Coeur", "Carreau", "Trèfle", "Pique"};
  std::vector<std::pair<char, int>> values = {
    {"7", 0}, {"8", 0}, {"9", 14}, {"10", 10}, {"Valet", 20}, {"Dame", 3}, {"Roi", 4}, {"As", 11}
  };

  // Générer les 32 cartes
  for (const auto& colors : values) {
    for (const auto& cards : card) {
      jeu.emplace_back(colors, card.first, card.second);
    }
  }

  // Mélanger les cartes
  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(jeu.begin(), jeu.end(), g);

  return jeu;
}

bool cardIsPlacable(Card card, std::vector<std::Card> cardsPlaced , char atout, std::vector<std::vector<Card>> deck){
    if (cardPlaced[0]["colors"]==card["colors"]) return true;
    if (cardPlaced[0]["colors"]==atout) {
        for 
    }

}


void setup() {
  Serial.begin(115200);
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr); // Police pour les chiffres
  Wire.begin(SDA_PIN, SCL_PIN);  
  u8g2.setFont(u8g2_font_tiny_simon_tr);
}

void loop() {

  while (touchRead(button1)>35 && touchRead(button2) >35) {
    u8g2.clearBuffer();
    u8g2.drawStr(10, 10, "difficulté des adversaires:");
    u8g2.drawStr(20, 20, char(IndexFocused % 10));
    u8g2.sendBuffer();
  }
  int botDificult = IndexFocused % 10;

  while (touchRead(button1)>35 && touchRead(button2) >35) {
    u8g2.clearBuffer();
    u8g2.drawStr(10, 10, "nombre de tours: ");
    if (IndexFocused % 12 != 0) {
      u8g2.drawStr(20, 20, char(IndexFocused % 12));
    } else {
      u8g2.drawStr(20, 20, "à la demande");
    }
    u8g2.sendBuffer();
  }
  int roundsMax = IndexFocused % 12;

  int currentRound = 1;
  std::vector<int> totalTeamsScores = {0, 0};
  while (true) { //each rounds
    int firstPlayerPlay = millis() % 3;
    Char currentAtout = "";
    int teamsTaked;
    int numberBeloteSaid = 0;
    int beloteTeam;
    std::vector<Card> cardsGame = generateRandomGame();
    std::vector<std::vector<Card>> playersDecks;
    while (true) {
      for (int player = 0; player < 4; player++) { //distribution for each player
        for (int i = 0; i < 5; i++) { //add 5 cards
          if (player == 0) {
            delay(500);
            //TODO animation for add card in the deck
          }

          playersDecks[player].push_back(cardsGame[0]);
          cardGame = cardsGame.erase(0);
        }
      }
      bool isTake = false;

      for (int atoutChoiceType = 1; atoutChoiceType <= 2; atoutChoiceType++) { // first step for say ONE et sexond for say TWO
        for (int player = firstPlayerPlay % 3; player < firstPlayerPlay % 3 + 4; player++) {
          if (!isTake) {
            if (player % 3 == 0) { //if reel player must to choice
              while (true) {
                u8g2.clearBuffer();
                cardsGame[0]//TODO show card
                if (atoutChoiceType == 1) {
                  u8g2.drawStr(20, 20, ":veut tu prendre? (<- passer, * prendre)");
                } else {
                  //TODO made display for choice what type of card or skip
                }
                u8g2.sendBuffer();

                if (touchRead(button1)<35 && touchRead(button2) <35) {
                  if (atoutChoiceType == 1 || IndexFocused % 4 > 0) isTake = true;
                } else if (touchRead(button1)<35 || touchRead(button2) <35 && (touchRead(button1)<35)!=(touchRead(button2) <35)) {
                  if (atoutChoiceType == 1) break;
                }
                if (isTake) {
                  std::vector<String> listAtout = {"Coeur", "Carreau", "Trèfle", "Pique"};
                  if (atoutChouceType == 1)
                    currentAtout = ; cardsGame[0].colors;
                  else currentAtout = listAtout[IndexFocused % 4 - 1];
                  playersDecks.push_back(gardsGame[0]);
                  cardsGames.erase(0);
                  break;
                }
              }
            } else {
              //show display
              delay(500);
              //TODO made bot choice if it take
            }
            teamsTaked = player % 3 % 2;
          }
        }
      }
      if (isTake) break;

      u8g2.clearBuffer();
      u8g2.drawStr(20, 20, "personne n'a pris, relancement");
      u8g2.sendBuffer();
      delay(1000);
    }
    u8g2.clearBuffer();
    u8g2.drawStr(20, 20, "C'est partie !");
    u8g2.sendBuffer();
    delay(1000);

    for (int player = 0; player < 4; player++) { // distribution of the rest
      while (playersDecks[player].size() < 8) { //TODO made a while player have not 8 cards
        if (player == 0) {
          delay(500);
          //TODO animation for add card in his deck
        }
        playersDecks[player].push_back(cardsGame[0])
        cardsGame.erase(0);

      }
    }
    //TODO animations for sort cards
    while (touchRead(button1) <35 || touchRead(button2) <35) {}

    std::vector<int> splitTeamsScores = {0, 0};
    for (int split = 1; split <= 8; split++) { //each rounds in the round
      std::vector<int> pliTeamsScores = {0, 0};
      std::vector<Card> cardsPlaced = {};
      for (int player = firstPlayerPlay % 3; player < firstPlayerPlay % 3 + 4; player++) { // for each players
        if (player % 3 == 0) { // if reel player must to play
          bool playerPlayed = false;
          while (!playerPlayed) {
            //TODO show display with deck and card in the table
            if (touchRead(button1) <35 && touchRead(button2) <35) { // if player choiced his card
              int cardSelected = IndexFocused;
              int timeClicked = 0;
              while (touchRead(button1) <35 && touchRead(button2) <35) {
                timeClicked++;
              }
              if (timeClicked > 20000) { //if long click
                if (cardSelected == Card(currentAtout, 5) || cardSelectes == Card(currentAtout, 6)) { //TODO if card choiced is Belote or reBelote
                  numberBeloteSaid++;
                  beloteTeam = player % 3 % 2;

                } else {
                  continue;
                }
              }
              if (cardIsPlacable(cardSelected, cardsPlaced, currentAtout, playersDecks[player]))continue;//TODO if card can't be play(is correct)
              cardsPlaced.push_back(playersDecks[player % 3][cardsSelected]);
              playersDecks[player % 3].erase(cardSelected);
              for (int i = 0; i < 4; i++) { //for each card played in the pli
                pliTeamsScores[i % 2] += ; //TODO calcul and add point to pli
              }
              playerPlayed = true;
            }
          }

        } else {
          //TODO show display
          delay(1000);
          //TODO make bot play
        }
        //TODO animation for place card
      }
      //TODO show scores of the split
      for (int i = 0; i < 8; i++) {
        splitTeamsScores[0] += pliTeamsScores[0];
        splitTeamsScores[1] += pliTeamsScores[1];
      }
    }
    totalTeamsScores[0] += splitTeamsScores[0];
    totalTeamsScores[1] += splitTeamsScores[1];


    if (currentRound >= roundsMax && roundsMax > 0) { // If all rounds are did
      u8g2.clearBuffer();
      u8g2.drawStr(20, 20, "fin de la partie");
      u8g2.sendBuffer();
      delay(1000);
      break;
    }
    if (roundsMax==0){
      //TODO ask if the player want to finish the game
    }

    currentRound++;
    firstPlayerPlay++;
  }
  //TODO show results and winners
}

*/












































#include <U8g2lib.h>
#include <vector>
#include <algorithm>
#include <random>
#include <Wire.h>

#define button1 14
#define button2 12
#define SDA_PIN 13
#define SCL_PIN 15

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

int IndexFocused = 0;


void updateIndexFocused(void *pvParameters) {
    while(true){
            if (touchRead(button1) > 35  || touchRead(button2) > 35){
                while (touchRead(button1) < 35 && touchRead(button2) > 35) {
                    if (IndexFocused>0){
                        IndexFocused-=1;
                        delay(500);

                    }
                }
                while(touchRead(button1)> 35 && touchRead(button2) < 35){
                    IndexFocused+=1;
                    delay(500);
                }
                delay(2);
            }
        
    }
}

class Card {
public:
    std::string colors;
    std::string name;
    int value;

    Card(std::string c, std::string n, int v) : colors(c), name(n), value(v) {}
};

std::vector<Card> generateRandomGame() {
    std::vector<Card> jeu;
    std::vector<std::string> colors = {"Coeur", "Carreau", "Trèfle", "Pique"};
    std::vector<std::pair<std::string, int>> values = {
        {"7", 0}, {"8", 0}, {"9", 14}, {"10", 10}, {"Valet", 20}, {"Dame", 3}, {"Roi", 4}, {"As", 11}
    };

    // Générer les 32 cartes
    for (const auto& color : colors) {
        for (const auto& card : values) {
            jeu.emplace_back(color, card.first, card.second);
        }
    }

    // Mélanger les cartes
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(jeu.begin(), jeu.end(), g);

    return jeu;
}

bool cardIsPlacable(Card card, std::vector<Card> cardsPlaced, std::string atout, std::vector<Card> playerDeck) {
    if (cardsPlaced.empty()) return true; // Si c'est la première carte du pli
    std::string couleurDemandee = cardsPlaced[0].colors;

    if (card.colors == couleurDemandee) return true; // Si la carte a la même couleur que celle demandée

    // Si la carte est un atout
    if (card.colors == atout) {
        // Si le joueur n'a pas la couleur demandée
        for (const auto& c : playerDeck) {
            if (c.colors == couleurDemandee) return false;
        }
        return true;
    }

    // Si le joueur n'a ni la couleur demandée ni de l'atout
    for (const auto& c : playerDeck) {
        if (c.colors == couleurDemandee || c.colors == atout) return false;
    }

    return true;
}

void displayCard(Card card, int x, int y) {
    char buffer[20];
    sprintf(buffer, "%s %s", card.colors.c_str(), card.name.c_str());
    u8g2.drawStr(x, y, buffer);
}

void setup() {
    Wire.begin(SDA_PIN, SCL_PIN);
    Serial.begin(115200);
    u8g2.begin();
    u8g2.setFont(u8g2_font_tiny_simon_tr);

    xTaskCreate(updateIndexFocused, "updateIndexFocused", 2048, NULL, 1, NULL);
}

void loop() {
    while (touchRead(button1) > 35 || touchRead(button2) > 35) {
        u8g2.clearBuffer();
        u8g2.drawStr(10, 10, "Difficulté des adversaires:");
        char diffText[2];
        sprintf(diffText, "%d", IndexFocused % 10);
        u8g2.drawStr(20, 20, diffText);
        u8g2.sendBuffer();
        delay(10);
        Serial.println(diffText);
    }
    int botDificult = IndexFocused % 10;

    while (touchRead(button1) > 35 || touchRead(button2) > 35) {
        u8g2.clearBuffer();
        u8g2.drawStr(10, 10, "Nombre de tours: ");
        char roundText[12];
        if (IndexFocused % 12 != 0) {
            sprintf(roundText, "%d", IndexFocused % 12);
            u8g2.drawStr(20, 20, roundText);
        } else {
            u8g2.drawStr(20, 20, "à la demande");
        }
        u8g2.sendBuffer();
    }
    int roundsMax = IndexFocused % 12;

    int currentRound = 1;
    std::vector<int> totalTeamsScores = {0, 0};
    while (true) { //each rounds
        int firstPlayerPlay = millis() % 4; // Changement de 3 à 4 pour inclure 4 joueurs
        std::string currentAtout = "";
        int teamsTaked;
        int numberBeloteSaid = 0;
        int beloteTeam;
        std::vector<Card> cardsGame = generateRandomGame();
        std::vector<std::vector<Card>> playersDecks(4);

        while (true) {
            for (int player = 0; player < 4; player++) { // Distribution pour chaque joueur
                for (int i = 0; i < 5; i++) { // Ajout de 5 cartes
                    if (player == 0) {
                        delay(500);
                        // Animation for adding card to the deck
                        u8g2.clearBuffer();
                        char msg[20];
                        sprintf(msg, "Carte pour vous: %s %s", cardsGame[0].colors.c_str(), cardsGame[0].name.c_str());
                        u8g2.drawStr(10, 20, msg);
                        u8g2.sendBuffer();
                    }

                    playersDecks[player].push_back(cardsGame[0]);
                    cardsGame.erase(cardsGame.begin());
                }
            }
            bool isTake = false;

            for (int atoutChoiceType = 1; atoutChoiceType <= 2; atoutChoiceType++) { // Étape 1 pour dire ONE et deuxième pour dire TWO
                for (int player = firstPlayerPlay; player < firstPlayerPlay + 4; player++) {
                    int currentPlayer = player % 4; // Ajustement pour les 4 joueurs
                    if (!isTake) {
                        if (currentPlayer == 0) { // Si c'est le joueur réel qui doit choisir
                            while (true) {
                                u8g2.clearBuffer();
                                // Afficher la carte pour laquelle il doit choisir
                                displayCard(cardsGame[0], 10, 10);
                                if (atoutChoiceType == 1) {
                                    u8g2.drawStr(20, 20, ":veut tu prendre? (<- passer, * prendre)");
                                } else {
                                    // Afficher pour choisir le type de carte ou passer
                                    u8g2.drawStr(20, 20, ":choisir une couleur? (<- passer, * choisir)");
                                }
                                u8g2.sendBuffer();

                                if (touchRead(button1) < 35 && touchRead(button2) < 35) {
                                    if (atoutChoiceType == 1 || IndexFocused % 4 > 0) {
                                        isTake = true;
                                    }
                                } else if ((touchRead(button1) < 35) || (touchRead(button2) < 35)) {
                                    if (atoutChoiceType == 1) break;
                                }
                                if (isTake) {
                                    std::vector<std::string> listAtout = {"Coeur", "Carreau", "Trèfle", "Pique"};
                                    if (atoutChoiceType == 1)
                                        currentAtout = cardsGame[0].colors;
                                    else
                                        currentAtout = listAtout[IndexFocused % 4];
                                    playersDecks[currentPlayer].push_back(cardsGame[0]);
                                    cardsGame.erase(cardsGame.begin());
                                    break;
                                }
                            }
                        } else {
                            delay(500);
                            // Choix du bot s'il prend
                            if (botDificult > 5 && std::rand() % 2 == 0) {
                                isTake = true;
                                currentAtout = cardsGame[0].colors;
                                playersDecks[currentPlayer].push_back(cardsGame[0]);
                                cardsGame.erase(cardsGame.begin());
                                teamsTaked = currentPlayer % 2;
                            }
                        }
                    }
                }
            }
            if (isTake) break;

            u8g2.clearBuffer();
            u8g2.drawStr(20, 20, "personne n'a pris, relancement");
            u8g2.sendBuffer();
            delay(1000);
        }
        u8g2.clearBuffer();
        u8g2.drawStr(20, 20, "C'est parti !");
        u8g2.sendBuffer();
        delay(1000);

        for (int player = 0; player < 4; player++) { // Distribution du reste des cartes
            while (playersDecks[player].size() < 8) {
                if (player == 0) {
                    delay(500);
                    // Animation for adding card to the deck
                    u8g2.clearBuffer();
                    char msg[20];
                    sprintf(msg, "Carte pour vous: %s %s", cardsGame[0].colors.c_str(), cardsGame[0].name.c_str());
                    u8g2.drawStr(10, 20, msg);
                    u8g2.sendBuffer();
                }
                playersDecks[player].push_back(cardsGame[0]);
                cardsGame.erase(cardsGame.begin());
            }
        }
        // Animation for sorting cards
        u8g2.clearBuffer();
        u8g2.drawStr(20, 20, "Tri des cartes...");
        u8g2.sendBuffer();
        delay(2000);
        std::sort(playersDecks[0].begin(), playersDecks[0].end(), [](Card a, Card b) {
            return a.colors < b.colors || (a.colors == b.colors && a.value < b.value);
        });

        while (touchRead(button1) < 35 || touchRead(button2) < 35) {}

        std::vector<int> splitTeamsScores = {0, 0};
        for (int split = 1; split <= 8; split++) { // Chaque pli dans le round
            std::vector<int> pliTeamsScores = {0, 0};
            std::vector<Card> cardsPlaced = {};
            for (int player = firstPlayerPlay; player < firstPlayerPlay + 4; player++) { // pour chaque joueur
                int currentPlayer = player % 4;
                if (currentPlayer == 0) { // Si c'est le joueur réel qui doit jouer
                    bool playerPlayed = false;
                    while (!playerPlayed) {
                        // Affichage du deck et de la carte sur la table
                        u8g2.clearBuffer();
                        u8g2.drawStr(10, 10, "Votre deck:");
                        for (size_t i = 0; i < playersDecks[0].size(); i++) {
                            displayCard(playersDecks[0][i], 10, 20 + i * 10);
                        }
                        u8g2.drawStr(80, 10, "Sur la table:");
                        for (size_t i = 0; i < cardsPlaced.size(); i++) {
                            displayCard(cardsPlaced[i], 80, 20 + i * 10);
                        }
                        u8g2.sendBuffer();

                        if (touchRead(button1) < 35 && touchRead(button2) < 35) { // Si le joueur a choisi sa carte
                            int cardSelected = IndexFocused;
                            int timeClicked = 0;
                            while (touchRead(button1) < 35 && touchRead(button2) < 35) {
                                timeClicked++;
                            }
                            if (timeClicked > 20000) { // si long click
                                if (playersDecks[0][cardSelected].name == "Valet" || playersDecks[0][cardSelected].name == "Dame") {
                                    numberBeloteSaid++;
                                    beloteTeam = currentPlayer % 2;
                                } else {
                                    continue;
                                }
                            }
                            if (!cardIsPlacable(playersDecks[0][cardSelected], cardsPlaced, currentAtout, playersDecks[0])) continue;
                            cardsPlaced.push_back(playersDecks[0][cardSelected]);
                            playersDecks[0].erase(playersDecks[0].begin() + cardSelected);
                            playerPlayed = true;
                        }
                    }

                } else {
                    delay(1000);
                    // Faire jouer le bot
                    auto it = std::find_if(playersDecks[currentPlayer].begin(), playersDecks[currentPlayer].end(),
                                           [&](Card &c) { return cardIsPlacable(c, cardsPlaced, currentAtout, playersDecks[currentPlayer]); });
                    if (it != playersDecks[currentPlayer].end()) {
                        cardsPlaced.push_back(*it);
                        playersDecks[currentPlayer].erase(it);
                    } else {
                        // Si aucun coup valide, jouer la première carte disponible
                        cardsPlaced.push_back(playersDecks[currentPlayer][0]);
                        playersDecks[currentPlayer].erase(playersDecks[currentPlayer].begin());
                    }
                }
                // Animation pour placer la carte
                u8g2.clearBuffer();
                u8g2.drawStr(20, 20, "Carte placée:");
                displayCard(cardsPlaced.back(), 20, 30);
                u8g2.sendBuffer();
                delay(500);
            }
            // Montrer les scores du pli
            for (int i = 0; i < 4; i++) {
                pliTeamsScores[i % 2] += cardsPlaced[i].value;
            }
            splitTeamsScores[0] += pliTeamsScores[0];
            splitTeamsScores[1] += pliTeamsScores[1];

            u8g2.clearBuffer();
            u8g2.drawStr(20, 20, "Scores du pli:");
            char scoreBuffer[20];
            sprintf(scoreBuffer, "Team 1: %d, Team 2: %d", pliTeamsScores[0], pliTeamsScores[1]);
            u8g2.drawStr(20, 30, scoreBuffer);
            u8g2.sendBuffer();
            delay(1000);
        }
        totalTeamsScores[0] += splitTeamsScores[0];
        totalTeamsScores[1] += splitTeamsScores[1];

        if (currentRound >= roundsMax && roundsMax > 0) { // Si tous les rounds sont joués
            u8g2.clearBuffer();
            u8g2.drawStr(20, 20, "Fin de la partie");
            u8g2.sendBuffer();
            delay(1000);
            break;
        }
        if (roundsMax == 0) {
            // Demander si le joueur veut finir la partie
            u8g2.clearBuffer();
            u8g2.drawStr(20, 20, "Finir la partie? (<- Non, * Oui)");
            u8g2.sendBuffer();
            while (true) {
                if (touchRead(button1) < 35) {
                    break;
                } else if (touchRead(button2) < 35) {
                    currentRound = roundsMax; // Forcer la fin
                    break;
                }
            }
        }

        currentRound++;
        firstPlayerPlay++;
    }
    // Montrer les résultats et le vainqueur
    u8g2.clearBuffer();
    u8g2.drawStr(20, 20, "Résultats finaux:");
    char finalScoreBuffer[20];
    sprintf(finalScoreBuffer, "Team 1: %d, Team 2: %d", totalTeamsScores[0], totalTeamsScores[1]);
    u8g2.drawStr(20, 30, finalScoreBuffer);
    u8g2.drawStr(20, 40, (totalTeamsScores[0] > totalTeamsScores[1]) ? "Team 1 Wins!" : "Team 2 Wins!");
    u8g2.sendBuffer();
    delay(5000);
}
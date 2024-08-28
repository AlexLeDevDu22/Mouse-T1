#include <U8g2lib.h>

#include <vector>

#include <algorithm>

#include <random>

#include <Wire.h>

#include <unordered_map>

#define button1 14
#define button2 12
#define SDA_PIN 13
#define SCL_PIN 15

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

int IndexFocused = 0;

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

class Card {
    public: std::string colors;
    std::string name;
    int value;

    Card(std::string c, std::string n, int v): colors(c),
    name(n),
    value(v) {}
};

std::vector < Card > generateRandomGame() {
    std::vector < Card > jeu;
    std::vector < std::string > colors = {
        "Coeur",
        "Carreau",
        "Trefle",
        "Pique"
    };
    std::vector < std::pair < std::string, int >> values = {
        {
            "7",
            0
        },
        {
            "8",
            0
        },
        {
            "9",
            14
        },
        {
            "10",
            10
        },
        {
            "Valet",
            20
        },
        {
            "Dame",
            3
        },
        {
            "Roi",
            4
        },
        {
            "As",
            11
        }
    };

    // Générer les 32 cartes
    for (const auto & color: colors) {
        for (const auto & card: values) {
            jeu.emplace_back(color, card.first, card.second);
        }
    }

    // Mélanger les cartes
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(jeu.begin(), jeu.end(), g);

    return jeu;
}

bool cardIsPlacable(Card card, std::vector < Card > cardsPlaced, std::string atout, std::vector < Card > playerDeck) {
    if (cardsPlaced.empty()) return true; // Si c'est la première carte du pli
    std::string couleurDemandee = cardsPlaced[0].colors;

    if (card.colors == couleurDemandee) return true; // Si la carte a la même couleur que celle demandée

    // Si la carte est un atout
    if (card.colors == atout) {
        // Si le joueur n'a pas la couleur demandée
        for (const auto & c: playerDeck) {
            if (c.colors == couleurDemandee) return false;
        }
        return true;
    }

    // Si le joueur n'a ni la couleur demandée ni de l'atout
    for (const auto & c: playerDeck) {
        if (c.colors == couleurDemandee || c.colors == atout) return false;
    }

    return true;
}


std::unordered_map<std::string, int> coeurMap = {
    {"7", 0x21}, {"8", 0x22}, {"9", 0x23}, {"10", 0x24},
    {"Valet", 0x25}, {"Dame", 0x26}, {"Roi", 0x27}, {"As", 0x28}
};

std::unordered_map<std::string, int> carreauMap = {
    {"7", 0x31}, {"8", 0x32}, {"9", 0x33}, {"10", 0x34},
    {"Valet", 0x35}, {"Dame", 0x36}, {"Roi", 0x37}, {"As", 0x38}
};

std::unordered_map<std::string, int> trefleMap = {
    {"7", 0x41}, {"8", 0x42}, {"9", 0x43}, {"10", 0x44},
    {"Valet", 0x45}, {"Dame", 0x46}, {"Roi", 0x47}, {"As", 0x48}
};

std::unordered_map<std::string, int> piqueMap = {
    {"7", 0x51}, {"8", 0x52}, {"9", 0x53}, {"10", 0x54},
    {"Valet", 0x55}, {"Dame", 0x56}, {"Roi", 0x57}, {"As", 0x58}
};

int getHexCodeForCard(const Card& card) {
    if (card.colors == "Coeur") {
        return coeurMap[card.name];
    } else if (card.colors == "Carreau") {
        return carreauMap[card.name];
    } else if (card.colors == "Trefle") {
        return trefleMap[card.name];
    } else if (card.colors == "Pique") {
        return piqueMap[card.name];
    }
    return -1; // En cas d'erreur, retourner -1
}

void displayCard(Card card, int x, int y, bool hidden=false) {
    u8g2.setFont(u8g2_font_unifont_t_cards);
    if (!hidden)  u8g2.drawGlyph(x, y, getHexCodeForCard(card) );
    else u8g2.drawGlyph(x, y, 0x20 );
    u8g2.setFont(u8g2_font_ncenB08_tr);
    //char buffer[20];
    //sprintf(buffer, "%s %s", card.colors.c_str(), card.name.c_str());
    //u8g2.drawStr(x, y, buffer);
}

std::vector < std::vector < Card >> playersDecks(4);

void displayDeck(int numDeck, bool showCurrent){
    std::vector < Card > deck = playersDecks[numDeck];

    if (IndexFocused>deck.size()-1) IndexFocused=deck.size()-1;
    for (int i = 0; i < deck.size(); i++){
        if (showCurrent && IndexFocused==i){

            u8g2.setDrawColor(0);
            displayCard(deck[i], 64-deck.size()*7+i*14, 28);
            u8g2.setDrawColor(1);
        }
        else  
            displayCard(deck[i], 64-deck.size()*7+i*14, 32);
    }
}

std::vector<std::vector<int>> calculateTrajectory(std::vector<int> start, std::vector<int> end, int steps) {
    std::vector<std::vector<int>> trajectory;
    
    for (int i = 0; i <= steps; ++i) {
        float ratio = static_cast<float>(i) / steps;
        Serial.println(start[0]);
        Serial.println(start[1]);
        Serial.println(end[0]);
        Serial.println(end[1]);

        std::vector<int> current = {(int)(start[0] + (end[0] - start[0]) * ratio) , (int)( start[1] + (end[1] - start[1]) * ratio)};

        trajectory.push_back(current);
    }
    
    return trajectory;
}



void setup() {
    Wire.begin(SDA_PIN, SCL_PIN);
    Serial.begin(115200);
    u8g2.begin();
    u8g2.setFont(u8g2_font_ncenB08_tr);
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
    std::vector < int > totalTeamsScores = {
        0,
        0
    };
    std::vector<std::vector<int>> posPlayers = {{60,35},{-5,18},{60, 8},{120,18}};

    while (true) { //each rounds
        int firstPlayerPlay = millis() % 4;
        std::string currentAtout = "";
        int teamsTaked;
        int numberBeloteSaid = 0;
        int beloteTeam;
        std::vector < Card > cardsGame;
        int dealtsCards;

        while (true) { // while is not take
            cardsGame = generateRandomGame();

            dealtsCards = 0;
            for (int tour = firstPlayerPlay; tour < firstPlayerPlay+8; tour++) { // Distribution pour chaque joueur
                int currentPlayer=tour%4;

                if (tour<=3) playersDecks[currentPlayer].clear();

                for (int i = 0; i < 3; i++) { // Ajout de 3 ou 2 cartes

                    if (tour>3 && i==2) continue;// for pass 3 and 2 times
                
                    std::vector<int> endAnimation;
                    if (currentPlayer == 0) endAnimation={64-playersDecks[currentPlayer].size()*7+playersDecks[currentPlayer].size()*14, 32};
                    else endAnimation=posPlayers[currentPlayer];

                    std::vector<std::vector<int>> positions = calculateTrajectory(posPlayers[firstPlayerPlay-1%4], endAnimation, 10);

                    for (const auto& pos : positions) {
                        u8g2.clearBuffer();
                        displayDeck(0, false);

                        for (int p=0; p<4; p++){
                            if ((playersDecks[p].size()>0  || p==firstPlayerPlay-1%4) && p!=0) displayCard(cardsGame[dealtsCards], posPlayers[p][0], posPlayers[p][1], true);
                        }

                        displayCard(cardsGame[dealtsCards], pos[0], pos[1], true);
                        u8g2.sendBuffer();
                        delay(60);
                    }

                    playersDecks[currentPlayer].push_back(cardsGame[dealtsCards]);

                    dealtsCards++;
                }
            }

            std::vector<std::vector<int>> positions = calculateTrajectory(posPlayers[firstPlayerPlay-1%4], {38, 15}, 10);

            for (const auto& pos : positions){
                u8g2.clearBuffer();
                displayDeck(0, false);

                for (int p=0; p<4; p++){
                    if (p!=0) displayCard(cardsGame[dealtsCards], posPlayers[p][0], posPlayers[p][1], true);
                }

                displayCard(cardsGame[dealtsCards], pos[0], pos[1], true);
                u8g2.sendBuffer();
                delay(60);
            }


            bool isTake = false;

            int lastPlayerTaked;

            for (int atoutChoiceType = 1; atoutChoiceType <= 2; atoutChoiceType++) { // Étape 1 pour dire ONE et deuxième pour dire TWO
                for (int player = firstPlayerPlay; player < firstPlayerPlay + 4; player++) {
                    int currentPlayer = player % 4; // Ajustement pour les 4 joueurs
                    lastPlayerTaked=currentPlayer;
                    if (!isTake) {
                        if (currentPlayer == 0) { // Si c'est le joueur réel qui doit choisir
                            while ((touchRead(button1) < 35) || (touchRead(button2) < 35));
                            while (!isTake) { // while choice
                                u8g2.clearBuffer();
                                displayCard(cardsGame[dealtsCards], 38, 15);

                                u8g2.setDrawColor(1);
                                u8g2.setCursor(80, 11);
                                u8g2.print(atoutChoiceType);
                                u8g2.drawStr(90, 11, "Yes");
                                if (IndexFocused % 2 == 0) {
                                    u8g2.drawBox(80, 2, 7, 10);
                                    u8g2.setDrawColor(0);
                                    u8g2.setCursor(80, 11);
                                    u8g2.print(atoutChoiceType);
                                } else {
                                    u8g2.drawBox(90, 2, 20, 10);
                                    u8g2.setDrawColor(0);
                                    u8g2.drawStr(90, 11, "Yes");
                                }
                                u8g2.setDrawColor(1);
                                displayDeck(currentPlayer, false);
                                u8g2.sendBuffer();

                                if (touchRead(button1) < 35 && touchRead(button2) < 35) { //if click
                                    u8g2.clearBuffer();
                                    while ((touchRead(button1) < 35) || (touchRead(button2) < 35)) {};
                                    if (IndexFocused % 2 == 1) { //if he click on yes
                                        if (atoutChoiceType == 1) {
                                            isTake = true;
                                            currentAtout = cardsGame[dealtsCards].colors;
                                        } else {
                                            while (true) { // while choice the colors or cancel
                                                u8g2.clearBuffer();
                                                displayCard(cardsGame[dealtsCards], 25, 15);
                                                u8g2.setFont(u8g2_font_9x15_t_symbols);
                                                for (int i = 0; i < 5; i++) {
                                                    if (i == IndexFocused % 5) {
                                                        if (i == 0) u8g2.drawBox(50, 1, 11, 12);
                                                        else u8g2.drawBox(55+i*15, 1, 11, 12);
                                                        u8g2.setDrawColor(0);
                                                    }
                                                    if (i == 0) {
                                                        u8g2.drawGlyph(50, 10, 0x2715);
                                                    } else {
                                                        u8g2.drawGlyph(56+i*15, 12, 0x2660 + i);
                                                    }
                                                    u8g2.setDrawColor(1);
                                                }
                                                displayDeck(currentPlayer, false);
                                                u8g2.sendBuffer();
                                                delay(10);
                                                if (touchRead(button1) < 35 && touchRead(button2) < 35) break; //if confirm color or cancel

                                            }
                                            if (IndexFocused % 5 == 0) {
                                                break;
                                            } else {
                                                isTake = true;
                                                switch (IndexFocused % 5) {
                                                case 1:
                                                    currentAtout = "Pique";
                                                case 2:
                                                    currentAtout = "Coeur";
                                                case 3:
                                                    currentAtout = "Carreau";
                                                case 4:
                                                    currentAtout = "Trefle";
                                                }

                                            }
                                        }
                                    } else
                                        break;

                                }
                            }
                        } else {
                            delay(500);
                            // Choix du bot s'il prend
                            if (botDificult > 5 && std::rand() % 2 == 0) {
                                isTake = true;
                                currentAtout = cardsGame[dealtsCards].colors;
                            }
                        }
                    }
                }
            }
            if (isTake) {
                playersDecks[lastPlayerTaked].push_back(cardsGame[dealtsCards]);
                dealtsCards++;
                teamsTaked = lastPlayerTaked % 2;
                break;
            };
            u8g2.clearBuffer();
            u8g2.drawStr(20, 20, "personne n'a pris, relancement");
            u8g2.sendBuffer();
            delay(1000);
        }
        u8g2.clearBuffer();
        std::string message;
        if (teamsTaked==0) message = "Vous avez pris à " + currentAtout;
        else message = "L'adversaire à pris à " + currentAtout;
        u8g2.drawStr(20, 12, message.c_str());

        u8g2.drawStr(20, 32, "C'est parti !");
        u8g2.sendBuffer();
        delay(1000);

        for (int player = firstPlayerPlay; player < firstPlayerPlay+4; player++) { // Distribution du reste des cartes
            int currentPlayer = player%4;

            while (playersDecks[currentPlayer].size() < 8) {

                std::vector<int> endAnimation;
                    if (currentPlayer == 0) endAnimation={64-playersDecks[currentPlayer].size()*7+playersDecks[currentPlayer].size()*14, 32};
                    else endAnimation=posPlayers[currentPlayer];

                    std::vector<std::vector<int>> positions = calculateTrajectory(posPlayers[firstPlayerPlay-1%4], endAnimation, 10);

                    for (const auto& pos : positions) {
                        u8g2.clearBuffer();
                        displayDeck(0, false);

                        for (int p=0; p<4; p++){
                            if ((playersDecks[p].size()>0  || p==firstPlayerPlay-1%4) && p!=0) displayCard(cardsGame[dealtsCards], posPlayers[p][0], posPlayers[p][1], true);
                        }

                        displayCard(cardsGame[dealtsCards], pos[0], pos[1], true);
                        u8g2.sendBuffer();
                        delay(60);
                    }


                playersDecks[currentPlayer].push_back(cardsGame[dealtsCards]);

                dealtsCards++;
            }
        }

        u8g2.clearBuffer();
        u8g2.drawStr(20, 20, "Tri des cartes...");
        u8g2.sendBuffer();
        delay(1000);
        std::sort(playersDecks[0].begin(), playersDecks[0].end(), [](Card a, Card b) {
            return a.colors < b.colors || (a.colors == b.colors && a.value < b.value);
        });

        while (touchRead(button1) < 35 || touchRead(button2) < 35) {}

        std::vector < int > splitTeamsScores = {
            0,
            0
        };
        int lastWinIndex;
        for (int split = 1; split <= 8; split++) { // Chaque pli dans le round
            int pliScores = 0;
            std::vector < Card > cardsPlaced = {};
            delay(1000);

            int startPlayer;
            if (split==1) startPlayer=firstPlayerPlay;
            else startPlayer=lastWinIndex;
            for (int player = startPlayer; player < startPlayer + 4; player++) { // pour chaque joueur
                int currentPlayer = player % 4;
                if (currentPlayer == 0) { // Si c'est le joueur réel qui doit jouer
                    bool playerPlayed = false;
                    while (!playerPlayed) {
                        // Affichage du deck et de la carte sur la table
                        u8g2.clearBuffer();
                        for (size_t i = 0; i < cardsPlaced.size(); i++) {
                            displayCard(cardsPlaced[i], 34 + i * 14, 15);
                        }
                        displayDeck(currentPlayer, true);
                        u8g2.sendBuffer();

                        if (touchRead(button1) < 35 && touchRead(button2) < 35) { // Si le joueur a choisi sa carte
                            int cardSelected = IndexFocused;
                            int timeClicked = 0;
                            while (touchRead(button1) < 35 && touchRead(button2) < 35) {
                                timeClicked++;
                                delay(1);
                            }
                            if (timeClicked > 2000) { // si long click
                                if (playersDecks[0][cardSelected].name == "Roi" || playersDecks[0][cardSelected].name == "Dame") {
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
                        [ & ](Card & c) {
                            return cardIsPlacable(c, cardsPlaced, currentAtout, playersDecks[currentPlayer]);
                        });
                    if (it != playersDecks[currentPlayer].end()) {
                        cardsPlaced.push_back( * it);
                        playersDecks[currentPlayer].erase(it);
                    } else {
                        // Si aucun coup valide, jouer la première carte disponible
                        cardsPlaced.push_back(playersDecks[currentPlayer][0]);
                        playersDecks[currentPlayer].erase(playersDecks[currentPlayer].begin());
                    }
                }

                std::vector<int> posAnimatedCard;

                if (currentPlayer == 0) posAnimatedCard={64-playersDecks[currentPlayer].size()*7+IndexFocused*14, 28};
                else posAnimatedCard=posPlayers[currentPlayer];


                std::vector<std::vector<int>> positions = calculateTrajectory(posAnimatedCard, {34 + (cardsPlaced.size()-1) * 14 , 15}, 10);

                for (const auto& pos : positions) {
                    u8g2.clearBuffer();
                    for (size_t i = 0; i < cardsPlaced.size()-1; i++) {
                        displayCard(cardsPlaced[i], 34 + i * 14, 15);
                    }
                    if (currentPlayer == 0) {
                        displayDeck(0, false);
                    } else {
                        displayDeck(0, true);
                    }

                    displayCard(cardsPlaced.back(), pos[0], pos[1]);
                    u8g2.sendBuffer();

                    delay(70);
                }

            }




            std::string requestedColor = cardsPlaced[0].colors;  // Couleur demandée (première carte jouée)
            bool atoutPlayed = false;  // Flag pour savoir si un atout est joué

            // Vérifier s'il y a un atout dans le cardsPlaced
            for (int i = 0; i < cardsPlaced.size(); ++i) {
                if (cardsPlaced[i].colors == currentAtout) {
                    atoutPlayed = true;
                    break;
                }
            }

            for (int i = 1; i < cardsPlaced.size(); ++i) {
                if (atoutPlayed) {  // Si un atout a été joué
                    if (cardsPlaced[i].colors == currentAtout && cardsPlaced[i].value > cardsPlaced[lastWinIndex].value) {
                        lastWinIndex = i;
                    }
                } else {  // Si aucun atout n'a été joué, on suit la couleur demandée
                    if (cardsPlaced[i].colors == requestedColor && cardsPlaced[i].value > cardsPlaced[lastWinIndex].value) {
                        lastWinIndex = i;
                    }
                }
            }


            //animation de fin de pli
            std::vector<int> endAnim;

            if (lastWinIndex%2==0) endAnim={100,45};
            else endAnim={145, 15};

            std::vector<std::vector<int>> posCard1 = calculateTrajectory({34,15}, endAnim, 10);
            std::vector<std::vector<int>> posCard2 = calculateTrajectory({48,15}, endAnim, 10);
            std::vector<std::vector<int>> posCard3 = calculateTrajectory({62,15}, endAnim, 10);
            std::vector<std::vector<int>> posCard4 = calculateTrajectory({76,15}, endAnim, 10);

            std::vector<std::vector<std::vector<int>>> posCards = {posCard1, posCard2, posCard3, posCard4};

            for (int i=0;i<posCard1.size();i++) {
                u8g2.clearBuffer();
                for (size_t i_ = 0; i_ < cardsPlaced.size()-1; i_++) {
                    displayCard(cardsPlaced[i_], posCards[i_][i][0], posCards[i_][i][1]);
                }
                
                displayDeck(0, true);

                u8g2.sendBuffer();

                delay(100);
            }


            // Montrer et enregistrer les scores du pli

            for (int i = 0; i < 4; i++) {
                pliScores += cardsPlaced[i].value;
            }
            splitTeamsScores[lastWinIndex % 2] += pliScores;

            u8g2.clearBuffer();
            u8g2.drawStr(20, 20, "Scores du pli:");
            char scoreBuffer[20];
            if (lastWinIndex % 2==0) sprintf(scoreBuffer, "Vous avez gagné! ( %d ) points", pliScores);
            else sprintf(scoreBuffer, "Les adversaires ont gagné! ( %d ) points", pliScores);
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
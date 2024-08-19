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

void displayCard(Card card, int x, int y) {
    u8g2.setFont(u8g2_font_unifont_t_cards);
    u8g2.drawGlyph(x, y, getHexCodeForCard(card) );
    u8g2.setFont(u8g2_font_ncenB08_tr);
    //char buffer[20];
    //sprintf(buffer, "%s %s", card.colors.c_str(), card.name.c_str());
    //u8g2.drawStr(x, y, buffer);
}

std::vector < std::vector < Card >> playersDecks(4);

void displayDeck(int numDeck){
    std::vector < Card > deck = playersDecks[numDeck];

    for (int i = 0; i < deck.size(); i++){
        displayCard(deck[i], 64-deck.size()*7+i*14, 32);
    }
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
    while (true) { //each rounds
        int firstPlayerPlay = millis() % 4; // Changement de 3 à 4 pour inclure 4 joueurs
        std::string currentAtout = "";
        int teamsTaked;
        int numberBeloteSaid = 0;
        int beloteTeam;
        std::vector < Card > cardsGame;
        int dealtsCards;

        while (true) { // while is not take
            cardsGame = generateRandomGame();

            dealtsCards = 0;
            for (int player = 0; player < 4; player++) { // Distribution pour chaque joueur
                playersDecks[player].clear();
                for (int i = 0; i < 5; i++) { // Ajout de 5 cartes
                    playersDecks[player].push_back(cardsGame[dealtsCards]);
                    if (player == 0) {
                        u8g2.clearBuffer();
                        displayDeck(player);
                        u8g2.sendBuffer();
                        delay(500);
                    }
                    dealtsCards++;
                }
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
                                displayDeck(currentPlayer);
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
                                                displayDeck(currentPlayer);
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
        if (teamsTaked==1) message = "Vous avez pris à " + currentAtout;
        else message = "L'adversaire à pris à " + currentAtout;
        u8g2.drawStr(20, 12, message.c_str());

        u8g2.drawStr(20, 32, "C'est parti !");
        u8g2.sendBuffer();
        delay(1000);

        for (int player = 0; player < 4; player++) { // Distribution du reste des cartes
            while (playersDecks[player].size() < 8) {
                playersDecks[player].push_back(cardsGame[dealtsCards]);
                if (player == 0) {
                    u8g2.clearBuffer();
                    displayDeck(player);
                    u8g2.sendBuffer();
                    delay(500);
                }
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
        for (int split = 1; split <= 8; split++) { // Chaque pli dans le round
            std::vector < int > pliTeamsScores = {
                0,
                0
            };
            std::vector < Card > cardsPlaced = {};
            for (int player = firstPlayerPlay; player < firstPlayerPlay + 4; player++) { // pour chaque joueur
                int currentPlayer = player % 4;
                if (currentPlayer == 0) { // Si c'est le joueur réel qui doit jouer
                    bool playerPlayed = false;
                    while (!playerPlayed) {
                        // Affichage du deck et de la carte sur la table
                        u8g2.clearBuffer();
                        displayDeck(currentPlayer);
                        for (size_t i = 0; i < cardsPlaced.size(); i++) {
                            displayCard(cardsPlaced[i], 60+ i * 14);
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
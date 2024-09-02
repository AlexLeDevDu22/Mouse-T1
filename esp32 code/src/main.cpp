#include <unordered_map>

#include <algorithm>

#include <U8g2lib.h>

#include <cstdlib>

#include <vector>

#include <random>

#include <Wire.h>

#include <chrono>

#include <ctime>

#include <map>

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
public:
    std::string color;
    std::string name;
    int value;

    Card(std::string c, std::string n, int v) : color(c), name(n), value(v) {}

    int getValue(std::string currentAtout) const {

        if (currentAtout==color) {
            if (name=="Valet"){
                return 20;
            }
            if (name=="9"){
                return 14;
            }
        }
        return value;
        
    }
};


std::vector < Card > generateRandomGame() {
    std::vector < Card > jeu;
    std::vector < std::string > color = {
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
            0
        },
        {
            "10",
            10
        },
        {
            "Valet",
            2
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
    for (const auto & color: color) {
        for (const auto & card: values) {
            jeu.emplace_back(color, card.first, card.second);
        }
    }

    // Mélanger les cartes avec une graine basée sur l'heure actuelle
    unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::mt19937 g(seed);
    std::shuffle(jeu.begin(), jeu.end(), g);

    return jeu;
}

bool cardIsPlacable(Card card, std::vector < Card > cardsPlaced, std::string atout, std::vector < Card > playerDeck) {
    if (cardsPlaced.empty()) return true; // Si c'est la première carte du pli
    std::string couleurDemandee = cardsPlaced[0].color;

    if (card.color == couleurDemandee) return true; // Si la carte a la même color que celle demandée

    // Si la carte est un atout
    if (card.color == atout) {
        // Si le joueur n'a pas la color demandée
        for (const auto & c: playerDeck) {
            if (c.color == couleurDemandee) return false;
        }
        return true;
    }

    // Si le joueur n'a ni la color demandée ni de l'atout
    for (const auto & c: playerDeck) {
        if (c.color == couleurDemandee || c.color == atout) return false;
    }

    return true;
}

int evaluerCarte(Card carte,
        std::string couleurDemandee,
        std::string atout) {

    int score = carte.getValue(atout);

    // Bonus si la carte est de la color demandée

    if (carte.color == couleurDemandee) {
        score += 10;
    }
    // Bonus plus élevé si la carte est un atout
    if (carte.color == atout) {
        score += 20;
    }
    // Valeur spéciale pour les cartes clés
    if (carte.name == "As") {
        score += 10;
    } else if (carte.name == "10") {
        score += 8;
    } else if (carte.name == "Roi") {
        score += 6;
    } else if (carte.name == "Dame") {
        score += 4;
    } else if (carte.name == "Valet") {
        score += (carte.color == atout) ? 20 : 2; // Valet d'atout a une très haute value
    }

    return score;
}

// Fonction pour choisir la meilleure carte à jouer
int choisirCarteAJouer(std::vector < Card > main, std::vector < Card > cardsPlaced, std::string atout, int difficulte) {
    std::vector < std::pair<Card, int > > cartesJouables;

    // Filtrer les cartes jouables
    for (int i = 0; i < main.size(); i++) {
        if (cardIsPlacable(main[i], cardsPlaced, atout, main)) {
            cartesJouables.push_back({main[i],i});
        }
    }


    // Si aucune carte n'est trouvée, toutes les cartes sont jouables
    if (cartesJouables.empty()) {
        for (int i = 0; i < main.size(); i++) {
            cartesJouables.push_back({main[i],i});
        }
    }

    // Évaluer chaque carte jouable
    std::vector < std::pair < int, int >> evaluations;
    for (int i = 0; i < cartesJouables.size(); i++) {
                    
        int score;
        if (cardsPlaced.size()>0) score = evaluerCarte(cartesJouables[i].first, cardsPlaced[0].color, atout);
        else score = evaluerCarte(cartesJouables[i].first, "", atout);

        evaluations.push_back({
            score,
            cartesJouables[i].second
        });

    }

    // Trier les cartes par évaluation décroissante
    std::sort(evaluations.begin(), evaluations.end(), [](const std::pair < int, int > & a,
        const std::pair < int, int > & b) {
        return a.first > b.first;
    });

    // Sélectionner la carte en fonction de la difficulté
    int indexMax = evaluations.size() - 1;
    int indexChoisi = indexMax - (indexMax * (difficulte - 1) / 10);

    // Prendre en compte une part d'aléatoire pour la difficulté
    if (indexChoisi > 0 && difficulte < 10) {
        srand(time(0));
        indexChoisi = rand() % (indexChoisi + 1);
    }


    return evaluations[indexChoisi].second;
}

int evaluerCarteAppel(Card carteAppel,
    std::vector < Card > main,
        std::string & atout) {
    int score = carteAppel.getValue(carteAppel.name);

    // Bonus si la carte d'appel est un atout
    if (carteAppel.color == atout) {
        score += 20;
    }

    // Valeur spéciale pour les cartes clés de l'atout
    if (carteAppel.name == "Valet" && carteAppel.color == atout) {
        score += 30; // Le Valet d'atout est très fort
    } else if (carteAppel.name == "9" && carteAppel.color == atout) {
        score += 20; // Le 9 d'atout est également très fort
    }

    int belotePossible = 0;
    for (const auto & carte: main) {
        if (carte.color == atout) {
            score += 2 + carte.getValue(carteAppel.name); // Bonus pour avoir d'autres atouts
        }

        if (carte.name == "Dame" || carte.name == "Roi") belotePossible++;
    }

    if (belotePossible == 2) score += 20;

    return score;
}

// Fonction pour évaluer quelle color serait le meilleur atout au second tour
std::string choisirMeilleurAtout(std::vector < Card > main,
    const std::string & couleurExclue) {
    std::map < std::string, int > scoresCouleurs;

    // Évaluer chaque color (sauf celle de la carte d'appel)
    for (const auto & carte: main) {
        if (carte.color != couleurExclue) {
            scoresCouleurs[carte.color] += carte.getValue(carte.name);
        }
    }

    // Trouver la color avec le meilleur score
    std::string meilleurAtout;
    int meilleurScore = 0;
    for (const auto & [color, score]: scoresCouleurs) {
        if (score > meilleurScore) {
            meilleurScore = score;
            meilleurAtout = color;
        }
    }

    return meilleurAtout;
}

// Fonction pour décider si le bot prend la carte d'appel au premier ou second tour
std::string botPrendCarte(Card carteAppel, std::vector < Card > & main, int difficulte, bool premierTour) {
    if (premierTour) {
        // Évaluer la carte d'appel au premier tour
        int score = evaluerCarteAppel(carteAppel, main, carteAppel.color);

        // Déterminer un seuil en fonction de la difficulté
        int seuil = 40 + (difficulte * 6); // Plus la difficulté est élevée, plus le seuil est bas (plus agressif)

        // Ajouter un facteur aléatoire basé sur la difficulté
        srand(time(0));
        int chance = rand() % 100;

        // Décision : prendre la carte si le score est suffisant ou si le hasard favorise la prise
        return ((score > seuil) || (chance < 10-difficulte * 10)) ? carteAppel.color : "no"; // Plus la difficulté est haute, plus on favorise la prise
    } else {
        // Second tour : choisir un autre atout
        std::string meilleurAtout = choisirMeilleurAtout(main, carteAppel.color);
        int score = evaluerCarteAppel(carteAppel, main, meilleurAtout);

        // Déterminer un seuil en fonction de la difficulté
        int seuil = 50 + (difficulte * 5); // Plus la difficulté est élevée, plus le seuil est bas (plus agressif)

        // Ajouter un facteur aléatoire basé sur la difficulté
        srand(time(0));
        int chance = rand() % 100;

        // Décision : prendre avec la nouvelle color d'atout
        bool prend = (score > seuil) || (chance < difficulte * 10);

        return prend ? meilleurAtout : "no";
    }
}

std::unordered_map < std::string, int > Map = {
    {
        "7",
        7
    },
    {
        "8",
        8
    },
    {
        "9",
        9
    },
    {
        "10",
        10
    },
    {
        "Valet",
        11
    },
    {
        "Dame",
        13
    },
    {
        "Roi",
        14
    },
    {
        "As",
        01
    }
};

int getHexCodeForCard(const Card & card) {
    if (card.color == "Pique") {
        return Map[card.name] + 32;
    } else if (card.color == "Coeur") {
        return Map[card.name] + 48;
    } else if (card.color == "Carreau") {
        return Map[card.name] + 64;
    } else if (card.color == "Trefle") {
        return Map[card.name] + 80;
    }
    return -1;
}

void displayCard(Card card, int x, int y, bool hidden = false) {
    u8g2.setFont(u8g2_font_unifont_t_cards);
    if (!hidden) u8g2.drawGlyph(x, y, getHexCodeForCard(card));
    else u8g2.drawGlyph(x, y, 0x20);
    u8g2.setFont(u8g2_font_ncenB08_tr);
    //char buffer[20];
    //sprintf(buffer, "%s %s", card.color.c_str(), card.name.c_str());
    //u8g2.drawStr(x, y, buffer);
}

std::vector < std::vector < Card >> playersDecks(4);

void displayDeck(int numDeck, bool showCurrent) {
    std::vector < Card > deck = playersDecks[numDeck];

    if (IndexFocused > deck.size() - 1) IndexFocused = deck.size() - 1;

    u8g2.setDrawColor(0);
    u8g2.drawBox(8, 14, 112, 17);
    u8g2.setDrawColor(1);
    for (int i = 0; i < deck.size(); i++) {
        if (showCurrent && IndexFocused == i) {

            u8g2.setDrawColor(0);
            displayCard(deck[i], 64 - deck.size() * 7 + i * 14, 28);
            u8g2.setDrawColor(1);
        } else
            displayCard(deck[i], 64 - deck.size() * 7 + i * 14, 32);
    }
}

std::vector < std::vector < int >> calculateTrajectory(std::vector < int > start, std::vector < int > end, float slowing = 1) {
    std::vector < std::vector < int >> trajectory;

    float distance = std::sqrt(std::pow(end[0] - start[0], 2) + std::pow(end[1] - start[1], 2));

    int steps = static_cast < int > (distance * slowing);

    for (int i = 0; i <= steps; ++i) {
        float ratio = static_cast < float > (i) / steps;

        std::vector < int > current = {
            static_cast < int > (start[0] + (end[0] - start[0]) * ratio),
            static_cast < int > (start[1] + (end[1] - start[1]) * ratio)
        };

        trajectory.push_back(current);
    }

    return trajectory;
}

void showSplitsScores(std::vector < std::vector < Card >> splitTeamCards, std::vector < int > splitTeamsScores, std::string currentAtout, int currentAtoutIndex, int lastPlayerTaked,
    const char * event = NULL) {

    u8g2.drawStr(0, 12, "Vous");
    u8g2.setFont(u8g2_font_9x15_t_symbols);
    if (lastPlayerTaked % 2 == 0) u8g2.drawGlyph(50, 12, 0x2660 + currentAtoutIndex);
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.setCursor(30, 12);
    u8g2.print(splitTeamsScores[0]);

    if (splitTeamCards[0].size() > 0) {
        u8g2.drawStr(105, 12,"+");
        u8g2.setCursor(113, 12);
        u8g2.print(splitTeamCards[0][0].getValue(currentAtout));
    }

    u8g2.drawLine(0, 16, 128, 16);

    u8g2.drawStr(0, 28, "Eux");
    u8g2.setFont(u8g2_font_9x15_t_symbols);
    if (lastPlayerTaked % 2 == 1) u8g2.drawGlyph(50, 28, 0x2660 + currentAtoutIndex);
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.setCursor(30, 28);
    u8g2.print(splitTeamsScores[1]);
    if (splitTeamCards[1].size() > 0) {
        u8g2.drawStr(105, 28,"+");
        u8g2.setCursor(113, 28);
        u8g2.print(splitTeamCards[1][0].getValue(currentAtout));
    }

    if (event != NULL) {
        u8g2.drawBox(85, 11, u8g2.getStrWidth(event) + 4, 10);
        u8g2.setDrawColor(0);
        u8g2.drawStr(87, 20, event);
        u8g2.setDrawColor(1);
    }

    u8g2.sendBuffer();
}

void setup() {
    Wire.begin(SDA_PIN, SCL_PIN);
    Serial.begin(115200);
    u8g2.begin();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    xTaskCreate(updateIndexFocused, "updateIndexFocused", 2048, NULL, 1, NULL);
}

void loop() {

    IndexFocused=5;
    while (touchRead(button1) > 35 || touchRead(button2) > 35) {
        u8g2.clearBuffer();
        u8g2.drawStr(10, 10, "Difficulté des bots:");

        if (IndexFocused <= 0) IndexFocused=1;
        if (IndexFocused >= 10) IndexFocused=9;

        u8g2.setCursor(20,24);
        u8g2.print(IndexFocused);

        u8g2.drawBox(42,16, 3, 8);
        u8g2.drawBox(45,19, 37, 2);
        u8g2.drawBox(42+IndexFocused*4,15, 3, 10);
        u8g2.drawBox(82,16, 4, 8);

        u8g2.sendBuffer();
        delay(10);
    }
    int botDificult = IndexFocused % 10;

    while (touchRead(button1) < 35 || touchRead(button2) < 35) {delay(5);}

    IndexFocused=0;
    while (touchRead(button1) > 35 || touchRead(button2) > 35) {
        u8g2.clearBuffer();
        u8g2.drawStr(10, 10, "Nombre de tours: ");
        char roundText[12];
        if (IndexFocused % 12 != 0) {
            sprintf(roundText, "%d", IndexFocused % 12);
            u8g2.drawStr(20, 25, roundText);
        } else {
            u8g2.drawStr(20, 25, "a la demande");
        }
        u8g2.sendBuffer();
    }
    int roundsMax = IndexFocused % 12;

    int currentRound = 1;
    std::vector < std::vector < int >> totalTeamsScores = {
        {},
        {}
    };
    std::vector < std::vector < int >> posPlayers = {
        {
            60,
            35
        },
        {
            -5,
            18
        },
        {
            60,
            8
        },
        {
            120,
            18
        }
    };

    while (true) { //each rounds
        int firstPlayerPlay = millis() % 4;
        std::string currentAtout = "";
        int lastPlayerTaked;
        int numberBeloteSaid = 0;
        int beloteTeam = 0;
        std::vector < Card > cardsGame;
        int dealtsCards;

        while (true) { // while is not take
            cardsGame = generateRandomGame();

            dealtsCards = 0;
            for (int i = 0; i < 4; i++) playersDecks[i].clear();
            for (int tour = firstPlayerPlay; tour < firstPlayerPlay + 8; tour++) { // Distribution pour chaque joueur
                int currentPlayer = tour % 4;

                for (int i = 0; i < 3; i++) { // Ajout de 3 ou 2 cartes

                    if (tour > firstPlayerPlay + 3 && i == 2) continue; // for pass 3 and 2 times

                    std::vector < int > endAnimation;
                    if (currentPlayer == 0) endAnimation = {
                        64 - playersDecks[currentPlayer].size() * 7 + playersDecks[currentPlayer].size() * 14,
                        32
                    };
                    else endAnimation = posPlayers[currentPlayer];

                    std::vector < std::vector < int >> positions = calculateTrajectory(posPlayers[firstPlayerPlay - 1 % 4], endAnimation, 0.5);

                    for (const auto & pos: positions) {
                        u8g2.clearBuffer();
                        displayDeck(0, false);

                        for (int p = 0; p < 4; p++) {
                            if ((playersDecks[p].size() > 0 || p == firstPlayerPlay - 1 % 4) && p != 0) displayCard(cardsGame[dealtsCards], posPlayers[p][0], posPlayers[p][1], true);
                        }

                        displayCard(cardsGame[dealtsCards], pos[0], pos[1], true);
                        u8g2.sendBuffer();
                    }

                    playersDecks[currentPlayer].push_back(cardsGame[dealtsCards]);

                    dealtsCards++;
                }
            }

            std::vector < std::vector < int >> positions = calculateTrajectory(posPlayers[firstPlayerPlay - 1 % 4], {
                38,
                15
            }, 0.5);

            for (const auto & pos: positions) {
                u8g2.clearBuffer();
                displayDeck(0, false);

                for (int p = 0; p < 4; p++) {
                    if (p != 0) displayCard(cardsGame[dealtsCards], posPlayers[p][0], posPlayers[p][1], true);
                }

                displayCard(cardsGame[dealtsCards], pos[0], pos[1], true);
                u8g2.sendBuffer();
            }

            bool isTake = false;

            for (int atoutChoiceType = 1; atoutChoiceType <= 2; atoutChoiceType++) { // Étape 1 pour dire ONE et deuxième pour dire TWO
                for (int player = firstPlayerPlay; player < firstPlayerPlay + 4; player++) {
                    int currentPlayer = player % 4; // Ajustement pour les 4 joueurs
                    if (!isTake) {
                        lastPlayerTaked = currentPlayer;

                        u8g2.clearBuffer();

                        u8g2.setFont(u8g2_font_5x7_tf);
                        u8g2.setCursor(3, 9);
                        u8g2.print(currentRound);

                        switch (currentPlayer) {
                        case 0:
                            u8g2.drawLine(0, 31, 128, 31);
                            break;
                        case 1:
                            u8g2.drawLine(1, 0, 1, 32);
                            break;
                        case 2:
                            u8g2.drawLine(0, 1, 128, 1);
                            break;
                        case 3:
                            u8g2.drawLine(127, 0, 127, 32);
                            break;
                        }

                        displayDeck(0, false);

                        displayCard(cardsGame[dealtsCards], 38, 15);

                        u8g2.sendBuffer();

                        if (currentPlayer == 0) { // Si c'est le joueur réel qui doit choisir
                            while ((touchRead(button1) < 35) || (touchRead(button2) < 35));
                            while (!isTake) { // while choice

                                u8g2.setDrawColor(0);
                                u8g2.drawBox(80, 2, 30, 10);
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

                                u8g2.sendBuffer();

                                if (touchRead(button1) < 35 && touchRead(button2) < 35) { //if click
                                    u8g2.clearBuffer();
                                    while ((touchRead(button1) < 35) || (touchRead(button2) < 35)) {delay(5);};
                                    if (IndexFocused % 2 == 1) { //if he click on yes
                                        if (atoutChoiceType == 1) {
                                            isTake = true;
                                            currentAtout = cardsGame[dealtsCards].color;
                                        } else {
                                            while (true) { // while choice the color or cancel
                                                u8g2.clearBuffer();
                                                displayDeck(0, false);
                                                displayCard(cardsGame[dealtsCards], 25, 15);
                                                u8g2.setFont(u8g2_font_9x15_t_symbols);
                                                for (int i = 0; i < 5; i++) {
                                                    if (i == IndexFocused % 5) {
                                                        if (i == 0) u8g2.drawBox(50, 1, 11, 12);
                                                        else u8g2.drawBox(55 + i * 15, 1, 11, 12);
                                                        u8g2.setDrawColor(0);
                                                    }
                                                    if (i == 0) {
                                                        u8g2.drawGlyph(50, 10, 0x2715);
                                                    } else {
                                                        u8g2.drawGlyph(56 + i * 15, 12, 0x2660 + i);
                                                    }
                                                    u8g2.setDrawColor(1);
                                                }
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
                            delay((10 - botDificult) * 200 + millis()/100);
                            // Choix du bot s'il prend
                            std::string botColorTaked = botPrendCarte(cardsGame[dealtsCards], playersDecks[currentPlayer], botDificult, true);
                            if (botColorTaked != "no") {
                                isTake = true;
                                currentAtout = botColorTaked;
                            }
                        }
                    }
                }
            }
            if (isTake) {
                playersDecks[lastPlayerTaked].push_back(cardsGame[dealtsCards]);
                dealtsCards++;
                break;
            };
            u8g2.clearBuffer();
            u8g2.drawStr(20, 20, "personne n'a pris, relancement");
            u8g2.sendBuffer();
            delay(1000);
        }

        std::vector < int > endAnimation;
        if (lastPlayerTaked == 0) endAnimation = {
            64 - playersDecks[lastPlayerTaked].size() * 7 + playersDecks[lastPlayerTaked].size() * 14,
            32
        };
        else endAnimation = posPlayers[lastPlayerTaked];
        std::vector < std::vector < int >> positions = calculateTrajectory({
            38,
            15
        }, endAnimation, 0.4);

        for (const auto & pos: positions) {
            u8g2.clearBuffer();
            displayDeck(0, false);
            displayCard(cardsGame[dealtsCards - 1], pos[0], pos[1], true);
            u8g2.sendBuffer();
        }

        for (int player = firstPlayerPlay; player < firstPlayerPlay + 4; player++) { // Distribution du reste des cartes
            int currentPlayer = player % 4;

            while (playersDecks[currentPlayer].size() < 8) {

                std::vector < int > endAnimation;
                if (currentPlayer == 0) endAnimation = {
                    64 - playersDecks[currentPlayer].size() * 7 + playersDecks[currentPlayer].size() * 14,
                    32
                };
                else endAnimation = posPlayers[currentPlayer];

                std::vector < std::vector < int >> positions = calculateTrajectory(posPlayers[firstPlayerPlay - 1 % 4], endAnimation, 0.5);

                for (const auto & pos: positions) {
                    u8g2.clearBuffer();
                    displayDeck(0, false);

                    for (int p = 0; p < 4; p++) {
                        if ((playersDecks[p].size() > 0 || p == firstPlayerPlay - 1 % 4) && p != 0) displayCard(cardsGame[dealtsCards], posPlayers[p][0], posPlayers[p][1], true);
                    }

                    displayCard(cardsGame[dealtsCards], pos[0], pos[1], true);
                    u8g2.sendBuffer();
                }

                playersDecks[currentPlayer].push_back(cardsGame[dealtsCards]);

                dealtsCards++;
            }
        }

        int currentAtoutIndex;

        if (currentAtout == "Pique") {
            currentAtoutIndex = 0;
        } else if (currentAtout == "Coeur") {
            currentAtoutIndex = 1;
        } else if (currentAtout == "Carreau") {
            currentAtoutIndex = 2;
        } else if (currentAtout == "Trefle") {
            currentAtoutIndex = 3;
        }

        u8g2.clearBuffer();
        u8g2.drawStr(20, 20, "Tri des cartes...");
        u8g2.sendBuffer();
        delay(200);
        std::sort(playersDecks[0].begin(), playersDecks[0].end(),  [currentAtout](Card a, Card b) {
            return a.color < b.color || (a.color == b.color && a.getValue(currentAtout) < b.getValue(currentAtout));
        });

        while (touchRead(button1) < 35 || touchRead(button2) < 35) {delay(5);}

        std::vector < std::vector < Card > > splitTeamCards = {
            {},
            {}
        };
        int lastWinIndex = 0;
        int reelLastWinIndex;
        for (int split = 1; split <= 8; split++) { // Chaque pli dans le round
            int pliScores = 0;
            std::vector < Card > cardsPlaced = {};
            delay(1000);

            int startPlayer;
            if (split == 1) startPlayer = firstPlayerPlay;
            else startPlayer = lastWinIndex;
            for (int player = startPlayer; player < startPlayer + 4; player++) { // pour chaque joueur
                int currentPlayer = player % 4;

                u8g2.clearBuffer();

                switch (currentPlayer) {
                case 0:
                    u8g2.drawLine(0, 31, 128, 31);
                    break;
                case 1:
                    u8g2.drawLine(1, 0, 1, 32);
                    break;
                case 2:
                    u8g2.drawLine(0, 1, 128, 1);
                    break;
                case 3:
                    u8g2.drawLine(127, 0, 127, 32);
                    break;
                }

                u8g2.setFont(u8g2_font_5x7_tf);
                u8g2.setCursor(3, 9);
                u8g2.print(currentRound);

                u8g2.setFont(u8g2_font_9x15_t_symbols);
                u8g2.drawGlyph(118, 12, 0x2660 + currentAtoutIndex);

                displayDeck(0, false);

                for (size_t i = 0; i < cardsPlaced.size(); i++) {
                    displayCard(cardsPlaced[i], 34 + i * 14, 15);
                }

                u8g2.sendBuffer();

                if (currentPlayer == 0) { // Si c'est le joueur réel qui doit jouer
                    bool isPlayed = false;

                    while (!isPlayed) {

                        displayDeck(0, true);
                        for (size_t i = 0; i < cardsPlaced.size(); i++) {
                            displayCard(cardsPlaced[i], 34 + i * 14, 15);
                        }
                        u8g2.sendBuffer();

                        if (touchRead(button1) < 35 && touchRead(button2) < 35) { // Si le joueur a choisi sa carte
                            int cardSelected = IndexFocused;
                            int timeClicked = 0;
                            while (touchRead(button1) < 35 && touchRead(button2) < 35) {
                                timeClicked++;
                                delay(1);
                            }

                            if (cardIsPlacable(playersDecks[0][cardSelected], cardsPlaced, currentAtout, playersDecks[0])) {
                                if (timeClicked > 2000) { // si long click
                                    if (playersDecks[0][cardSelected].color == currentAtout && (playersDecks[0][cardSelected].name == "Roi" || playersDecks[0][cardSelected].name == "Dame")) {

                                        Serial.println(numberBeloteSaid == 0 || beloteTeam == currentPlayer % 2);
                                        if (numberBeloteSaid == 0 || beloteTeam == currentPlayer % 2) {
                                            numberBeloteSaid++;
                                            beloteTeam = currentPlayer % 2;
                                            if (numberBeloteSaid == 1) u8g2.drawStr(20,20, "Belote!");
                                            else u8g2.drawStr(20,20, "Rebelote!");
                                            u8g2.sendBuffer();
                                            delay(1000);
                                        }
                                    } else {
                                        continue;
                                    }
                                }
                                
                                cardsPlaced.push_back(playersDecks[0][cardSelected]);
                                playersDecks[0].erase(playersDecks[0].begin() + cardSelected);
                                isPlayed = true;
                            }
                        }
                    }

                } else {
                    int iCardPlayed = choisirCarteAJouer(playersDecks[currentPlayer], cardsPlaced, currentAtout, botDificult);

                    cardsPlaced.push_back(playersDecks[currentPlayer][iCardPlayed]);

                    playersDecks[currentPlayer].erase(playersDecks[currentPlayer].begin() + iCardPlayed);


                    if (playersDecks[currentPlayer][iCardPlayed].color == currentAtout && (playersDecks[currentPlayer][iCardPlayed].name == "Roi" || playersDecks[currentPlayer][iCardPlayed].name == "Dame") && (numberBeloteSaid == 0 || beloteTeam == currentPlayer % 2) && std::rand() % 10 < botDificult) {
                        numberBeloteSaid++;
                        beloteTeam = currentPlayer % 2;
                    }

                    delay((10 - botDificult) * 200 + millis()/100);

                }

                std::vector < int > posAnimatedCard;

                if (currentPlayer == 0) posAnimatedCard = {
                    64 - playersDecks[0].size() * 7 + IndexFocused * 14,
                    28
                };
                else posAnimatedCard = posPlayers[currentPlayer];

                std::vector < std::vector < int >> positions = calculateTrajectory(posAnimatedCard, {
                    34 + (cardsPlaced.size() - 1) * 14,
                    15
                });

                for (const auto & pos: positions) {
                    u8g2.clearBuffer();

                    u8g2.setFont(u8g2_font_5x7_tf);
                    u8g2.setCursor(3, 9);
                    u8g2.print(currentRound);
                    
                    u8g2.drawGlyph(118, 12, 0x2660 + currentAtoutIndex);

                    displayDeck(0, false);
                    

                    for (size_t i = 0; i < cardsPlaced.size() - 1; i++) {
                        displayCard(cardsPlaced[i], 34 + i * 14, 15);
                    }

                    displayCard(cardsPlaced.back(), pos[0], pos[1]);
                    u8g2.sendBuffer();
                }

            }

            std::string requestedColor = cardsPlaced[0].color; // Couleur demandée (première carte jouée)
            bool atoutPlayed = false; // Flag pour savoir si un atout est joué

            // Vérifier s'il y a un atout dans le cardsPlaced
            for (int i = 0; i < cardsPlaced.size(); ++i) {
                if (cardsPlaced[i].color == currentAtout) {
                    atoutPlayed = true;
                    break;
                }
            }
            for (int i = 0; i < cardsPlaced.size(); ++i) {
                if (atoutPlayed) { // Si un atout a été joué
                    if (cardsPlaced[i].color == currentAtout && cardsPlaced[i].getValue(currentAtout) > cardsPlaced[lastWinIndex].getValue(currentAtout)) {
                        lastWinIndex = i;
                    }
                } else { // Si aucun atout n'a été joué, on suit la color demandée
                    if (cardsPlaced[i].color == requestedColor && cardsPlaced[i].getValue(currentAtout) > cardsPlaced[lastWinIndex].getValue(currentAtout)) {
                        lastWinIndex = i;
                    }
                }
            }

            reelLastWinIndex = (lastWinIndex - startPlayer) % 4;

            //animation de fin de pli
            std::vector < int > endAnim;

            if (reelLastWinIndex % 2 == 0) endAnim = {
                100,
                45
            };
            else endAnim = {
                145,
                15
            };

            std::vector < std::vector < int >> posCard1 = calculateTrajectory({
                34,
                15
            }, endAnim);
            std::vector < std::vector < int >> posCard2 = calculateTrajectory({
                48,
                15
            }, endAnim);
            std::vector < std::vector < int >> posCard3 = calculateTrajectory({
                62,
                15
            }, endAnim);
            std::vector < std::vector < int >> posCard4 = calculateTrajectory({
                76,
                15
            }, endAnim);

            // Grouper les trajectoires
            std::vector < std::vector < std::vector < int >>> posCards = {
                posCard1,
                posCard2,
                posCard3,
                posCard4
            };

            // Trouver la longueur maximale des trajectoires
            size_t maxSteps = 0;
            for (const auto & card: posCards) {
                maxSteps = std::max(maxSteps, card.size());
            }

            for (size_t i = 0; i < maxSteps; ++i) {
                u8g2.clearBuffer();

                displayDeck(0, true);

                for (size_t cardIndex = 0; cardIndex < posCards.size(); ++cardIndex) {
                    size_t positionIndex = std::min(i, posCards[cardIndex].size() - 1);
                    displayCard(cardsPlaced[cardIndex], posCards[cardIndex][positionIndex][0], posCards[cardIndex][positionIndex][1]);
                }

                u8g2.sendBuffer();

            }

            // Montrer et enregistrer les scores du pli

            for (int i = 0; i < 4; i++) {
                splitTeamCards[reelLastWinIndex % 2].push_back(cardsPlaced[i]);
            }

            u8g2.clearBuffer();

            u8g2.drawStr(20, 30, "fin du pli");
            u8g2.sendBuffer();
            delay(1000);
        }

        std::vector < int > splitTeamsScores = {
            0,
            0
        };

        splitTeamsScores[reelLastWinIndex % 2] = 10; // dix de der

        std::vector < Card > lastCardPlaced = {Card("no","no",0), Card("no","no",0)};
        while (splitTeamCards[0].size() > 0 || splitTeamCards[1].size() > 0) {

            for (int i = 0; i < 30; i++) { //anim mooving cards
                u8g2.clearBuffer();


                if (splitTeamCards[0].size() > 0) {
                    
                    if (splitTeamCards[0].size() >= 2) displayCard(splitTeamCards[0][1], 60, 12); //next card

                    if (lastCardPlaced[0].color!="no") displayCard(lastCardPlaced[0], 89, 12); //past card

                    displayCard(splitTeamCards[0][0], 60 + i, 12);

                }

                if (splitTeamCards[1].size() > 0) {
                    if (splitTeamCards[1].size() >= 2) displayCard(splitTeamCards[1][1], 60, 30); //next card
                    if (lastCardPlaced[1].color!="no") displayCard(lastCardPlaced[1], 89, 30); //past card

                    displayCard(splitTeamCards[1][0], 60 + i, 30);
                }

                showSplitsScores(splitTeamCards, splitTeamsScores, currentAtout, currentAtoutIndex, lastPlayerTaked);

                delay(20);

            }

            if (splitTeamCards[0].size() > 0) {
                splitTeamsScores[0] += splitTeamCards[0][0].getValue(currentAtout);
                lastCardPlaced[0] = splitTeamCards[0][0];
                splitTeamCards[0].erase(splitTeamCards[0].begin());
            }

            if (splitTeamCards[1].size() > 0) {
                splitTeamsScores[1] += splitTeamCards[1][0].getValue(currentAtout);
                lastCardPlaced[1] = splitTeamCards[1][0];
                splitTeamCards[1].erase(splitTeamCards[1].begin());
            }

        }

        if (numberBeloteSaid == 2) {
            for (int i = 1; i >= 20; ++i) {
                splitTeamsScores[beloteTeam] += 1;
                u8g2.clearBuffer();
                showSplitsScores(splitTeamCards, splitTeamsScores, currentAtout, currentAtoutIndex, lastPlayerTaked, "+ belote");
                delay(25 + i * 2);
            }
        }

        bool litige = false;

        if (lastPlayerTaked % 2 == 0 && splitTeamsScores[0] <= splitTeamsScores[1] && splitTeamsScores[1] != 0) { //litige
            while (splitTeamsScores[1] < 162 || splitTeamsScores[0] > 0) {
                if (splitTeamsScores[1] < 162) splitTeamsScores[1]++;
                if (splitTeamsScores[0] > 0) splitTeamsScores[0]--;
                u8g2.clearBuffer();
                showSplitsScores(splitTeamCards, splitTeamsScores, currentAtout, currentAtoutIndex, lastPlayerTaked, "litige");
                delay(100 - splitTeamsScores[0]);
            }
            litige=true;

        } else if (lastPlayerTaked % 2 == 1 && splitTeamsScores[1] <= splitTeamsScores[0] && splitTeamsScores[0] != 0) { //litige
            while (splitTeamsScores[0] < 162 || splitTeamsScores[1] > 0) {
                if (splitTeamsScores[0] < 162) splitTeamsScores[0]++;
                if (splitTeamsScores[1] > 0) splitTeamsScores[1]--;
                u8g2.clearBuffer();
                showSplitsScores(splitTeamCards, splitTeamsScores, currentAtout, currentAtoutIndex, lastPlayerTaked, "litige");
                delay(100 - splitTeamsScores[1]);
            }
            litige=true;


        }


        if (splitTeamsScores[0] == 0 && !litige) { //capot
            while (splitTeamsScores[1] < 252) {
                splitTeamsScores[1]++;
                u8g2.clearBuffer();
                showSplitsScores(splitTeamCards, splitTeamsScores, currentAtout, currentAtoutIndex, lastPlayerTaked, "capot");
                delay(100 - (int)(splitTeamsScores[1] / 4));
            }
        }
        if (splitTeamsScores[1] == 0 && !litige) { //capot
            while (splitTeamsScores[0] < 252) {
                splitTeamsScores[0]++;
                u8g2.clearBuffer();
                showSplitsScores(splitTeamCards, splitTeamsScores, currentAtout, currentAtoutIndex, lastPlayerTaked, "capot");
                delay(100 - (int)(splitTeamsScores[0] / 4));
            }
        }

        u8g2.clearBuffer();
        for (int i = 0; i < 5; ++i){
            showSplitsScores(splitTeamCards, splitTeamsScores, currentAtout, currentAtoutIndex, lastPlayerTaked);
            delay(300);
        }

        totalTeamsScores[0].push_back(splitTeamsScores[0]);
        totalTeamsScores[1].push_back(splitTeamsScores[1]);

        bool isFinished = false;
        if (roundsMax == 0) {
            // Demander si le joueur veut finir la partie
            u8g2.clearBuffer();
            u8g2.drawStr(0, 20, "Finir la partie? (<- Non, -> Oui)");
            u8g2.sendBuffer();
            while (true) {
                if (touchRead(button1) < 35 && touchRead(button2) > 35) {
                    break;
                } else if (touchRead(button2) < 35) {
                    isFinished = true;
                    break;
                }
            }
        }
        if ((currentRound >= roundsMax && roundsMax > 0) || isFinished) { // Si tous les rounds sont joués
            u8g2.clearBuffer();
            u8g2.drawStr(20, 20, "Fin de la partie");
            u8g2.sendBuffer();
            delay(1000);
            break;
        }

        currentRound++;
        firstPlayerPlay++;
    }
    // Montrer les résultats et le vainqueur

    u8g2.clearBuffer();
    u8g2.drawStr(20, 10, "Résultats finaux:");
    u8g2.drawStr(20, 20, "Team 1:");
    u8g2.drawStr(20, 30, "0");
    u8g2.drawStr(50, 20, "Team 2:");
    u8g2.drawStr(50, 30, "0");

    u8g2.sendBuffer();

    std::vector < int > intTotalTeamsScores = {
        0,
        0
    };
    for (int i = 0; i < totalTeamsScores[0].size(); ++i) {
        intTotalTeamsScores[0] += totalTeamsScores[0][i];
        intTotalTeamsScores[1] += totalTeamsScores[1][i];

        u8g2.clearBuffer();
        u8g2.drawStr(20, 10, "Résultats finaux:");
        u8g2.drawStr(20, 20, "Team 1:");
        u8g2.setCursor(20, 30);
        u8g2.print(intTotalTeamsScores[0]);
        u8g2.drawStr(50, 20, "Team 2:");
        u8g2.setCursor(50, 30);
        u8g2.print(intTotalTeamsScores[0]);


        u8g2.sendBuffer();
        delay(300 + i * 5);
    }
    delay(400);
    u8g2.clearBuffer();
    u8g2.drawStr(20, 22, (intTotalTeamsScores[0] > intTotalTeamsScores[1]) ? "GG!" : "La loose!");
    u8g2.sendBuffer();
    while (touchRead(button1) < 35 || touchRead(button2) < 35) {delay(5);}
    while (touchRead(button1) > 35 && touchRead(button2) > 35) {delay(5);}

}
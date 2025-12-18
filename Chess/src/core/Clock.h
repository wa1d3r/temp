#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <string>
#include <iomanip> 
#include <sstream>

class Clock {
    float timeWhite; 
    float timeBlack; 
    float increment; 

    bool isWhiteTurn;
    bool isPaused; 

    sf::Clock deltaClock; 

public:
    
    Clock(float startTimeSeconds, float inc, bool isWhite);

    void start();
    void update();
    void switchTurn();
    bool isTimeUp() const;
    float getWhiteTime() const;
    float getBlackTime() const;
    void stop();
};
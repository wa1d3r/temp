#include "Clock.h"

Clock::Clock(float startTimeSeconds, float inc, bool isWhite)
    : timeWhite(startTimeSeconds * 1000)
    , timeBlack(startTimeSeconds * 1000)
    , increment(inc * 1000)
    , isWhiteTurn(isWhite)
    , isPaused(true)
{
}

void Clock::start()
{
    isPaused = false;
}

void Clock::update()
{
    sf::Time dt = deltaClock.restart();

    if (isPaused)
        return;

    if (isWhiteTurn)
    {
        timeWhite -= dt.asMilliseconds();
        if (timeWhite < 0)
            timeWhite = 0;
    }
    else
    {
        timeBlack -= dt.asMilliseconds();
        if (timeBlack < 0)
            timeBlack = 0;
    }
}

void Clock::switchTurn()
{
    if (isPaused)
        return;

    if (isWhiteTurn)
        timeWhite += increment;
    else
        timeBlack += increment;
    isWhiteTurn = !isWhiteTurn;
}

bool Clock::isTimeUp() const
{
    return timeWhite <= 0 || timeBlack <= 0;
}

float Clock::getWhiteTime() const
{
    return timeWhite / 1000;
}

float Clock::getBlackTime() const
{
    return timeBlack / 1000;
}

void Clock::stop()
{
    isPaused = true;
}
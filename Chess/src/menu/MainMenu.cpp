#include "MainMenu.h"
#include <iostream>

MainMenu::MainMenu(sf::RenderWindow& win, ResourceManager& rm)
    : window(win)
    , resourceManager(rm)
    , currentScreen(MenuScreen::Main)
{
    backgroundSprite = std::make_unique<sf::Sprite>(*resourceManager.getTexture("background"));
    resize();
}

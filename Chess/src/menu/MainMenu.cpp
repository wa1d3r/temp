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

void MainMenu::resize()
{
    const sf::Texture bgTex = backgroundSprite->getTexture();
    float scaleX = static_cast<float>(window.getSize().x) / bgTex.getSize().x;
    float scaleY = static_cast<float>(window.getSize().y) / bgTex.getSize().y;
    backgroundSprite->setScale(sf::Vector2f(scaleX, scaleY));
    initButtons();
}
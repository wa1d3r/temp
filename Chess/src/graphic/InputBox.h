#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>

class InputBox
{
    sf::RectangleShape shape;
    std::unique_ptr<sf::Text> text;
    std::string currentString;
    bool isFocused;
    bool onlyNumbers;
    int limit;

    sf::Color boxColor;
    sf::Color textColor;
    sf::Color focusColor;

    std::function<void(std::string)> onChange;
    void updateTextPosition();

public:
    InputBox(sf::Vector2f position, sf::Vector2f size, const sf::Font& font,
        std::string initValue = "", bool numeric = true, int charLimit = 5);

    void handleEvent(const std::optional<sf::Event>& event, const sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);

    void setString(const std::string& str);
    std::string getString() const;
    void setOnChange(std::function<void(std::string)> callback);

    // Хелпер для получения float значения (например, минут)
    float getFloatValue() const;
};
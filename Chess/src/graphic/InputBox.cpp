#include "InputBox.h"
#include <iostream>

InputBox::InputBox(sf::Vector2f position, sf::Vector2f size, const sf::Font& font,
    std::string initValue, bool numeric, int charLimit)
    : isFocused(false)
    , onlyNumbers(numeric)
    , limit(charLimit)
    , currentString(initValue)
{
    shape.setPosition(position);
    shape.setSize(size);
    shape.setOutlineThickness(2.0f);

    boxColor = sf::Color(255, 255, 255);
    focusColor = sf::Color(230, 255, 230);
    textColor = sf::Color::Black;

    shape.setFillColor(boxColor);
    shape.setOutlineColor(sf::Color(150, 150, 150));

    text = std::make_unique<sf::Text>(font, currentString);
    text->setCharacterSize(static_cast<unsigned int>(size.y * 0.55f));
    text->setFillColor(textColor);

    updateTextPosition();
}

void InputBox::draw(sf::RenderWindow& window)
{
    window.draw(shape);
    window.draw(*text);
}

void InputBox::setString(const std::string& str)
{
    currentString = str;
    text->setString(currentString);
}

std::string InputBox::getString() const
{
    return currentString;
}

void InputBox::setOnChange(std::function<void(std::string)> callback)
{
    onChange = callback;
}

float InputBox::getFloatValue() const
{
    if (currentString.empty())
        return 0.0f;
    try
    {
        return std::stof(currentString);
    }
    catch (...)
    {
        return 0.0f;
    }
}
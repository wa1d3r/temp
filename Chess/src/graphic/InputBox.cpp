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

void InputBox::updateTextPosition()
{
    float paddingLeft = 10.0f;

    float charHeight = static_cast<float>(text->getCharacterSize());
    float yOffset = (shape.getSize().y - charHeight) / 2.0f;

    text->setPosition(sf::Vector2f(
        shape.getPosition().x + paddingLeft,
        shape.getPosition().y + yOffset - (charHeight * 0.15f)));
}

void InputBox::handleEvent(const std::optional<sf::Event>& event, const sf::RenderWindow& window)
{
    if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>())
    {
        if (mouseEvent->button == sf::Mouse::Button::Left)
        {
            sf::Vector2i mousePos = mouseEvent->position;
            sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);

            bool contains = shape.getGlobalBounds().contains(worldPos);
            if (contains != isFocused)
            {
                isFocused = contains;
                shape.setFillColor(isFocused ? focusColor : boxColor);
                shape.setOutlineColor(isFocused ? sf::Color(100, 200, 100) : sf::Color(150, 150, 150));
            }
        }
    }

    if (isFocused)
    {
        if (const auto* textEvent = event->getIf<sf::Event::TextEntered>())
        {
            char32_t unicode = textEvent->unicode;

            if (unicode == 8) // Backspace
            {
                if (!currentString.empty())
                    currentString.pop_back();
            }
            else if (currentString.length() < limit)
            {
                bool isValid = true;
                if (onlyNumbers && (unicode < '0' || unicode > '9') && unicode != '.')
                    isValid = false;

                if (isValid && unicode >= 32 && unicode < 128)
                {
                    currentString += static_cast<char>(unicode);
                }
            }

            text->setString(currentString);
            if (onChange)
                onChange(currentString);
        }
    }
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
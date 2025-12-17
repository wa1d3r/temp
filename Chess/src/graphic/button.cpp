#include "button.h"
#include <iostream>

auto get_global = [](sf::Vector2f coord, sf::Vector2u window_size) {
    return sf::Vector2f(window_size.x * coord.x, window_size.y * coord.y);
};

Button::Button(sf::Vector2f position, sf::Vector2f size, sf::Vector2u window_size,
    float outline_thickness,
    std::function<void(void)> click_handler,
    const std::string& button_text,
    unsigned int text_size,
    const sf::Font* font,
    const sf::Color default_color,
    const sf::Color hover_color,
    const sf::Color text_color,
    const sf::Color outline_color,
    const sf::Texture* default_texture,
    const sf::Texture* hover_texture)
    : default_color(default_color)
    , hover_color(hover_color)
    , on_click(click_handler)
    , default_texture(default_texture)
    , hover_texture(hover_texture)
    , is_hovered(false)
{
    position = get_global(position, window_size);
    size = get_global(size, window_size);

    shape.setPosition(position);
    shape.setSize(size);
    shape.setOutlineThickness(outline_thickness * window_size.y);
    shape.setOutlineColor(outline_color);

    if (default_texture)
    {
        shape.setTexture(default_texture);
        use_texture = true;
    }
    else
    {
        shape.setFillColor(default_color);
        use_texture = false;
    }
    if (!button_text.empty())
    {
        text = std::make_unique<sf::Text>(*font, button_text, text_size);
        text->setFillColor(text_color);

        sf::FloatRect textBounds = text->getLocalBounds();
        text->setOrigin(textBounds.getCenter());
        text->setPosition(sf::Vector2f(position.x + size.x / 2.f, position.y + size.y / 2.f));
    }
}

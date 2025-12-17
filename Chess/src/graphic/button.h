#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>
#include <optional>
#include <string>

class Button
{
    sf::RectangleShape shape;
    std::unique_ptr<sf::Text> text;

    const sf::Texture* default_texture;
    const sf::Texture* hover_texture;

    std::function<void(void)> on_click;

    sf::Color default_color;
    sf::Color hover_color;

    bool is_hovered;
    bool use_texture;

public:
    Button(sf::Vector2f position, sf::Vector2f size, sf::Vector2u window_size,
        float outline_thickness,
        std::function<void(void)> click_handler,
        const std::string& button_text = "",
        unsigned int text_size = 0,
        const sf::Font* font = nullptr,
        const sf::Color default_color = sf::Color::White,
        const sf::Color hover_color = sf::Color(200, 200, 200),
        const sf::Color text_color = sf::Color::Black,
        const sf::Color outline_color = sf::Color::Transparent,
        const sf::Texture* default_texture = nullptr,
        const sf::Texture* hover_texture = nullptr);

    void draw(sf::RenderWindow& window);
    void handleEvent(const std::optional<sf::Event>& event, const sf::RenderWindow& window);

    void setTexture(const sf::Texture* texture);
    void setFillColor(sf::Color color);
    void setOutlineColor(sf::Color color);
};
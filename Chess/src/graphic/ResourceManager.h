#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <string>

class ResourceManager
{
    std::map<std::string, sf::Texture> textures;

public:
    ResourceManager() = default;

    bool loadTexture(const std::string& name, const std::string& path)
    {
        sf::Texture tex;
        if (tex.loadFromFile(path))
        {
            tex.setSmooth(true);
            textures[name] = tex;
            return true;
        }
        return false;
    }

    const sf::Texture* getTexture(const std::string& name) const
    {
        auto it = textures.find(name);
        if (it != textures.end())
        {
            return &it->second;
        }
        return nullptr;
    }

    bool loadFont(const std::string& name, const std::string& path)
    {
        sf::Font font;
        if (font.openFromFile(path))
        {
            fonts[name] = font;
            return true;
        }
        return false;
    }

    const sf::Font* getFont(const std::string& name) const
    {
        auto it = fonts.find(name);
        if (it != fonts.end())
        {
            return &it->second;
        }
        return nullptr;
    }
};
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

    bool loadTexture(std::string name, std::string path)
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

    const sf::Texture* getTexture(std::string name) const
    {
        auto it = textures.find(name);
        if (it != textures.end())
        {
            return it->second;
        }
        return nullptr;
    }
};
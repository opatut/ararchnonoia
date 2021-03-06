#ifndef RESOURCEMANAGER_HPP
#define RESOURCEMANAGER_HPP

#include <string>
#include <memory>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

class ResourceManager {
public:
    void addTexture(const std::string& name, const std::string& filename);
    std::shared_ptr<sf::Texture> getTexture(const std::string& name);

    void addFont(const std::string& name, const std::string& filename);
    std::shared_ptr<sf::Font> getFont(const std::string& name);

    void addSound(const std::string& name, const std::string& filename);
    std::shared_ptr<sf::SoundBuffer> getSound(const std::string& name);

    void addShader(const std::string& name, const std::string& filename, sf::Shader::Type type);
    std::shared_ptr<sf::Shader> getShader(const std::string& name);

    void addMusic(const std::string& name, const std::string& filename);
    std::shared_ptr<sf::Music> getMusic(const std::string& name);

private:
    std::map<std::string, std::shared_ptr<sf::Texture>> m_textures;
    std::map<std::string, std::shared_ptr<sf::Font>> m_fonts;
    std::map<std::string, std::shared_ptr<sf::Shader>> m_shaders;
    std::map<std::string, std::shared_ptr<sf::SoundBuffer>> m_sounds;
    std::map<std::string, std::string> m_musicFiles;
};

#endif

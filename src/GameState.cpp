#include "GameState.hpp"

#include <iostream>
#include <functional>

#include <Thor/Math.hpp>

#include "Wall.hpp"
#include "Pair.hpp"
#include "Root.hpp"
#include "Marker.hpp"
#include "Toy.hpp"
#include "CollisionShape.hpp"

GameState::GameState() {
    m_zoom = 6;
    m_debugDrawEnabled = false;
}

void GameState::onInit() {
    resize();

    loadLevel(1);
}

void GameState::onUpdate(float dt) {
    if(m_renderTexture.getSize() != Root().window->getSize()) {
        resize();
    }

    // m_zoom = 6;
    if(m_player) {
        float targetZoom = 6;// + m_player->physicsBody()->getLinearVelocity().length();
        float zoomSpeed = 2;
        m_zoom = m_zoom * (1 - dt * zoomSpeed) + targetZoom * (dt * zoomSpeed);

        glm::vec2 target = m_player->position() - glm::vec2(0, 0.3);

        glm::vec2 d(0.5, 0.5);
        glm::vec2 diff = target - m_center;
        diff.x = diff.x < -d.x ? -d.x : (diff.x > d.x ? d.x : diff.x);
        diff.y = diff.y < -d.y ? -d.y : (diff.y > d.y ? d.y : diff.y);
        auto new_center = target - diff;
        zoomSpeed = 8;
        m_center = m_center * (1 - dt * zoomSpeed) + new_center * (dt * zoomSpeed);
    }

    if(m_message != "") {
        m_messageTime += dt;
        if(m_messageTime > 5) {
            m_message = "";
        }
    }
}

void GameState::onDraw(sf::RenderTarget& target) {
    auto& t = m_renderTexture;

    target.clear();
    t.clear(sf::Color(80, 80, 80));

    // backdrop
    // t.setView(t.getDefaultView());
    // sf::RectangleShape backdrop(sf::Vector2f(t.getSize()));
    // backdrop.setFillColor(sf::Color(100, 150, 255));
    // backdrop.setFillColor(sf::Color(40, 40, 40));
    // auto shader = Root().resources.getShader("backdrop");
    // shader->setParameter("size", sf::Vector2f(t.getSize()));
    // shader->setParameter("time", getTime());
    // t.draw(backdrop, shader.get());

    int backTiles = 50;
    setView(t);
    auto tex = Root().resources.getTexture("cave-1");
    tex->setRepeated(true);
    sf::Sprite back(*tex.get());
    back.setTextureRect(sf::IntRect(0, 0, tex->getSize().x * backTiles, tex->getSize().y * backTiles));
    float s = 4.0;
    back.setScale(s / tex->getSize().x, s / tex->getSize().y);
    back.setPosition(m_center.x * 0.2, m_center.y * 0.2);
    back.setOrigin(tex->getSize().x / 2 * backTiles, tex->getSize().y / 2 * backTiles);

    auto levelColor = {
        sf::Color(100, 20, 0),
        sf::Color(100, 120, 200),
        sf::Color(250, 200, 0),
        sf::Color(255, 0, 128)
    };
    std::initializer_list<sf::Color> x;
    back.setColor(*(levelColor.begin() + (m_currentLevel - 1) % levelColor.size()));

    t.draw(back);

    // draw
    setView(t);
    drawEntities(t);

    // post-processing
    float w = target.getSize().x;
    float h = target.getSize().y;
    target.setView(sf::View(sf::FloatRect(0, h, w, -h)));
 
    sf::Sprite sprite;
    sprite.setTexture(m_renderTexture.getTexture());
    target.draw(sprite, Root().resources.getShader("pixel").get());

    // message
    target.setView(target.getDefaultView());
    if(m_message != "") {
        float alpha = fmin(1, fmax(0, m_messageTime)) * fmin(1, fmax(0, 4 - m_messageTime));
        alpha = tween::Cubic().easeOut(alpha, 0, 1, 1);

        sf::Text text;
        text.setFont(* Root().resources.getFont("default"));
        text.setCharacterSize(36);
        text.setString(m_message);
        text.setStyle(sf::Text::Bold);
        text.setPosition(sf::Vector2f(target.getSize().x / 2 - text.getLocalBounds().width / 2, target.getSize().y * 0.8 - fabs(sin(m_time)) * 20));
        text.setColor(sf::Color(255, 255, 255, 255 * alpha));

        sf::Vector2f b(10, 5);
        sf::RectangleShape rect(sf::Vector2f(text.getLocalBounds().width + 2 * b.x, text.getLocalBounds().height * 1.5 + 2 * b.y));
        rect.setPosition(text.getPosition() - b);
        rect.setFillColor(sf::Color(0, 0, 0, 100 * alpha));

        target.draw(rect);
        target.draw(text);
    }

    if(m_levelFade) {
        sf::RectangleShape rect(sf::Vector2f(target.getSize()));
        rect.setFillColor(sf::Color(0, 0, 0, 255 * m_levelFade));
        target.draw(rect);
    }
}

void GameState::onHandleEvent(sf::Event& event) {
    if(event.type == sf::Event::KeyPressed) {
        if(event.key.code == sf::Keyboard::Period) {
            m_debugDrawEnabled = !m_debugDrawEnabled;
            message("Debug draws toggled");
        } else if(event.key.code == sf::Keyboard::Escape) {
            Root().window->close();
        }
    } else if(event.type == sf::Event::Resized) {
        resize();
    }    
}

void GameState::resize() {
    m_renderTexture.create(Root().window->getSize().x, Root().window->getSize().y);
}


void GameState::loadLevel(int num) {
    if(num < 1 || num > LEVEL_COUNT) {
        std::cout << "Warning: level number " << num << " does not exist." << std::endl;
        return;
    }

    std::string filename = "level" + std::to_string(num) + ".dat";
    loadFromFile("levels/" + filename);

    m_player = nullptr;
    for(auto marker : getEntitiesByType<Marker>("Marker")) {
        if(marker->getType() == Marker::SPAWN) {
            if(!m_player) {
                spawnPlayer(marker->position());
            } else {
                std::cout << "Warning: level " << filename << " contains multiple SPAWN markers. Using first one." << std::endl;
            }
        }
    }

    if(!m_player) {
        std::cout << "Warning: level " << filename << " does not contain any spawn marker. Spawning at (0, 0)." << std::endl;
        spawnPlayer(glm::vec2(0, 0));
    }

    m_currentLevel = num;

    m_levelFade = 1.f;
    tween::TweenerParam p2(500, tween::SINE, tween::EASE_IN_OUT);
    p2.addProperty(&m_levelFade, 0.f);
    m_tweener.addTween(p2);

    message("Level " + std::to_string(num));
}

void GameState::spawnPlayer(const glm::vec2& pos) {
    m_player = std::make_shared<Player>();
    add(m_player);
    m_player->setPhysicsPosition(pos);
    m_center = m_player->position();
}

void GameState::switchLevel(int num) {
    if(m_nextLevel == num) return;
    tween::TweenerParam p(1500, tween::SINE, tween::EASE_IN_OUT);
    m_levelFade = 0;
    m_nextLevel = num;
    p.addProperty(&m_levelFade, 1.f);
    p.onCompleteCallBack = []() {
        Root().game_state.loadLevel(Root().game_state.m_nextLevel);
    };
    m_tweener.addTween(p);
}

void GameState::nextLevel() {
    switchLevel(m_currentLevel + 1);
}

void GameState::message(const std::string& msg) {
    if(m_message == msg) return;

    m_message = msg;
    m_messageTime = 0.f;
}

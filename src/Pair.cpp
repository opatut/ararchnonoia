#include "Pair.hpp"

#include <iostream>

#include <Thor/Math.hpp>
#include <Thor/Vectors.hpp>
#define GLM_FORCE_RADIANS
#include <glm/gtx/vector_angle.hpp>

#include <CppTweener.h>

#include "Root.hpp"

Pair::Pair() {
    m_type = 1;
    m_mass = 0.f;
    m_physicsShape = new btSphereShape(0.1);
    m_solved = false;
    m_active = false;
    m_solvedTime = 0;
    m_activationTime = 0;
    m_sound.setBuffer(*Root().resources.getSound("bell"));
    m_sound.setVolume(10);
}

std::string Pair::getTypeName() const {
    return "Pair";
}

void Pair::onAdd(State* state) {
    m_physicsBody->setCollisionFlags(m_physicsBody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
}

void Pair::setMetadata(int data) {
    if(data >= PAIR_TYPE_MIN && data <= PAIR_TYPE_MAX) {
        setType(data);
    }
}

void Pair::onUpdate(double dt) {
    if(m_active) {
        m_activationTime += dt;
    }
    if(m_solved) {
        m_solvedTime += dt;
    }
}

void Pair::onDraw(sf::RenderTarget& target) {
    glm::vec2 root(0, 0.5 * m_scale.y);
    root = glm::rotate(root, m_rotation);
    root += m_position;

    float offset = fmax(0, 1.0 - m_solvedTime);
    float height = fmin(1, fmax(0, 2.0 - m_solvedTime));
    offset = tween::Cubic().easeIn(offset, 0, 1, 1);
    height = tween::Cubic().easeIn(height, 0, 1, 1);
    offset *= 0.5 * m_scale.x;
    height *= m_scale.y;

    for(int i = 0; i < 2; ++i) {
        sf::ConvexShape shape(3);
        shape.setPoint(0, sf::Vector2f(-0.02,  0));
        shape.setPoint(1, sf::Vector2f(offset * (i == 0 ? -1 : 1), -height));
        shape.setPoint(2, sf::Vector2f( 0.02,  0));
        shape.setFillColor(sf::Color::Black);
        shape.setRotation(thor::toDegree(m_rotation));
        shape.setPosition(root.x, root.y);
        target.draw(shape);
    }

    sf::ConvexShape shape(3);
    shape.setPoint(0, sf::Vector2f(0, 0));
    shape.setPoint(1, sf::Vector2f(-offset, -height));
    shape.setPoint(2, sf::Vector2f( offset, -height));
    shape.setPosition(root.x, root.y);
    shape.setRotation(thor::toDegree(m_rotation));
    shape.setTexture(Root().resources.getTexture("spiderweb").get());
    target.draw(shape);

    if(m_active || m_solved) {
        auto tex = Root().resources.getTexture("blob");
        auto s = tex->getSize();
        sf::Sprite sprite(*tex.get());
        sf::Color c = getColor();
        c.a = 255 * fmax(0, fmin(1, m_activationTime)) * fmax(0, fmin(1, 1.5 - m_solvedTime));
        sprite.setColor(c);
        auto pos = m_position + glm::rotate(glm::vec2(0, -1), m_rotation) * tween::Cubic().easeOut(m_solvedTime, 0, 1, 1);
        sprite.setPosition(pos.x, pos.y);
        sprite.setScale(0.3 / s.x, (0.3 + 0.2 * abs(sin(m_activationTime * 2))) / s.y);
        sprite.setOrigin(0.6 * s.x, 0.6 * s.y);
        sprite.setRotation(thor::toDegree(m_rotation));
        target.draw(sprite);
    }
}

std::vector<std::shared_ptr<Pair>> Pair::findMatchingPairs() {
    std::vector<std::shared_ptr<Pair>> r;
    for(auto entity : m_state->getEntities()) {
        if(entity->getTypeName() == "Pair") {
            auto p = std::static_pointer_cast<Pair>(entity);
            if(p->m_type == m_type) {
                r.push_back(p);
            }
        }
    }
    return r;
}

void Pair::deactivateAllOtherPairs() {
    for(auto entity : m_state->getEntities()) {
        if(entity->getTypeName() == "Pair") {
            auto p = std::static_pointer_cast<Pair>(entity);
            if(p->m_type != m_type) {
                p->deactivate();
            }
        }
    }
}

void Pair::setType(int type) {
    m_type = type;
}

void Pair::activate() {
    if(m_solved) return;

    if(!m_active) {
        m_active = true;
        m_activationTime = 0;

        deactivateAllOtherPairs();

        auto pairs = findMatchingPairs();
        unsigned int activeCount = 0;
        for(auto p : pairs) {
            if(p->m_active) activeCount++;
        }
        m_sound.setPitch(1.f);
        if(activeCount == pairs.size()) {
            for(auto p : pairs) {
                p->solve();
            }
            m_sound.setPitch(1.5f);
        }
        m_sound.play();
    }
}

void Pair::deactivate() {
    m_active = false;
}

void Pair::solve() {
    if(!m_solved) {
        m_solved = true;
        m_solvedTime = 0;
    }
}

sf::Color Pair::getColor() const {
    if(m_type == 1) {
        return sf::Color::Cyan;
    } else if(m_type == 2) {
        return sf::Color::Yellow;
    } else if(m_type == 3) {
        return sf::Color::Blue;
    } else if(m_type == 4) {
        return sf::Color::Magenta;
    } else if(m_type == 5) {
        return sf::Color::Green;
    } else if(m_type == 6) {
        return sf::Color::Red;
    } else {
        return sf::Color::Black;
    }
}

bool Pair::isActive() const {
    return m_active;
}

bool Pair::isSolved() const {
    return m_solved;
}

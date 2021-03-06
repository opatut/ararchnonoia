#include "Marker.hpp"

#include "State.hpp"
#include "Root.hpp"

Marker::Marker() {
    m_zLevel = 500;
    m_type = NONE;
}

std::string Marker::getTypeName() const {
    return "Marker";
}

void Marker::onInitialize() {
    if(m_type == GOAL) {
        m_physicsShape = new btSphereShape(0.01);
    }
}

void Marker::onAdd(State* state) {
    if(m_physicsBody) {
        m_physicsBody->setCollisionFlags(m_physicsBody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    }
}

void Marker::onDraw(sf::RenderTarget& target) {
    if(m_state != &Root().editor_state) return;

    auto pix = m_state->getPixelSize();

    sf::CircleShape shape(pix * 10);
    shape.setOutlineColor(sf::Color::White);
    shape.setPosition(m_position.x, m_position.y);
    shape.setOrigin(pix * 10, pix * 10);
    if(m_type == SPAWN) {
        shape.setFillColor(sf::Color::Red);
    } else if(m_type == GOAL) {
        shape.setFillColor(sf::Color::Green);
    } else if(m_type == HELP_TRIGGER) {
        shape.setFillColor(sf::Color::Yellow);
    } else {
        shape.setFillColor(sf::Color::White);
    }
    target.draw(shape);
}

void Marker::setMetadata(int data) {
    if(data >= 1 && data <= 3) {
        m_type = (Type)data;
    }
}

Marker::Type Marker::getType() const {
    return m_type;
}

glm::vec2 Marker::getSize() {
    auto s = 20 * m_state->getPixelSize();
    return glm::vec2(s, s);
}

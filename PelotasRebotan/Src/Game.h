#ifndef GAME_H
#define GAME_H

#include "Avatar.h"
#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include "SFMLRenderer.h"

using namespace sf;

// Definición de la clase QueryCallback
class QueryCallback : public b2QueryCallback
{
public:
    QueryCallback(const b2Vec2& point) : point(point), found(false), body(nullptr) {}

    bool ReportFixture(b2Fixture* fixture) override
    {
        b2Body* body = fixture->GetBody();
        if (fixture->TestPoint(point))
        {
            found = true;
            this->body = body;
            return false; // Termina la búsqueda
        }
        return true; // Continúa la búsqueda
    }

    b2Vec2 point;
    bool found;
    b2Body* body;
};

class Game
{
public:
    Game(int ancho, int alto, std::string titulo);
    void Loop();

private:
    int ancho, alto;
    RenderWindow* wnd;
    int fps;
    float frameTime;
    b2World* phyWorld;
    SFMLRenderer* debugRender;
    b2Body* controlBody;
    b2Body* controlBody2;
    Avatar* controlBodyAvatar;
    Avatar* controlBodyAvatar2;
    b2MouseJoint* mouseJoint = nullptr;
    b2Body* selectedBody = nullptr;
    b2Body* groundBody;
    Texture texturaPelota;
    Color clearColor = Color::Black;

    void UpdatePhysics();
    void DrawGame();
    void DoEvents();
    void SetZoom();
    void InitPhysics();
    void OnMousePress(sf::Event::MouseButtonEvent mouseEvent);
    void OnMouseRelease(sf::Event::MouseButtonEvent mouseEvent);
    void OnMouseMove(sf::Event::MouseMoveEvent mouseEvent);
};

#endif
#include "Game.h"
#include "Box2DHelper.h"

// Constructor de la clase Game
Game::Game(int ancho, int alto, std::string titulo)
{
    this->ancho = ancho;
    this->alto = alto;
    wnd = new RenderWindow(VideoMode(ancho, alto), titulo);
    wnd->setVisible(true);
    fps = 60;
    wnd->setFramerateLimit(fps);
    frameTime = 1.0f / fps;
    SetZoom(); // Configuración del zoom de la cámara
    InitPhysics(); // Inicialización del mundo físico
}

// Método principal que maneja el bucle del juego
void Game::Loop()
{
    sf::Clock clock;
    while (wnd->isOpen())
    {
        float deltaTime = clock.restart().asSeconds();
        wnd->clear(clearColor); // Limpia la ventana con un color especificado
        DoEvents(); // Procesa los eventos del sistema
        UpdatePhysics(); // Actualiza la simulación física
        controlBodyAvatar->Actualizar(); // Actualiza la posición de la primera pelota
        controlBodyAvatar2->Actualizar(); // Actualiza la posición de la segunda pelota
        DrawGame(); // Dibuja el juego en la ventana
        wnd->display(); // Muestra la ventana renderizada
    }
}

// Actualiza la simulación física
void Game::UpdatePhysics()
{
    phyWorld->Step(frameTime, 8, 8); // Avanza la simulación física
    phyWorld->ClearForces(); // Limpia las fuerzas aplicadas a los cuerpos
    phyWorld->DebugDraw();
}

// Dibuja los elementos del juego en la ventana
void Game::DrawGame()
{
    // Suelo
    sf::RectangleShape groundShape(sf::Vector2f(500, 5));
    groundShape.setFillColor(sf::Color::Red);
    groundShape.setPosition(0, 95);
    wnd->draw(groundShape);

    // Techo
    sf::RectangleShape roofShape(sf::Vector2f(500, 5));
    roofShape.setFillColor(sf::Color::Blue);
    roofShape.setPosition(0, 0);
    wnd->draw(roofShape);

    // Pared izquierda
    sf::RectangleShape leftWallShape(sf::Vector2f(5, alto));
    leftWallShape.setFillColor(sf::Color::Green);
    leftWallShape.setPosition(0, 0);
    wnd->draw(leftWallShape);

    // Pared derecha
    sf::RectangleShape rightWallShape(sf::Vector2f(5, alto));
    rightWallShape.setFillColor(sf::Color::Yellow);
    rightWallShape.setPosition(ancho - 5, 0);
    wnd->draw(rightWallShape);

    controlBodyAvatar->Dibujar(*wnd); // Dibuja el avatar de la primera pelota en la ventana
    controlBodyAvatar2->Dibujar(*wnd); // Dibuja el avatar de la segunda pelota en la ventana
}

// Procesa los eventos del sistema
void Game::DoEvents()
{
    Event evt;
    while (wnd->pollEvent(evt))
    {
        switch (evt.type)
        {
        case Event::Closed:
            wnd->close();
            break;
        case Event::MouseButtonPressed:
            OnMousePress(evt.mouseButton);
            break;
        case Event::MouseButtonReleased:
            OnMouseRelease(evt.mouseButton);
            break;
        case Event::MouseMoved:
            OnMouseMove(evt.mouseMove);
            break;
        }
    }
}

// Maneja el evento de presionar el botón del ratón
void Game::OnMousePress(sf::Event::MouseButtonEvent mouseEvent)
{
    // Obtener las coordenadas del mouse en el mundo físico
    sf::Vector2f mousePos = wnd->mapPixelToCoords(sf::Vector2i(mouseEvent.x, mouseEvent.y));
    b2Vec2 worldMousePos(mousePos.x, mousePos.y);

    // Crear un query para detectar si se hizo clic sobre alguna de las pelotas
    b2AABB aabb;
    b2Vec2 d(0.001f, 0.001f);
    aabb.lowerBound = worldMousePos - d;
    aabb.upperBound = worldMousePos + d;

    QueryCallback queryCallback(worldMousePos);
    phyWorld->QueryAABB(&queryCallback, aabb);

    if (queryCallback.found)
    {
        selectedBody = queryCallback.body;
        b2MouseJointDef mouseJointDef;
        mouseJointDef.bodyA = groundBody;
        mouseJointDef.bodyB = selectedBody;
        mouseJointDef.target = worldMousePos;
        mouseJointDef.maxForce = 1000.0f * selectedBody->GetMass();
        mouseJoint = (b2MouseJoint*)phyWorld->CreateJoint(&mouseJointDef);
    }
}

// Maneja el evento de soltar el botón del ratón
void Game::OnMouseRelease(sf::Event::MouseButtonEvent mouseEvent)
{
    if (mouseJoint)
    {
        phyWorld->DestroyJoint(mouseJoint);
        mouseJoint = nullptr;
        selectedBody = nullptr;
    }
}

// Maneja el evento de mover el ratón
void Game::OnMouseMove(sf::Event::MouseMoveEvent mouseEvent)
{
    if (mouseJoint)
    {
        // Obtener las coordenadas del mouse en el mundo físico
        sf::Vector2f mousePos = wnd->mapPixelToCoords(sf::Vector2i(mouseEvent.x, mouseEvent.y));
        b2Vec2 worldMousePos(mousePos.x, mousePos.y);
        mouseJoint->SetTarget(worldMousePos);
    }
}

// Configura el área visible en la ventana de renderizado
void Game::SetZoom()
{
    View camara;
    camara.setSize(100.0f, 100.0f);
    camara.setCenter(50.0f, 50.0f);
    wnd->setView(camara);
}

// Inicializa las físicas
void Game::InitPhysics()
{
    phyWorld = new b2World(b2Vec2(0.0f, 9.8f)); // Gravedad del mundo físico

    debugRender = new SFMLRenderer(wnd);
    debugRender->SetFlags(UINT_MAX); // Configura el renderizado para que muestre todo
    phyWorld->SetDebugDraw(debugRender);

    // Crea el suelo
    groundBody = Box2DHelper::CreateRectangularStaticBody(phyWorld, 100, 10);
    groundBody->SetTransform(b2Vec2(50.0f, 100.0f), 0.0f);

    // Crea el techo
    b2Body* roofBody = Box2DHelper::CreateRectangularStaticBody(phyWorld, 100, 10);
    roofBody->SetTransform(b2Vec2(50.0f, 0.0f), 0.0f);

    // Crea la pared izquierda
    b2Body* leftWallBody = Box2DHelper::CreateRectangularStaticBody(phyWorld, 10, 100);
    leftWallBody->SetTransform(b2Vec2(0.0f, 50.0f), 0.0f);

    // Crea la pared derecha
    b2Body* rightWallBody = Box2DHelper::CreateRectangularStaticBody(phyWorld, 10, 100);
    rightWallBody->SetTransform(b2Vec2(100.0f, 50.0f), 0.0f);

    // Carga la textura de la pelota y obtiene sus dimensiones
    texturaPelota.loadFromFile("Pelota.png"); // Carga de la textura de la pelota
    sf::Vector2u spriteSize = texturaPelota.getSize();
    float spriteRadius = spriteSize.x / 2.0f; // Asume que el sprite es cuadrado

    // Ajusta el tamaño del sprite a las coordenadas del mundo
    float worldSpriteRadius = spriteRadius / 10.0f; // Ajuste según el zoom del mundo (10 píxeles = 1 unidad)

    // Crea el cuerpo dinámico de la primera pelota
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(50.0f, 50.0f); // Posición inicial
    controlBody = phyWorld->CreateBody(&bodyDef);

    // Crea la forma de la pelota
    b2CircleShape circleShape;
    circleShape.m_radius = worldSpriteRadius; // Radio de la pelota ajustado al sprite

    // Propiedades de la pelota
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &circleShape;
    fixtureDef.density = 1.9f; // Densidad de la pelota
    fixtureDef.friction = 0.1f; // Fricción de la pelota
    fixtureDef.restitution = 1.8f; // Coeficiente de restitución

    controlBody->CreateFixture(&fixtureDef); // Crea el fixture con las propiedades definidas

    // Crea el cuerpo dinámico de la segunda pelota
    bodyDef.position.Set(60.0f, 50.0f); // Posición inicial
    controlBody2 = phyWorld->CreateBody(&bodyDef);

    // Propiedades de la segunda pelota
    controlBody2->CreateFixture(&fixtureDef); // Crea el fixture con las propiedades definidas

    // Crea el resorte entre las dos pelotas
    b2DistanceJointDef jointDef;
    jointDef.bodyA = controlBody;
    jointDef.bodyB = controlBody2;
    jointDef.localAnchorA.Set(0, 0);
    jointDef.localAnchorB.Set(0, 0);
    jointDef.length = 10.0f; // Longitud del resorte
    jointDef.damping = 0.2;
    jointDef.stiffness = 1.9;
    phyWorld->CreateJoint(&jointDef);

    // Crea el avatar de la primera pelota con la textura
    controlBodyAvatar = new Avatar(controlBody, new sf::Sprite(texturaPelota));

    // Crea el avatar de la segunda pelota con la textura
    controlBodyAvatar2 = new Avatar(controlBody2, new sf::Sprite(texturaPelota));
}
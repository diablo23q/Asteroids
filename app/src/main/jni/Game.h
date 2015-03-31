#pragma once

#include <vector>
#include <list>

#include <GameObject.h>
#include <Score.h>

class Game {
    //Синглтон, приватные конструкторы
    Game();
    Game(const Game&);
    Game& operator=(const Game&);

    //Все игровые объекты обитают здесь
    std::list<std::unique_ptr<GameObject>> objects;
    //Game распоряжается временем жизни объектов
    void DestroyRequestedObjects();

    Timer timer;

    //Игровая логика
    Vec2 playerPos;
    int asteroidCount;
    bool isUfoPresent;
    void ResetLogic();
    void SpawnAsteroids(int n);
    Transform GetSpawnPosition();
    Transform GetUfoSpawn();

    //Запуск/перезапуск
    bool isLevelRunning;
    bool wantRestart;
    float restartTimer;
    void Restart();
    bool IsLevelRunning(float dt);

    //Обнаружение коллизий
    void DetectCollisions(float dt);
    bool CollisionMask(const GameObject& a, const GameObject& b);
    bool DetectCollision(const GameObject& a, const GameObject& b, float dt);
    bool RefineCollision(const GameObject& a, const GameObject& b, float dt);
    bool SegmentCollision(Vec2 p, Vec2 r, Vec2 q, Vec2 s);
    bool MovingSegmentCollision(Vec2 p, Vec2 r, Vec2 vp, Vec2 q, Vec2 s, Vec2 vq, float dt);

public:
    static Game& Get();
    void Update();

    //Обновление рендеринга
    void OnGLInit();
    void OnResolutionChange(int w, int h);

    //Единственный способ добавить объект в игру - передать владение к Game
    GameObject& AddGameObject(std::unique_ptr<GameObject> obj);

    //Запуск/перезапуск. Можно запросить перезапуск через некоторое время
    //Например, в случае уничтожения корабля
    void RequestRestart(float timer = 0.0f);
    void Pause();
    void Resume();

    //Игровая логика
    void AddPoints(int pointsToAdd);
    void SetPlayerPos(const Ship& player);
    Vec2 GetPlayerPos();
    void IncAsteroidCount(const Asteroid& a);
    void DecAsteroidCount(const Asteroid& a);
    void OnUfoCreated(const UFO& u);
    void OnUfoDestroyed(const UFO& u);

};
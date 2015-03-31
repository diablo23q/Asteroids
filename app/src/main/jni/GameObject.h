#pragma once

#include <memory>

#include <Renderer.h>

enum class GOType{
    Ship, Asteroid, UFO, Bullet, Explosion
};


///Base Class///

class GameObject {
    static int currentId;
    int mId;
    bool aboutToDestroy = false;
    void setId(int id) {mId = id;};
    static GameObject& PushToGame(std::unique_ptr<GameObject>&& obj);

protected:
    Vec2 vel;
    Transform t; //где отрисовываем
    std::shared_ptr<Model> model; //что отрисовываем

    void Move(float deltaTime);
    void Rotate(float deltaAngle);

    //Нельзя создать вне метода Create
    GameObject(const Transform& pos, std::shared_ptr<Model> mod);

    //Нельзя удалить, не удалив unique_ptr
    virtual ~GameObject();
    friend std::unique_ptr<GameObject>::deleter_type;

public:
    //Нельзя скопировать
    GameObject(const GameObject&) = delete;
    GameObject& operator=(const GameObject&) = delete;

    //Создаём объект производного типа, назначем уникальный id, передаем в Game, возвращаем ссылку
    template <class T>
    static GameObject& Create(const Transform& pos, std::shared_ptr<Model> mod = nullptr) {
        auto obj = std::unique_ptr<GameObject>(new T(pos, mod));;
        currentId++;
        obj->setId(currentId);
        return PushToGame(std::move(obj));
    };

    template <class T>
    static GameObject& Create(float x = 0.f, float y = 0.f, float a = 0.f, std::shared_ptr<Model> mod = nullptr) {
        return Create<T>(Transform(x, y, a), mod);
    };

    //Потенциально небезопасное привидение, но синтаксис призван не дать запутаться
    template <class T> T& as() { return *static_cast<T*>(this); }
    template <class T> const T& as() const { return *static_cast<const T*>(this); }

    //Запросить удаление объекта в ближайшем Update
    void RequestDestruction();
    bool isDestructionRequested() const { return aboutToDestroy; };

    int getId() const { return mId; };
    void setVelocity(const Vec2& velocity) { vel = velocity; };
    Vec2 getVelocity() const { return vel; };
    Vec2 getPosition() const { return t.getPos(); };
    float getRadius() const { return model->getRadius(); };
    const Model& getModel() const { return *model; };

    virtual GOType getStaticType() const = 0; //Нельзя создать объект
    virtual void Update(float dt) { Move(dt); };
    //Вернуть true, если требуется обработка столкновений
    virtual bool CollisionMask(GOType type) const {return false;};
    virtual void OnCollision(const GameObject& obj) { RequestDestruction(); };
};


///Ship///

class Ship : public GameObject {
    Vec2 acc;
    const float rotSpeed = Constant::shipRotationSpeed;
    const float throttle = Constant::shipThrottle;
    const float friction = Constant::shipFriction;
    const float cooldown = Constant::shipShootingCooldown;
    float cooldownTimer = 0.f;
    const float teleportcd = Constant::shipTeleportCooldown;
    float teleportTimer = 0.f;
    float pointsTimer = Constant::pointsTimeInterval;
    std::shared_ptr<Model> engine;

protected:
    friend class GameObject;
    Ship(const Transform& pos, std::shared_ptr<Model> mod);
    virtual ~Ship();

public:
    GOType getStaticType() const override {return GOType::Ship;};
    void Update (float dt) override;
    bool CollisionMask(GOType type) const override;
    void OnCollision(const GameObject& obj) override;
};


///Asteroid///

class Asteroid : public GameObject {
    void Split(const GameObject& hitObj);

protected:
    friend class GameObject;
    Asteroid(const Transform& pos, std::shared_ptr<Model> mod);
    virtual ~Asteroid() {};

public:
    GOType getStaticType() const override {return GOType::Asteroid;};
    bool CollisionMask(GOType type) const override;
    void OnCollision(const GameObject& obj) override;

    bool isSmall() const;
};


///Bullet///

class Bullet : public GameObject {
    bool isShotByPl = false;
    float lifetime = Constant::bulletLifetime;

protected:
    friend class GameObject;
    Bullet(const Transform& pos, std::shared_ptr<Model> mod);
    virtual ~Bullet() {};

public:
    GOType getStaticType() const override {return GOType::Bullet;};
    void Update (float dt) override;
    bool CollisionMask(GOType type) const override;
    void OnCollision(const GameObject& obj) override;

    void shotByPlayer(bool isIt = true) {isShotByPl = isIt;};
    bool isShotByPlayer() const {return isShotByPl;};
};


///Explosion///

class Explosion : public GameObject {
    float lifetime = Constant::explosionLifetime;

protected:
    friend class GameObject;
    Explosion(const Transform& pos, std::shared_ptr<Model> mod);
    virtual ~Explosion() {};

public:
    GOType getStaticType() const override {return GOType::Explosion;};
    void Update (float dt) override;
};


///UFO///

class UFO : public GameObject {
    float cooldownTimer = Constant::ufoCooldown;

protected:
    friend class GameObject;
    UFO(const Transform& pos, std::shared_ptr<Model> mod);
    virtual ~UFO() {};

public:
    GOType getStaticType() const override {return GOType::UFO;};
    void Update (float dt) override;
    bool CollisionMask(GOType type) const override;
    void OnCollision(const GameObject& obj) override;
};
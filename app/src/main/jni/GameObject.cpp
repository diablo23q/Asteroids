#include <GameObject.h>
#include <Controls.h>
#include <Wrapper.h>
#include <Game.h>

//Для упрощения создания базового класса. Если в Create передана модель,
//то используем её, если нет, то создаём модель по умолчанию согласно типу
#define InitBase(type) GameObject(pos, mod ? mod : Model::Create##type())

int GameObject::currentId = 0;

GameObject& GameObject::PushToGame(std::unique_ptr<GameObject>&& obj) {
    return Game::Get().AddGameObject(std::move(obj));
}

void GameObject::RequestDestruction() {
    aboutToDestroy = true;
}

GameObject::GameObject(const Transform& pos, std::shared_ptr<Model> mod) : t(pos), model(mod) {
    //Если всё же кто-то передал mod == nullptr, то упадём здесь (а не внезапно где-нибудь позже)
    t.setMargin(model->getRadius());
    Renderer::GetGameObjectBatch().Add(model);//Добавляем модель на отрисовку
    model->ApplyTransform(t);
}

GameObject::~GameObject() {
   Renderer::GetGameObjectBatch().Remove(model);
}

void GameObject::Move(float deltaTime) {
    t.setPos(t.getPos() + vel * deltaTime);
    model->ApplyTransform(t);
}

void GameObject::Rotate(float deltaAngle) {
    t.setAngle(t.getAngle() + deltaAngle);
}


///Ship///

Ship::Ship(const Transform& pos, std::shared_ptr<Model> mod) : InitBase(Ship) {
    //Дополнительная модель
    engine = Model::CreateShipEngine();
    Renderer::GetGameObjectBatch().Add(engine);
};

Ship::~Ship() {
    Renderer::GetGameObjectBatch().Remove(engine);
};

void Ship::Update(float dt) {
    float fwd = Controls::Forward();
    engine->setDraw(fwd > 0.f); //Отрисовываем только когда двигатель включен (жмем вперед)

    Rotate(rotSpeed * dt * Controls::HorAxis());
    //Ускоряемся в направлении корабля, замедляемся в направлении скорости
    acc = (t.getDirection() * fwd * throttle) - (vel * friction);
    vel = vel + acc * dt;
    Move(dt);
    engine->ApplyTransform(t);
    Game::Get().SetPlayerPos(*this); //Оповещаем игровую логику об изменении позиции

    //Стреляем с кулдауном
    if(cooldownTimer <= 0.f) {
        if(Controls::Shooting()) {
            cooldownTimer = cooldown;
            //Создаём пулю в районе носа корабля, угол поворота пули совпадает с углом поворота корабля
            Vec2 spawn = t.getPos() + t.getDirection() * model->getRadius() * 0.5f;
            Bullet& b = GameObject::Create<Bullet>(spawn.x, spawn.y, t.getAngle()).as<Bullet>();
            //Пуля летит в направлении корабля в момент выстрела. К ее скорости добавляется часть скорости корабля
            b.setVelocity(t.getDirection() * (Constant::bulletSpeedBasic + vel.getLength() * Constant::bulletSpeedInherited));
            b.shotByPlayer(true);
        }
    } else {
        cooldownTimer -= dt;
    }

    //Телепортируемся в случайную точку на экране
    if(teleportTimer <= 0.f) {
        if(Controls::Teleport()) {
            std::uniform_real_distribution<float> w(-Constant::worldRatio, Constant::worldRatio);
            std::uniform_real_distribution<float> h(-1.f, 1.f);
            teleportTimer = teleportcd;
            t.setPos(Vec2(w(Random::generator), h(Random::generator)));
            setVelocity(Vec2());
        }
    } else {
        teleportTimer -= dt;
    }

    //Начисляем очки за каждые прожитые pointsTimeInterval секунд
    if(pointsTimer <= 0.f) {
        pointsTimer = Constant::pointsTimeInterval;
        Game::Get().AddPoints(Constant::pointsForTime);
    } else {
        pointsTimer -= dt;
    }
}

bool Ship::CollisionMask(GOType type) const {
    switch(type) {
    case GOType::Asteroid:
    case GOType::Bullet:
    case GOType::UFO:
        return true;
    default:
        return false;
    };
}

void Ship::OnCollision(const GameObject& obj) {
    if(obj.getStaticType() == GOType::Bullet) {
        //Не умираем от своей пули
        if(obj.as<Bullet>().isShotByPlayer()) return;
    }
    GameObject::Create<Explosion>(t); //Взрыв на месте уничтожения
    RequestDestruction();
    if(Constant::vibrateOnDeath) JavaCall::Vibrate();
    //Запрашиваем перезапуск игры через некоторое время после уничтожения
    Game::Get().RequestRestart(Constant::restartAfterDeathSec);
}


///Asteroid///

Asteroid::Asteroid(const Transform& pos, std::shared_ptr<Model> mod) : InitBase(Asteroid) {
    //Если астероид большой, то летим в случайном направлении с постоянной скоростью
    if(!isSmall())  {
        std::uniform_real_distribution<float> direction(0, 2*M_PI);
        float angle = direction(Random::generator);
        setVelocity(Vec2(cos(angle), sin(angle)) * Constant::asteroidSpeed);
        Game::Get().IncAsteroidCount(*this);
    }
    //Если маленький, то скорость сообщат снаружи через SetVelocity
}

bool Asteroid::CollisionMask(GOType type) const {
    switch(type) {
    case GOType::Ship:
    case GOType::Bullet:
        return true;
    default:
        return false;
    };
}

//Размер астероида определяем по количеству вершин в модели
bool Asteroid::isSmall() const {
    return model->getVerts().size() < Constant::asteroidVertCount * 2;
}

//Разбиваем астероид при столкновении с объектом hitObj
void Asteroid::Split(const GameObject& hitObj) {
    Vec2 hitPos = hitObj.getPosition();
    const std::vector<GLfloat>& verts = model->getVerts();
    const std::vector<GLfloat>& tverts = model->getTransformed();
    float minDist = Constant::bigNumber;
    int minInd = 0;

    //Находим ближайшую к hitObj вершину астероида
    for(int i = 0; i < verts.size(); i += 2) {
        float dist = Vec2(tverts[i], tverts[i + 1]).getSquaredDist(hitPos);
        if(dist < minDist) {
            minDist = dist;
            minInd = i;
        }
    }

    //Находим противоположную вершину
    int maxInd = (minInd + tverts.size() / 2) % tverts.size();
    //Направление разреза астероида
    Vec2 div = Vec2(tverts[minInd], tverts[minInd + 1]) - Vec2(tverts[maxInd], tverts[maxInd + 1]);
    //Направление, перпендикулярное разрезу
    Vec2 orthoVel = div.getOrthogonal().getNormalized() * Constant::asteroidBlastImpact;
    //Упорядочиваем индексы вершин разреза для удобства разделения
    bool swapped = false;
    if(minInd > maxInd) {
        std::swap(minInd, maxInd);
        swapped = true;
    }

    //Распределяем вершины текущего астероида по новым
    std::vector<GLfloat> v1, v2;
    for(int i = 0; i < verts.size(); i += 2) {
        if(i > minInd && i < maxInd) {
            v1.push_back(verts[i]);
            v1.push_back(verts[i + 1]);
        } else if(i == minInd || i == maxInd) {
            v1.push_back(verts[i]);
            v1.push_back(verts[i + 1]);
            v2.push_back(verts[i]);
            v2.push_back(verts[i + 1]);
        } else {
            v2.push_back(verts[i]);
            v2.push_back(verts[i + 1]);
        }
    }

    //Скорость новых астероидов определяется скоростью оригинального,
    //скоростью объекта столкновения и направлением разреза
    Vec2 vel = getVelocity() + hitObj.getVelocity() * Constant::asteroidBulletImpact;
    //Смотрим на swapped, чтобы обеспечить разлет половинок в корректном направлении
    GameObject::Create<Asteroid>(t, std::make_shared<Model>(swapped ? v2 : v1)).setVelocity(vel - orthoVel);
    GameObject::Create<Asteroid>(t, std::make_shared<Model>(swapped ? v1 : v2)).setVelocity(vel + orthoVel);
}

void Asteroid::OnCollision(const GameObject& obj) {
    switch(obj.getStaticType()) {
    case GOType::Bullet:
        if(obj.as<Bullet>().isShotByPlayer()) {
            //Начисляем очки, только если асетроид уничтожен игроком
            Game::Get().AddPoints(isSmall() ? Constant::pointsForSmall : Constant::pointsForLarge);
        }
    case GOType::Ship:
        //Большой астероид делим пополам и уничтожаем
        if(!isSmall()) Split(obj);
        //Маленький уничтожаем и оповещаем логику
        GameObject::Create<Explosion>(t);
        RequestDestruction();
        if(isSmall()) Game::Get().DecAsteroidCount(*this);
        return;
    default:
        return;
    };
}


///Bullet///

Bullet::Bullet(const Transform& pos, std::shared_ptr<Model> mod) : InitBase(Bullet) {

}

void Bullet::Update(float dt) {
    //Пуля летит с постоянной скоростью lifetime секунд
    Move(dt);

    lifetime -= dt;
    if(lifetime <= 0.f) {
        RequestDestruction();
    }
}

bool Bullet::CollisionMask(GOType type) const {
    switch(type) {
    case GOType::Asteroid:
    case GOType::UFO:
    case GOType::Ship:
        return true;
    default:
        return false;
    };
}

void Bullet::OnCollision(const GameObject& obj) {
    switch(obj.getStaticType()) {
    case GOType::Ship:
        if(isShotByPlayer()) return;
    case GOType::UFO:
         if(!isShotByPlayer()) return;
    case GOType::Asteroid:
        RequestDestruction();
        return;
    default:
        return;
    };
}


///Explosion///

Explosion::Explosion(const Transform& pos, std::shared_ptr<Model> mod) : InitBase(Explosion) {

}

void Explosion::Update(float dt) {
    //Квдратично увеличиваем масштаб в течение lifetime секунд
    lifetime -= dt;
    float scale = 1 + sqrt(1 - lifetime / Constant::explosionLifetime) * Constant::explosionMaxScale;
    t.setScale(Vec2(scale, scale));
    model->ApplyTransform(t);
    if(lifetime <= 0.f) {
        RequestDestruction();
    }
}


///UFO///

UFO::UFO(const Transform& pos, std::shared_ptr<Model> mod) : InitBase(UFO) {
    //НЛО летает горизонтально с постоянной скоростью
    setVelocity(Vec2(Constant::ufoSpeed, 0.f));
    //Чтобы корректно обрабатывать логику перемещений (см Update),
    //НЛО может залетать чуть дальше за границу экрана
    t.setMargin(model->getRadius() + Constant::ufoMargin);
    Game::Get().OnUfoCreated(*this);
}

void UFO::Update(float dt) {
    Move(dt);
    //При залете за край экрана НЛО меняет направление
    //и выбирает другую горизонтальную линию в пределах ufoZone
    if(Constant::worldRatio + model->getRadius() - fabs(t.getPos().x) < 0.f) {
        std::uniform_real_distribution<float> pos(-Constant::ufoZone, Constant::ufoZone);
        t.setPos(Vec2(t.getPos().x, pos(Random::generator)));
        setVelocity(Vec2(-vel.x, 0.f));
    }

    //НЛО стреляет в направлении игрока с некоторым шансом промахнуться
    if(cooldownTimer <= 0.f) {
        cooldownTimer = Constant::ufoCooldown;
        std::uniform_real_distribution<float> miss(-Constant::ufoAccuracy, Constant::ufoAccuracy);
        Vec2 missVec = Vec2(miss(Random::generator), miss(Random::generator));
        Vec2 dir = (Game::Get().GetPlayerPos() + missVec - t.getPos()).getNormalized();
        Vec2 spawn = t.getPos() + dir * model->getRadius() * 0.5f;
        float angle = dir.y > 0 ? acos(dir.x) + M_PI/2 : M_PI/2 - acos(dir.x);
        Bullet& b = GameObject::Create<Bullet>(spawn.x, spawn.y, angle).as<Bullet>();
        b.setVelocity(dir * Constant::bulletSpeedBasic);
        b.shotByPlayer(false);
    } else {
        cooldownTimer -= dt;
    }
}

//НЛО не сталкивается с астероидами
bool UFO::CollisionMask(GOType type) const {
    switch(type) {
    case GOType::Bullet:
    case GOType::Ship:
        return true;
    default:
        return false;
    };
}

void UFO::OnCollision(const GameObject& obj) {
    if(obj.getStaticType() == GOType::Bullet) {
        if(!obj.as<Bullet>().isShotByPlayer()) return;
    }
    GameObject::Create<Explosion>(t);
    RequestDestruction();
    Game::Get().OnUfoDestroyed(*this);
    Game::Get().AddPoints(Constant::pointsForUFO);
}
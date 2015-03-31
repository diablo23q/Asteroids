#include <Game.h>
#include <Controls.h>

#include <tuple>

Game::Game() {
    isLevelRunning = true;

    ResetLogic();
    RequestRestart();

    Renderer::InitInternals();
    Controls::Init();
    Score::Init();
}

Game& Game::Get() {
    static Game instance;
    return instance;
}

void Game::Restart() {
    objects.clear();
    Score::OnRestart();
    ResetLogic();
    GameObject::Create<Ship>();
    SpawnAsteroids(Constant::asteroidTargetCount);
}

//Внутри управляем рестартом, а наружу выдаем текущее состояние уровня (false - на паузе)
bool Game::IsLevelRunning(float dt) {
    if(wantRestart) {
        if(restartTimer < 0.0) {
            if(isLevelRunning) {
                Restart();
                wantRestart = false;
            }
        } else {
            restartTimer -= dt;
        }
    }
    return isLevelRunning;
}

void Game::Update() {
    float deltaTime = timer.Tick();

    if(IsLevelRunning(deltaTime)) {
        for(auto& go : objects) {
            go->Update(deltaTime);
        }

        DetectCollisions(deltaTime);

        DestroyRequestedObjects(); //Удаление объектов предполагается только здесь
    }

    Renderer::Draw();
}

void Game::OnGLInit() {
    Renderer::InitGLContext();
}

void Game::OnResolutionChange(int w, int h) {
    Renderer::OnResolutionChange(w, h);
    Controls::Resize();
    Score::Resize();
}

GameObject& Game::AddGameObject(std::unique_ptr<GameObject> obj) {
    objects.push_back(std::move(obj));
    return *objects.back().get();
}

void Game::DestroyRequestedObjects() {
    for(auto i = objects.begin(); i != objects.end();) {
        if((*i)->isDestructionRequested()) {
            i = objects.erase(i);
        } else {
            ++i;
        }
    }
}

void Game::DetectCollisions(float dt) {
    for(auto a = objects.begin(); a != objects.end(); ++a) {
        for(auto b = std::next(a); b != objects.end(); ++b) { //Для каждой пары объектов
            GameObject& ra = **a;
            GameObject& rb = **b;
            if(CollisionMask(ra, rb)) { //Требуется ли обработка столкновения
                if(DetectCollision(ra, rb, dt)) { //Базовый алгоритм
                    if(RefineCollision(ra, rb, dt)) { //Более точный
                        ra.OnCollision(rb);
                        rb.OnCollision(ra);
                    }
                }
            }
        }
    }
}


bool Game::CollisionMask(const GameObject& a, const GameObject& b) {
    //Если один из объектов логически уже уничтожен, то он не сталкивается с другими (return false)
    return !(a.isDestructionRequested() || b.isDestructionRequested()) &&
    //Если ни одному из объектов не требуется обработка столкновения с другим, то возвращаем false
    (a.CollisionMask(b.getStaticType()) || b.CollisionMask(a.getStaticType()));
}

//Обнаружение столкновений по радиусу окружности, описывающей модель объекта
//В непрерывном случае радиус расширяется на расстояние, которое объекты могут пройти за время dt
bool Game::DetectCollision(const GameObject& a, const GameObject& b, float dt) {
    if(Constant::continuousCollisions) {
        return (a.getPosition() - b.getPosition()).getLength() <
            a.getRadius() + b.getRadius() + (a.getVelocity().getLength() + b.getVelocity().getLength()) * dt;
    } else {
        return (a.getPosition() - b.getPosition()).getLength() < a.getRadius() + b.getRadius();
    }
}

bool Game::RefineCollision(const GameObject& a, const GameObject& b, float dt) {
    if(Constant::refineCollisions) {
        const Model& am = a.getModel();
        const std::vector<GLfloat>& av = am.getTransformed();
        const std::vector<GLubyte>& ai = am.getIndices();
        const Vec2 aBackVel = a.getVelocity() * -1;
        const Model& bm = b.getModel();
        const std::vector<GLfloat>& bv = bm.getTransformed();
        const std::vector<GLubyte>& bi = bm.getIndices();
        const Vec2 bBackVel = b.getVelocity() * -1;

        //Обработка для каждой пары отрезков, из которых состоит модель объекта
        for(int i = 0; i < ai.size(); i += 2) {
            Vec2 a0(i, av, ai);
            Vec2 at = Vec2(i + 1, av, ai) - a0;
            for(int j = 0; j < bi.size(); j += 2) {
                Vec2 b0(j, bv, bi);
                Vec2 bt = Vec2(j + 1, bv, bi) - b0;
                if(Constant::continuousCollisions ?
                    //Алгоритм считает, что отрезки передаются в момент времени t0,
                    //но наши отрезки уже в t0+dt, поэтому передаем скорости со знаком -
                    MovingSegmentCollision(a0, at, aBackVel, b0, bt, bBackVel, dt) :
                    SegmentCollision(a0, at, b0, bt)) return true;
            }
        }
        return false;
    } else {
        return true;
    }
}

bool Game::SegmentCollision(Vec2 p, Vec2 r, Vec2 q, Vec2 s) {
    //http://stackoverflow.com/a/565282/2502024
    float det = Vec2::CrossProd2D(r, s);
    if(fabs(det) > Constant::smallNumber) {
        Vec2 diff = q - p;
        float f = Vec2::CrossProd2D(diff, s / det);
        float g = Vec2::CrossProd2D(diff, r / det);
        return f >= 0 && f <= 1 && g >= 0 && g <= 1;
    }
    return false;
}

bool Game::MovingSegmentCollision(Vec2 p, Vec2 r, Vec2 vp, Vec2 q, Vec2 s, Vec2 vq, float dt) {
    float det = Vec2::CrossProd2D(r, s);
    if(fabs(det) > Constant::smallNumber) {
        const Vec2 v = vq - vp;
        const Vec2 diff = q - p;
        //Расширение предыдущего алгоритма с учетом:
        //q = q0 + v*t, t in [0, dt]
        //(v.x * s.y - v.y * s.x) * t + ((q.x - p.x) * s.y - (q.y - p.y) * s.x)

        //Точки пересечения f и g из SegmentCollision теперь зависят от t.
        //Отрезки пересекаются в точке t из [0, dt], если найдется такое t,
        //что f и g одновременно лежат в [0, 1]. Т.е. решаем 3 пары неравенств относительно t
        auto getInequation = [=](Vec2 dir)->std::tuple<float, float> {
            //0 <= a*t + cp <= 1
            float cp = Vec2::CrossProd2D(diff, dir / det);
            float a = Vec2::CrossProd2D(v, dir / det);
            float left = -cp, right = 1 - cp;
            if(fabs(a) < Constant::smallNumber) {
                if(cp >= 0 && cp <= 1) {
                    left = 0;
                    right = dt;
                } else {
                    left = dt + 1;
                    right = -1;
                }
            } else {
                left /= a;
                right /= a;
                if(a < 0) std::swap(left, right);
            }
            return std::make_tuple(left, right);
        };

        float ls, rs, lr, rr;
        //ls <= t <= rs
        std::tie(ls, rs) = getInequation(s);
        //lr <= t <= rr
        std::tie(lr, rr) = getInequation(r);

        //одновременно с 0 <= t <= dt
        float mx = std::max(0.f, std::max(ls, lr));
        float mn = std::min(dt, std::min(rs, rr));
        return mx <= mn;
    }
    return false;
}

void Game::RequestRestart(float t) {
    wantRestart = true;
    restartTimer = t;
}

void Game::Pause() {
    isLevelRunning = false;
    Controls::onPause();
}

void Game::Resume() {
    isLevelRunning = true;
    Controls::onResume();
    timer.Tick();
}

void Game::SetPlayerPos(const Ship& player) {
    playerPos = player.getPosition();
}

Vec2 Game::GetPlayerPos() {
    return playerPos;
}

void Game::AddPoints(int pointsToAdd) {
    Score::AddPoints(pointsToAdd);
}

void Game::DecAsteroidCount(const Asteroid& a) {
    asteroidCount--;
    if(asteroidCount == Constant::asteroidUfoCount * 2 && !isUfoPresent) {
        GameObject::Create<UFO>(GetUfoSpawn());
    }
    if(asteroidCount <= Constant::asteroidRespawnCount * 2) {
        SpawnAsteroids(Constant::asteroidTargetCount - Constant::asteroidRespawnCount);
    }
}

//Увеличиваем на 2, т.к. asteroidCount считаем по половинам большого астероида
void Game::IncAsteroidCount(const Asteroid& a) {
    asteroidCount += 2;
}

void Game::ResetLogic() {
    asteroidCount = 0;
    isUfoPresent = false;
}

void Game::SpawnAsteroids(int n) {
    for(int i = 0; i < n; ++i) {
        GameObject::Create<Asteroid>(GetSpawnPosition());
    }
}

//Создаем астероид так, чтобы сразу не убить игрока (с учетом зацикленности игровых координат)
Transform Game::GetSpawnPosition() {
    std::uniform_real_distribution<float> zone(-Constant::asteroidSpawnZone, Constant::asteroidSpawnZone);
    Vec2 pos(Constant::worldRatio + 0.2, 1.2);
    if(fabs(playerPos.x) > Constant::asteroidSpawnZone &&
         fabs(playerPos.y) < Constant::asteroidSpawnZone) {
        pos.y = zone(Random::generator);
    } else if(fabs(playerPos.x) < Constant::asteroidSpawnZone &&
                fabs(playerPos.y) > Constant::asteroidSpawnZone) {
        pos.x = zone(Random::generator);
    } else {
        if(Random::flipCoin()) {
            pos.x = zone(Random::generator);
        } else {
            pos.y = zone(Random::generator);
        }
    }
    return Transform(pos);
}

Transform Game::GetUfoSpawn() {
    std::uniform_real_distribution<float> zone(-Constant::ufoZone, Constant::ufoZone);
    return Transform((Constant::worldRatio + 0.12) * (playerPos.x > 0 ? -1 : 1), zone(Random::generator));
}

void Game::OnUfoCreated(const UFO& u) {
    isUfoPresent = true;
}

void Game::OnUfoDestroyed(const UFO& u) {
    isUfoPresent = false;
}
#pragma once

#include <chrono>
#include <cmath>
#include <random>
#include <memory>

#include <android/log.h>
#include <GLES2/gl2.h>

struct Vec2 {
    float x, y;

    constexpr Vec2(float _x, float _y) : x(_x), y(_y) {};
    constexpr Vec2() : x(0.0f), y(0.0f) {};
    //Вспомогательный конструктор, выделяющий n-ю вершину из вершин v с индексами i
    Vec2(unsigned n, const std::vector<GLfloat>& v, const std::vector<GLubyte>& i)
           : x(v[i[n] * 2]), y(v[i[n] * 2 + 1]) {};
    Vec2(const Vec2&) = default;
    Vec2& operator=(const Vec2&) = default;

    float getSquaredDist(const Vec2& other);

    float getLength() {
        return sqrt(x*x + y*y);
    }

    Vec2 getOrthogonal() {
        return Vec2(y, -x);
    }

    Vec2 getNormalized();

    static float CrossProd2D(Vec2 l, Vec2 r) {
        return l.x * r.y - l.y * r.x;
    };

    static float DotProduct(Vec2 l, Vec2 r) {
        return l.x * r.x + l.y * r.y;
    };

    friend Vec2 operator+(const Vec2 &v1, const Vec2 &v2) {
        return Vec2(v1.x + v2.x, v1.y + v2.y);
    };

    friend Vec2 operator-(const Vec2 &v1, const Vec2 &v2) {
        return Vec2(v1.x - v2.x, v1.y - v2.y);
    };

    friend Vec2 operator*(const Vec2 &v1, float mul) {
        return Vec2(v1.x * mul, v1.y * mul);
    };

    friend Vec2 operator/(const Vec2 &v1, float div) {
        return Vec2(v1.x / div, v1.y / div);
    };
};

//Все игровые "магические числа"
namespace Constant {
    static constexpr bool continuousCollisions = true;
    static constexpr bool refineCollisions = true;

    static constexpr int maxBatchSize = 2048;

    static constexpr int gameObjectLineWidth = 2;
    static constexpr int interfaceLineWidth = 3;

    static constexpr Vec2 smallButtonScale = Vec2(0.6f, 0.6f);
    static constexpr Vec2 largeButtonScale = Vec2(1.5f, 1.5f);
    static constexpr float buttonMargin = 0.3f;
    static constexpr float buttonInterval = 0.38f;
    static constexpr float buttonHitRadiusExtension = 1.1f;
    static constexpr float buttonAlpha = 0.4f;

    static constexpr int scoreDigits = 5;
    static constexpr float scoreMargin = 0.05f;
    static constexpr float scoreDigitWidth = 0.15f;
    static constexpr float scoreDigitsInterval = 0.05f;
    static constexpr Vec2 scoreDim(
        (scoreDigits * (scoreDigitsInterval + scoreDigitWidth) - scoreDigitsInterval) / 2.f,
        scoreDigitWidth * 3.f / 4.f
    );

    static constexpr float worldRatio = 16.0f / 9.0f;
    static constexpr float inverseWorldRatio = 0.5625f;

    static constexpr bool vibrateOnDeath = true;
    static constexpr float restartAfterDeathSec = 1.5f;
    static constexpr float shipRotationSpeed = 3.5f;
    static constexpr float shipThrottle = 2.0f;
    static constexpr float shipFriction = 1.0f;
    static constexpr float shipShootingCooldown = 0.3f;
    static constexpr float shipTeleportCooldown = 0.5f;
    static constexpr float bulletSpeedBasic = 2.0f;
    static constexpr float bulletSpeedInherited = 0.5f;
    static constexpr float bulletLifetime = 1.2f;
    static constexpr float asteroidSpawnZone = 0.7f;
    static constexpr float asteroidSpeed = 0.45f;
    static constexpr float asteroidBulletImpact = 0.25f;
    static constexpr float asteroidBlastImpact = 0.4f;
    static constexpr float asteroidAngleVariance = M_PI / 18.0f;
    static constexpr float asteroidBigRadius = 0.2f;
    static constexpr float asteroidSmallRadius = 0.1f;
    static constexpr float asteroidRadiusDistribution = 0.65f;
    static constexpr int asteroidVertCount = 12;
    static constexpr int explosionVertCount = 12;
    static constexpr int explosionMaxScale = 4;
    static constexpr float explosionLifetime = 0.6f;
    static constexpr float explosionSpikeLen = 0.015f;
    static constexpr float ufoSpeed = 0.4f;
    static constexpr float ufoCooldown = 2.5f;
    static constexpr float ufoMargin = 0.1f;
    static constexpr float ufoZone = 0.7f;
    static constexpr float ufoAccuracy = 0.13f;

    static constexpr int asteroidTargetCount = 5;
    static constexpr int asteroidUfoCount = 2;
    static constexpr int asteroidRespawnCount = 1;

    static constexpr float pointsTimeInterval = 5.f;
    static constexpr int pointsForTime = 5;
    static constexpr int pointsForUFO = 40;
    static constexpr int pointsForSmall = 20;
    static constexpr int pointsForLarge = 10;

    //может быть использовано для масштабирования времени
    static constexpr float microsecondsInSecond = 1000000.0f;
    static constexpr float bigNumber = 10000.0f;
    static constexpr float smallNumber = 0.0001f;
};

class Timer {
    std::chrono::time_point<std::chrono::system_clock> mTime;
    std::chrono::time_point<std::chrono::system_clock> mPrevTime;
    std::chrono::time_point<std::chrono::system_clock> mStartTime;

public:
    Timer();
    //Возвращает время с последнего вызова Tick() в секундах
    float Tick();
    //Возвращает время в секундах с момента создания таймера
    float getTotalTime();
};

//Положение в игровом мире
class Transform : public std::enable_shared_from_this<Transform> {
    static constexpr float worldWidth = 1.99f * Constant::worldRatio;
    static constexpr float worldHeight = 1.99f;
    static constexpr float worldXMax = Constant::worldRatio;
    static constexpr float worldYMax = 1.f;
    float angle, angleSin, angleCos;
    Vec2 pos, scale;
    //Расстояние, на которое можно зайти за границы экрана
    //В большинстве случаев - радиус окружности, описывающей модель
    float margin = 0.3f;

public:
    //Имитируем зацикленность игрового мира
    //Вызывается при каждом изменении позиции
    void ClampPos() {
        if(pos.x < -worldXMax - margin) pos.x += worldWidth + 2 * margin;
        if(pos.x > worldXMax + margin)  pos.x -= worldWidth + 2 * margin;
        if(pos.y < -worldYMax - margin) pos.y += worldHeight + 2 * margin;
        if(pos.y > worldYMax + margin)  pos.y -= worldHeight + 2 * margin;
    };

    void setPos(const Vec2& _pos) {
        pos = _pos;
        ClampPos();
    };

    Vec2 getPos() const {
        return pos;
    };

    //Заранее подсчитываем синус и косинус при каждом изменении угла
    void setAngle(float _angle) {
        angle = _angle;
        angleSin = sin(angle);
        angleCos = cos(angle);
    };

    float getAngle() const {
        return angle;
    };

    float getSin() const {
        return angleSin;
    };

    float getCos() const {
        return angleCos;
    };

    void setScale(const Vec2& _scale) {
        scale = _scale;
    };

    Vec2 getScale() const {
        return scale;
    };

    Vec2 getDirection() const {
        return Vec2(-angleSin, angleCos);
    };

    void setMargin(float _margin) {
        margin = _margin;
    };

    Transform(const Vec2& _pos = Vec2(0.0f, 0.0f),
              float _angle = 0.0f,
              const Vec2& _scale = Vec2(1.0f, 1.0f)) {
        setPos(_pos);
        setAngle(_angle);
        setScale(_scale);
        margin = 0.3f;
    };

    Transform(float _x, float _y, float _angle = 0.0f) :
        Transform(Vec2(_x, _y), _angle, Vec2(1.0f, 1.0f)) {};

    Transform(const Transform&) = default;
    Transform& operator=(const Transform&) = default;
    ~Transform() = default;
};

struct Random {
    //Предоставляем доступ к генератору
    static std::default_random_engine generator;
    //true/false c вероятностью 0.5
    static bool flipCoin();
};
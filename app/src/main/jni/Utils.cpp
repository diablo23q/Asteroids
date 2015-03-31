#include <Utils.h>

///Vec2///

float Vec2::getSquaredDist(const Vec2& other) {
    float dx = x - other.x;
    float dy = y - other.y;
    return dx*dx + dy*dy;
}

Vec2 Vec2::getNormalized() {
    float len = getLength();
    if(len > 0) {
        return Vec2(x / len, y / len);
    } else {
        return Vec2();
    }
}


///Timer///

Timer::Timer()  {
    mTime = std::chrono::system_clock::now();
    mStartTime = std::chrono::system_clock::now();
}

float Timer::Tick() {
    mPrevTime = mTime;
    mTime = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>
        (mTime - mPrevTime).count() / Constant::microsecondsInSecond;
}

float Timer::getTotalTime() {
    return std::chrono::duration_cast<std::chrono::microseconds>
        (std::chrono::system_clock::now() - mStartTime).count() / Constant::microsecondsInSecond;
}


///Random///

bool Random::flipCoin() {
    std::bernoulli_distribution distribution(0.5);
    return distribution(generator);
}

//Инициализируем текущим временем
std::default_random_engine Random::generator(std::chrono::system_clock::now().time_since_epoch().count());
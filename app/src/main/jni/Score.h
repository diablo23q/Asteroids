#pragma once

#include <Renderer.h>

class Score {
    //Модель одной цифры
    class DigitModel : public Model {
        unsigned curValue;
        static constexpr float w = Constant::scoreDigitWidth / 2.0f;
        static constexpr float h = Constant::scoreDigitWidth * 3.0f / 4.0f;
        static const std::array<std::vector<GLubyte>, 10> inds;
    public:
        DigitModel();
        void setDigit(unsigned digit);
        unsigned getDigit();
    };

    //Блок цифр
    class Numbers {
        bool ready = false;
        unsigned curValue;
        //Цифры в массиве расположены от младшего разряда к старшему
        std::array<std::shared_ptr<DigitModel>, Constant::scoreDigits> digits;
    public:
        void Init();
        ~Numbers();
        void set(unsigned num);
        unsigned get();
        void add(unsigned num);
        void ApplyTransform(const Transform& t);
    };

    static void SubmitHighScore();
    static void ReadHighScore();

    static Numbers cur;
    static Numbers high;

public:
    static void Init();
    static void AddPoints(int pointsToAdd);
    static void OnRestart();
    static void Resize();
    static int getScore();
    static int getHighScore();
};
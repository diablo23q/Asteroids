#include <Score.h>
#include <Wrapper.h>

///DigitModel///

const std::array<std::vector<GLubyte>, 10> Score::DigitModel::inds
                      {{ {0, 1, 1, 5, 5, 4, 4, 0},              //0
                         {1, 5},                                //1
                         {0, 1, 1, 3, 3, 2, 2, 4, 4, 5},        //2
                         {0, 1, 1, 5, 5, 4, 3, 2},              //3
                         {0, 2, 2, 3, 1, 5},                    //4
                         {1, 0, 0, 2, 2, 3, 3, 5, 5, 4},        //5
                         {0, 4, 4, 5, 5, 3, 3, 2},              //6
                         {0, 1, 1, 5},                          //7
                         {0, 1, 1, 5, 5, 4, 4, 0, 2, 3},        //8
                         {3, 2, 2, 0, 0, 1, 1, 5}          }};  //9

//Для всех цифр одни и те же вершины, но разные индексы
Score::DigitModel::DigitModel() : Model({-w, h,  w, h,  -w, 0,  w, 0,  -w, -h,  w, -h}, inds[0]) {
    curValue = 0;
}

void Score::DigitModel::setDigit(unsigned digit) {
    curValue = digit % 10;
    indices = inds[curValue];
}

unsigned Score::DigitModel::getDigit() {
    return curValue;
}


///Numbers///

void Score::Numbers::Init() {
    curValue = 0;
    for(auto& i : digits) {
        i = std::make_shared<DigitModel>();
        Renderer::GetHUDBatch().Add(i);
    }
    ApplyTransform(Transform());
    set(0);
    ready = true;
}

Score::Numbers::~Numbers() {
    ready = false;
    for(auto& i : digits) {
        Renderer::GetHUDBatch().Remove(i);
    }
}

void Score::Numbers::set(unsigned num) {
    curValue = num;
    if(ready) {
        for(auto& i : digits) {
            unsigned d = num % 10;
            if(d != i->getDigit()) i->setDigit(d);
            num /= 10;
        }
    }
}

unsigned Score::Numbers::get() {
    return curValue;
}

void Score::Numbers::add(unsigned num) {
    curValue += num;
    set(curValue);
}

//t воспринимается как положение центра цифрового блока
void Score::Numbers::ApplyTransform(const Transform& t) {
    if(ready) {
        Transform tr(t);
        float step = (Constant::scoreDigitsInterval + Constant::scoreDigitWidth) * tr.getScale().x;
        float shift = (Constant::scoreDigits - 1) * step / 2.f;
        tr.setPos(tr.getPos() + Vec2(shift, 0));
        for(auto& i : digits) {
            i->ApplyTransform(tr);
            tr.setPos(tr.getPos() - Vec2(step, 0));
        }
    }
}


///Score///

Score::Numbers Score::cur;
Score::Numbers Score::high;

void Score::SubmitHighScore() {
    JavaCall::SubmitHighScore(high.get());
}

void Score::ReadHighScore() {
    high.set(JavaCall::ReadHighScore());
}

void Score::Init() {
    cur.Init();
    high.Init();
    Resize();
}

void Score::Resize() {
    float wt = 1.0f / Renderer::GetHUDScale().x;
    float ht = 1.0f;
    cur.ApplyTransform(Vec2(-wt + Constant::scoreDim.x + Constant::scoreMargin,
                             ht - Constant::scoreDim.y - Constant::scoreMargin));
    high.ApplyTransform(Vec2(wt - Constant::scoreDim.x - Constant::scoreMargin,
                             ht - Constant::scoreDim.y - Constant::scoreMargin));
}

void Score::AddPoints(int pointsToAdd) {
    cur.add(pointsToAdd);
    if(cur.get() > high.get()) {
        high.set(cur.get());
        SubmitHighScore();
    }
}

void Score::OnRestart() {
    cur.set(0);
    ReadHighScore();
}

int Score::getScore() {
    return cur.get();
}

int Score::getHighScore() {
    return high.get();
}
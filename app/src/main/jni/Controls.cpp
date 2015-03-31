#include <Controls.h>
#include <Game.h>

float Controls::forward = 0.f;
float Controls::horAxis = 0.f;
bool Controls::shooting = false;
bool Controls::teleporting = false;

const int Controls::invalidId = -1;
//Идентифицируем палец по кнопке, на которой началось касание
int Controls::movementId = invalidId;
int Controls::shootingId = invalidId;
int Controls::teleportId = invalidId;
int Controls::pauseId = invalidId;
int Controls::resumeId = invalidId;
int Controls::restartId = invalidId;

std::unique_ptr<Controls::Button> Controls::fwd, Controls::left,
    Controls::right, Controls::shoot, Controls::teleport,
    Controls::pause, Controls::resume, Controls::restart;

void Controls::Init() {
    //Создаём кнопки, по местам кнопки расставляются позже (см. Resize)
    fwd = std::unique_ptr<Button>(new Button(Model::CreateTriangleBtn()));
    left = std::unique_ptr<Button>(new Button(Model::CreateTriangleBtn(), M_PI / 2.0));
    right = std::unique_ptr<Button>(new Button(Model::CreateTriangleBtn(), -M_PI / 2.0));
    shoot = std::unique_ptr<Button>(new Button(Model::CreateShootBtn(), 0.f, Constant::largeButtonScale));
    teleport = std::unique_ptr<Button>(new Button(Model::CreateTeleportBtn()));

    pause = std::unique_ptr<Button>(new Button(Model::CreatePauseBtn(), 0.f, Constant::smallButtonScale));
    resume = std::unique_ptr<Button>(new Button(Model::CreateTriangleBtn(), -M_PI / 2.0, Constant::smallButtonScale));
    resume->Disable();
    restart = std::unique_ptr<Button>(new Button(Model::CreateRestartBtn(), 0.f, Constant::smallButtonScale));
    restart->Disable();
}

void Controls::onPause() {
    pause->Disable();
    resume->Enable();
    restart->Enable();
}

void Controls::onResume() {
    pause->Enable();
    resume->Disable();
    restart->Disable();
}

void Controls::Resize() {
    float w = 1.0f / Renderer::GetHUDScale().x;
    float h = 1.0f;
    float distBB = Constant::buttonInterval * h;
    float dist = Constant::buttonMargin * h;
    right->setPos(Vec2(w - dist, -h + dist));
    fwd->setPos(Vec2(w - dist - distBB, -h + dist));
    left->setPos(Vec2(w - dist - 2*distBB, -h + dist));
    shoot->setPos(Vec2(-w + distBB, -h + distBB));
    teleport->setPos(Vec2(-w + dist, 0));

    pause->setPos(Vec2(0, h - dist/2));
    resume->setPos(Vec2(-dist/2, h - dist/2));
    restart->setPos(Vec2(dist/2, h - dist/2));
}

//Логика обработки нажатий в следующих трех функциях
void Controls::onPointerDown(int id, float x, float y) {
    if(shoot->Inside(x, y)) {
        shootingId = id;
        shooting = true;
        return;
    }
    if(teleport->Inside(x, y)) {
        teleportId = id;
        teleporting = true;
        return;
    }
    if(left->Inside(x, y)) {
        if(movementId != invalidId) {
            forward = 0.f;
            horAxis = 0.f;
        }
        movementId = id;
        horAxis = 1.f;
        return;
    }
    if(fwd->Inside(x, y)) {
        if(movementId != invalidId) {
            forward = 0.f;
            horAxis = 0.f;
        }
        movementId = id;
        forward = 1.f;
        return;
    }
    if(right->Inside(x, y)) {
        if(movementId != invalidId) {
            forward = 0.f;
            horAxis = 0.f;
        }
        movementId = id;
        horAxis = -1.f;
        return;
    }

    if(pause->Inside(x, y)) {
        pauseId = id;
        return;
    }
    if(resume->Inside(x, y)) {
        resumeId = id;
        return;
    }
    if(restart->Inside(x, y)) {
        restartId = id;
        return;
    }
}

void Controls::onPointerUp(int id, float x, float y) {
    if(id == movementId) {
        forward = 0.f;
        horAxis = 0.f;
        movementId = invalidId;
        return;
    }
    if(id == shootingId) {
        shooting = false;
        shootingId = invalidId;
        return;
    }
    if(id == teleportId) {
        teleporting = false;
        teleportId = invalidId;
        return;
    }
    if((id == pauseId) && pause->Inside(x, y)) {
        pauseId = invalidId;
        Game::Get().Pause();
        return;
    }
    if((id == resumeId) && resume->Inside(x, y)) {
        resumeId = invalidId;
        Game::Get().Resume();
        return;
    }
    if((id == restartId) && restart->Inside(x, y)) {
        restartId = invalidId;
        Game::Get().RequestRestart();
        Game::Get().Resume();
        return;
    }
}

void Controls::onPointerMove(int id, float x, float y) {
    if(id == movementId && forward > 0.f) {
        float pos = fwd->getPos().x;
        float rad = fwd->getRadius();
        if(x >= pos + rad) {
            horAxis = -1.f;
        } else if(x <= pos - rad) {
            horAxis = 1.f;
        } else {
            horAxis = 0.f;
        }
    }
}

//Возвращаем посчитанные ранее значения
float Controls::Forward() {
    return forward;
}

float Controls::HorAxis() {
    return horAxis;
}

bool Controls::Shooting() {
    return shooting;
}

bool Controls::Teleport() {
    return teleporting;
}
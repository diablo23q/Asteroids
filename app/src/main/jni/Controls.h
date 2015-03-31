#pragma once

#include <Utils.h>
#include <Renderer.h>

class Controls {
    class Button {
        std::shared_ptr<Model> model;
        Transform t;
        bool enabled;
    public:
        Button(std::shared_ptr<Model> _model,
               float angle = 0.0f,
               const Vec2& scale = Vec2(1.0f, 1.0f),
               const Vec2& pos = Vec2()) : model(_model), t(pos, angle, scale) {
            model->ApplyTransform(t);
            Renderer::GetControlsBatch().Add(model);
            Enable();
        };

        ~Button() {
            Renderer::GetControlsBatch().Remove(model);
        };

        void Enable() {
            enabled = true;
            model->setDraw(true);
        };

        void Disable() {
            enabled = false;
            model->setDraw(false);
        };

        //По координатам касания определяем, находится ли палец на кнопке
        bool Inside(float x, float y) {
            if(enabled) {
                float scale = std::max(t.getScale().x, t.getScale().y) * Constant::buttonHitRadiusExtension;
                return t.getPos().getSquaredDist(Vec2(x, y)) < model->getSquaredRadius() * scale * scale;
            } else {
                return false;
            }
        };

        void setPos(const Vec2& pos) {
            t.setPos(pos);
            model->ApplyTransform(t);
        };

        Vec2 getPos() {
            return t.getPos();
        };

        float getRadius() {
            return model->getRadius();
        };

    };

    static float forward;
    static float horAxis;
    static bool shooting;
    static bool teleporting;
    static const int invalidId;
    static int movementId, teleportId, shootingId, pauseId, resumeId, restartId;

    static std::unique_ptr<Button> fwd, left, right, shoot, teleport, pause, resume, restart;

public:
    static void Init();
    static void Resize();

    static void onPause();
    static void onResume();

    static void onPointerDown(int id, float x, float y);
    static void onPointerUp(int id, float x, float y);
    static void onPointerMove(int id, float x, float y);

    static float Forward();
    static float HorAxis();
    static bool Shooting();
    static bool Teleport();
};
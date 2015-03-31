#pragma once

#include <functional>
#include <vector>
#include <list>

#include <Utils.h>

//Модель содержит вершины и индексы для отрисовки в OpenGL
class Model : public std::enable_shared_from_this<Model> {
protected:
    // verts {x1, y1, x2, y2, ...}
    std::vector<GLfloat> verts; //Исходные вершины
    //tverts.size() === verts.size()
    std::vector<GLfloat> tverts; //Преобразованные вершины
    std::vector<GLubyte> indices; //Индексы

    bool draw = true;
    float radius = 0.0f;
    float sqrRadius = 0.0f;
    //Подсчет радиуса описывающей окружности с центром в (0,0)
    float CalcRadius();

public:
    //Для удобства создания индексов
    static std::vector<GLubyte> DefaultIndices(int sz, bool triangles);

    //Внимание! Количество вершин в два раза меньше verts.size()
    Model(const std::vector<GLfloat>& _verts, const std::vector<GLubyte>& _indices);
    Model(const std::vector<GLfloat>& _verts);

    //Преобразование вершин происходит на CPU, в этой функции
    void ApplyTransform(const Transform& t);

    const std::vector<GLfloat>& getVerts() const {return verts;};
    const std::vector<GLfloat>& getTransformed() const {return tverts;};
    const std::vector<GLubyte>& getIndices() const {return indices;};

    float getRadius() const {return radius;};
    float getSquaredRadius() const {return sqrRadius;};
    void setDraw(bool _draw) {draw = _draw;};
    bool getDraw() const {return draw;};

    //Копирование и присваивание запрещено
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;
    virtual ~Model() {};

    //Модели по умолчанию для игровых объектов
    static std::shared_ptr<Model> CreateShip();
    static std::shared_ptr<Model> CreateShipEngine();
    static std::shared_ptr<Model> CreateAsteroid();
    static std::shared_ptr<Model> CreateBullet();
    static std::shared_ptr<Model> CreateUFO();
    static std::shared_ptr<Model> CreateExplosion();

    //Модели кнопок
    static std::shared_ptr<Model> CreateTriangleBtn();
    static std::shared_ptr<Model> CreateShootBtn();
    static std::shared_ptr<Model> CreateTeleportBtn();
    static std::shared_ptr<Model> CreatePauseBtn();
    static std::shared_ptr<Model> CreateRestartBtn();
};

//Один Batch = один draw call
class Batch : public std::enable_shared_from_this<Batch> {
    std::vector<GLfloat> mVerts;
    std::vector<GLubyte> mIndices;
    std::function<void()> mSetup;
    std::vector< std::weak_ptr<Model> > mObjects;

    GLenum mMode;
    bool mHudBatch;
    Vec2 renderScale;

    friend class Renderer;
    //Определяет масштаб, который будет применяться к объектам этого Batch при отрисовке
    //Всё, что относится к интерфейсу (isHud) живет в рамках всего экрана
    //Игровой мир (!isHud) поддерживается в соотношении сторон Constant::worldRatio
    Batch(bool isHudBatch);
    //Renderer инициирует отрисовку
    void Draw();

    Batch(const Batch&) = delete;
    Batch& operator=(const Batch&) = delete;

public:
    //Функция, которая будет вызываться перед отрисовкой
    void setSetupFunction(std::function<void()> setup) {mSetup = setup;};
    //Режим отрисовки в смысле OpenGL (GL_LINES, GL_TRIANGLES)
    void setDrawMode(GLenum mode) {mMode = mode;};

    //Чтобы отрисовать модель, ее нужно добавить в Batch
    void Add(std::weak_ptr<Model> model);
    void Remove(std::weak_ptr<Model> model);
};

//Управляет отрисовкой, контролирует батчи
class Renderer {
    static GLint uniLoc;
    static Vec2 goScale;
    static Vec2 hudScale;
    static std::unique_ptr<Batch> goBatch;
    static std::unique_ptr<Batch> hudBatch;
    static std::unique_ptr<Batch> controlsBatch;

    static const std::string vShaderStr, fShaderStr;
    static GLuint LoadShader(GLenum type, const std::string& shaderSrc);

public:
    //Создает батчи
    static void InitInternals();
    //Инициализирует OpenGL, может вызываться неоднократно
    static void InitGLContext();
    static void Draw();
    //Один к-т для всех полупрозрачных объектов
    static void SetAlpha(float a);
    static void OnResolutionChange(int w, int h);

    static Batch& GetGameObjectBatch() { return *goBatch; }; //Игровые объекты
    static Batch& GetControlsBatch() { return *controlsBatch; }; //Кнопки
    static Batch& GetHUDBatch() { return *hudBatch; }; //Счёт

    //см. конструктор Batch(bool)
    static Vec2 GetGameObjectScale() {return goScale;};
    static Vec2 GetHUDScale() {return hudScale;};

};
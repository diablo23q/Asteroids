#include <Renderer.h>

#include <cassert>

///Model///

Model::Model(const std::vector<GLfloat>& _verts,
             const std::vector<GLubyte>& _indices) : verts(_verts), tverts(_verts), indices(_indices) {
    if(verts.size() % 2) {
        verts.pop_back();
        tverts.pop_back();
    }
    CalcRadius();
};

//С++11 позволяет вызывать один конструктор из другого
Model::Model(const std::vector<GLfloat>& _verts) : Model(_verts, std::move(DefaultIndices(_verts.size(), false))) {};

float Model::CalcRadius() {
    float maxSqDist = 0.0f;
    for(int i = 0; i < verts.size(); i += 2) {
        float sqDist = verts[i] * verts[i] + verts[i + 1] * verts[i + 1];
        if(sqDist > maxSqDist) maxSqDist = sqDist;
    }
    sqrRadius = maxSqDist;
    radius = sqrt(maxSqDist);
}

std::vector<GLubyte> Model::DefaultIndices(int sz, bool triangles) {
    std::vector<GLubyte> defInds(sz);
    if(triangles) {
        //Индексы для режима GL_TRIANGLES
        //Каждые 3 вершины формируют треугольник
        for(int i = 0; i < sz; ++i) {
            defInds[i] = i;
        }
    } else {
        //Индексы для режима GL_LINES
        //Каждая вершина добавляет линию (как в GL_LINE_LOOP)
        defInds.front() = 0;
        defInds.back() = 0;
        if(sz > 2) {
            for(int i = 1; i < sz - 1; ++i) {
                defInds[i] = (i + 1) / 2;
            }
        }
    }
    return defInds;
}

void Model::ApplyTransform(const Transform& t) {
    Vec2 pos = t.getPos();
    Vec2 scale = t.getScale();
    float a = t.getCos() * scale.x,
          b = t.getSin() * scale.x,
          c = t.getSin() * scale.y,
          d = t.getCos() * scale.y;

    for(int i = 0; i < verts.size(); i += 2) {
        tverts[i] = verts[i] * a - verts[i + 1] * b + pos.x;
        tverts[i + 1] = verts[i] * c + verts[i + 1] * d + pos.y;
    }
}


///Batch///

Batch::Batch(bool isHudBatch)
    : mMode(GL_LINES), mSetup([](){}), mHudBatch(isHudBatch),
      mVerts(Constant::maxBatchSize), mIndices(Constant::maxBatchSize) {};

void Batch::Draw() {
    mSetup();
    renderScale = mHudBatch ? Renderer::GetHUDScale() : Renderer::GetGameObjectScale();
    Renderer::SetAlpha(mHudBatch ? Constant::buttonAlpha : 1.0f);
    int totalVertSize = 0;
    int totalIndSize = 0;
    for(auto i = mObjects.begin(); i != mObjects.end();) {
        if(i->expired()) {
            //убираем "висячие" weak_ptr
            i = mObjects.erase(i);
        } else {
            const Model& m = *i->lock().get();
            ++i;

            if(m.getDraw()) {
                const std::vector<GLfloat>& verts = m.getTransformed();
                const std::vector<GLubyte>& indices = m.getIndices();

                for(int i = 0; i < verts.size(); i += 2) {
                    mVerts[totalVertSize + i] = verts[i] * renderScale.x;
                    mVerts[totalVertSize + i + 1] = verts[i + 1] * renderScale.y;
                }
                for(int i = 0; i < indices.size(); i += 1) {
                    mIndices[totalIndSize + i] = indices[i] + totalVertSize/2;
                }
                totalVertSize += verts.size();
                totalIndSize += indices.size();

                //Емкость Batch не изменяется
                assert(totalVertSize <= Constant::maxBatchSize);
                assert(totalIndSize <= Constant::maxBatchSize);
            }
        }
    }
    //Обновляем index и vertex buffer каждый кадр
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalIndSize * sizeof(GLubyte), mIndices.data(), GL_STREAM_DRAW);
    glBufferData(GL_ARRAY_BUFFER, totalVertSize * sizeof(GLfloat), mVerts.data(), GL_STREAM_DRAW);
    glDrawElements(mMode, totalIndSize, GL_UNSIGNED_BYTE, (void*)0);
    //buffer orphaning
    /*glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalIndSize * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
    glBufferData(GL_ARRAY_BUFFER, totalVertSize * sizeof(GLfloat), NULL, GL_STREAM_DRAW);*/
}

void Batch::Add(std::weak_ptr<Model> model) {
    mObjects.push_back(std::move(model));
}

void Batch::Remove(std::weak_ptr<Model> model) {
    for(auto i = mObjects.begin(); i != mObjects.end(); ++i) {
        if(i->lock() == model.lock()) {
            mObjects.erase(i);
            break;
        }
    }
}


///Игровые модели///

std::shared_ptr<Model> Model::CreateTriangleBtn() {
    return std::shared_ptr<Model> (new Model({  0.0f, 0.13f,
                                               0.13f, -0.13f,
                                              -0.13f, -0.13f    },
                                              Model::DefaultIndices(3, true)));
}

std::shared_ptr<Model> Model::CreateShootBtn() {
    return std::shared_ptr<Model> (new Model({-0.13f, 0.03f,
                                              -0.03f, -0.0f,
                                              -0.13f, -0.03f,

                                               0.13f, 0.03f,
                                               0.03f, 0.0f,
                                               0.13f, -0.03f,

                                               0.03f, -0.13f,
                                                0.0f, -0.03f,
                                              -0.03f, -0.13f,

                                               0.03f, 0.13f,
                                                0.0f, 0.03f,
                                              -0.03f, 0.13f     },
                                              Model::DefaultIndices(12, true)));
}

std::shared_ptr<Model> Model::CreateTeleportBtn() {
    return std::shared_ptr<Model> (new Model({  0.0f, 0.0f,
                                              -0.09f, -0.0f,
                                              -0.04f, 0.13f,
                                               0.04f, 0.13f,
                                               0.09f, 0.0f,
                                               0.04f, -0.13f,
                                              -0.04f, -0.13f},
                                              {0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6, 1}));
}

std::shared_ptr<Model> Model::CreatePauseBtn() {
    return std::shared_ptr<Model> (new Model({ -0.12f, -0.13f,
                                               -0.04f, -0.13f,
                                               -0.04f, 0.13f,
                                               -0.12f, 0.13f,
                                                0.04f, -0.13f,
                                                0.12f, -0.13f,
                                                0.12f, 0.13f,
                                                0.04f, 0.13f },
                                               {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4} ));
}

std::shared_ptr<Model> Model::CreateRestartBtn() {
    return std::shared_ptr<Model> (new Model({ -0.13f, 0.05f,
                                               -0.13f, 0.13f,
                                                0.13f, 0.13f,
                                                0.13f, 0.05f,
                                                0.05f, 0.05f,
                                                0.05f, -0.05f,
                                                0.13f, -0.05f,
                                                0.13f, -0.13f,
                                               -0.05f, -0.13f,
                                               -0.05f, -0.05f,
                                               -0.05f, -0.02f,
                                               -0.13f, -0.09f,
                                               -0.05f, -0.16f},
                                               {0, 1, 2, 2, 3, 0, 3, 4, 5, 5, 6, 3, 6, 7, 8, 8, 9, 6, 10, 11, 12} ));
}

std::shared_ptr<Model> Model::CreateShip() {
    return std::shared_ptr<Model> (new Model({   0.0f, 0.065f,
                                                0.05f, -0.065f,
                                               0.017f, -0.03f,
                                              -0.017f, -0.03f,
                                               -0.05f, -0.065f  }));
}

std::shared_ptr<Model> Model::CreateShipEngine() {
    return std::shared_ptr<Model> (new Model({   0.0f, -0.05f,
                                               0.017f, -0.03f,
                                              -0.017f, -0.03f},
                                             {0, 1, 0, 2}));
}

std::shared_ptr<Model> Model::CreateAsteroid() {
    const int vertCount = Constant::asteroidVertCount;
    std::bernoulli_distribution coin(Constant::asteroidRadiusDistribution);
    std::uniform_real_distribution<float> shift(-Constant::asteroidAngleVariance, Constant::asteroidAngleVariance);

    //Генерация большого астероида
    //Проходим по кругу, выбираем угол с некоторым смещением,
    //размещаем вершину либо на большом, либо на малом радиусе
    std::vector<float> verts(vertCount * 2);
    for(int i = 0; i < vertCount; ++i) {
        float angle = 2.0 * M_PI * i / vertCount + shift(Random::generator);
        float radius = coin(Random::generator) ? Constant::asteroidBigRadius : Constant::asteroidSmallRadius;
        verts[i*2] = radius * cos(angle);
        verts[i*2 + 1] = radius * sin(angle);
    }

    return std::make_shared<Model>(verts);
}

std::shared_ptr<Model> Model::CreateBullet() {
    return std::shared_ptr<Model> (new Model({ 0.0f, -0.02f,
                                               0.0f, 0.02f  },
                                               {0, 1}));
}

std::shared_ptr<Model> Model::CreateUFO() {
    return std::shared_ptr<Model> (new Model({-0.125f, 0.0f,
                                               -0.05f, 0.05f,
                                              -0.025f, 0.1f,
                                               0.025f, 0.1f,
                                                0.05f, 0.05f,
                                               0.125f, 0.0f,
                                                0.05f, -0.05f,
                                               -0.05f, -0.05f },
                                               {0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 0, 0, 5, 1, 4} ));
}

std::shared_ptr<Model> Model::CreateExplosion() {
    const int vertCount = Constant::explosionVertCount;
    std::uniform_real_distribution<float> disp(Constant::explosionSpikeLen / 3, Constant::explosionSpikeLen);
    std::uniform_real_distribution<float> shift(-Constant::asteroidAngleVariance, Constant::asteroidAngleVariance);

    //Аналогично астероидам (см. CreateAsteroid)
    std::vector<float> verts(vertCount * 4);
    for(int i = 0; i < vertCount; ++i) {
        float angle = 2.0 * M_PI * i / vertCount + shift(Random::generator);
        float dist = disp(Random::generator);
        verts[i*4] = dist * cos(angle);
        verts[i*4 + 1] = dist * sin(angle);
        verts[i*4 + 2] = (dist + Constant::explosionSpikeLen) * cos(angle);
        verts[i*4 + 3] = (dist + Constant::explosionSpikeLen) * sin(angle);
    }

    return std::make_shared<Model>(verts);
}


///Renderer///

GLint Renderer::uniLoc;
Vec2 Renderer::goScale;
Vec2 Renderer::hudScale;
std::unique_ptr<Batch> Renderer::goBatch;
std::unique_ptr<Batch> Renderer::hudBatch;
std::unique_ptr<Batch> Renderer::controlsBatch;

//Максимально простые шейдеры
const std::string Renderer::vShaderStr {"attribute vec4 vPos;       \n"
                                        "void main()                \n"
                                        "{                          \n"
                                        " gl_Position = vPos;       \n"
                                        "}                          \n"};

const std::string Renderer::fShaderStr {"precision lowp float;                       \n"
                                        "uniform float fAlpha;                       \n"
                                        "void main()                                 \n"
                                        "{                                           \n"
                                        " gl_FragColor = vec4(1.0, 1.0, 1.0, fAlpha);\n"
                                        "}                                           \n"};

void Renderer::InitGLContext() {
    //Компилируем шейдерную программу, запоминаем расположение переменных
    GLuint programObject = glCreateProgram();
    glAttachShader(programObject, Renderer::LoadShader(GL_VERTEX_SHADER, vShaderStr));
    glAttachShader(programObject, Renderer::LoadShader(GL_FRAGMENT_SHADER, fShaderStr));
    glLinkProgram(programObject);
    glUseProgram(programObject);

    GLint attrLoc = glGetAttribLocation(programObject, "vPos");
    glEnableVertexAttribArray(attrLoc);
    uniLoc = glGetUniformLocation(programObject, "fAlpha");

    //Для отрисовки используем буферы и индексацию
    GLuint vb, ib;

    glGenBuffers(1, &vb);
    glBindBuffer(GL_ARRAY_BUFFER, vb);
    glVertexAttribPointer(attrLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &ib);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);

    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glDisable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

void Renderer::SetAlpha(float a) {
    glUniform1f(uniLoc, a);
}

void Renderer::InitInternals() {
    goBatch = std::unique_ptr<Batch> (new Batch(false));
    goBatch->setSetupFunction([](){ glLineWidth(Constant::gameObjectLineWidth); });

    hudBatch = std::unique_ptr<Batch> (new Batch(true));
    hudBatch->setSetupFunction([](){ glLineWidth(Constant::interfaceLineWidth); });

    controlsBatch = std::unique_ptr<Batch> (new Batch(true));
    controlsBatch->setDrawMode(GL_TRIANGLES);
}

void Renderer::Draw() {
    glClear(GL_COLOR_BUFFER_BIT);
    goBatch->Draw();
    hudBatch->Draw();
    controlsBatch->Draw();
}

void Renderer::OnResolutionChange(int width, int height) {
    float realRatio = (float) width / height;
    float inverseRatio = 1.0f / realRatio;

    //Подсчитываем scale для игровых объектов и интерфейса
    //Подробнее см. конструктор Batch(bool)
    goScale.x = realRatio < Constant::worldRatio ? Constant::inverseWorldRatio : inverseRatio;
    goScale.y = realRatio < Constant::worldRatio ? Constant::inverseWorldRatio * realRatio : 1.0f;

    hudScale.x = inverseRatio;
    hudScale.y = 1.0f;

    glViewport(0, 0, width, height);
}

GLuint Renderer::LoadShader(GLenum type, const std::string& shaderSrc) {
	GLuint shader = glCreateShader(type);
	const char *p = shaderSrc.c_str();
	glShaderSource(shader, 1, &p, NULL);
	glCompileShader(shader);
	return shader;
}
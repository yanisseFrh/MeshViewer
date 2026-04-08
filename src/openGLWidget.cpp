#include "openGLWidget.h"
#include "shaders.h"

OpenGLWidget::OpenGLWidget(QWidget *parent) : QOpenGLWidget(parent), VAO(0), VBO(0), EBO(0), shaderLight(nullptr), shaderTexture(nullptr), shaderCurrent(nullptr), texture(nullptr), leftPressed(false), middlePressed(false), wireframe(false), useTexCoords(false) {

}

OpenGLWidget::~OpenGLWidget() {
    makeCurrent();
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    texture->destroy();
    shaderCurrent = nullptr;
    doneCurrent();
}

int OpenGLWidget::loadMesh(const char *link) {
    int ok = mesh.loadFile(link);
    if (ok > 0) {
        mesh.clear();
    }

    float radius = mesh.getBoundingRadius();
    if (radius > 100.0f) {
        mesh.normalize();
        radius = 30.0f;
    }
    QVector3D center = mesh.getCenter();

    camera.initialize(center, radius);

    updateMeshBuffers();
    return ok;
}

int OpenGLWidget::saveMesh(const char *link) {
    int ok = mesh.saveFile(link);
    return ok;
}

void OpenGLWidget::loadTexture(const QString& textureFilePath) {
    QImage image(textureFilePath);
    if (image.isNull()) {
        qWarning() << "Failed to load texture image";
        return;
    }
    image = image.convertToFormat(QImage::Format_RGBA8888);

    useTexCoords = true;
    texture = std::make_unique<QOpenGLTexture>(image.mirrored(false, true));
    texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::Repeat);

    emit textureChanged(image);
    updateMeshBuffers();

}



void OpenGLWidget::deleteTexture() {
    if (!texture)
        return;

    texture->bind(0);

    emit textureChanged(QImage());
    useTexCoords = false;
    doneCurrent();
    updateMeshBuffers();

}


void OpenGLWidget::updateMeshBuffers() {
    makeCurrent();
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);

    auto vertices = mesh.getVertices();
    auto indices = mesh.getIndices();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    if (useTexCoords && mesh.hasTexture()) {
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
        glEnableVertexAttribArray(2);

        shaderCurrent = shaderTexture.get();
    } else {
        shaderCurrent = shaderLight.get();
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    doneCurrent();

    emit verticesChanged(vertices.size());
    emit trianglesChanged(indices.size() / 3);

    update();
}

void OpenGLWidget::setWireframe(bool enabled) {
    wireframe = enabled;
    update();
}

void OpenGLWidget::initializeGL() {
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);


    shaderLight = std::make_unique<QOpenGLShaderProgram>(this);
    shaderLight->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShader);
    shaderLight->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader);
    shaderLight->link();

    shaderTexture = std::make_unique<QOpenGLShaderProgram>(this);
    shaderTexture->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexTexShader);
    shaderTexture->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentTexShader);
    shaderTexture->link();

    shaderCurrent = shaderLight.get();

    emit verticesChanged(0);
    emit trianglesChanged(0);
}

void OpenGLWidget::resizeGL(int w, int h) {
    glViewport(0.f, 0.f, w, h);

    camera.setAspect(w, h);
}

void OpenGLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    auto indices = mesh.getIndices();

    QMatrix4x4 model, view, projection;
    model.setToIdentity();
    view = camera.getView();
    projection = camera.getProjection();


    shaderCurrent->bind();
    shaderCurrent->setUniformValue("model", model);
    shaderCurrent->setUniformValue("view", view);
    shaderCurrent->setUniformValue("projection", projection);

    if (useTexCoords) {
        texture->bind(0);
        shaderCurrent->setUniformValue("textureSampler", 0);
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    shaderCurrent->release();
}

void OpenGLWidget::mousePressEvent(QMouseEvent *event) {
    lastMousePos = event->position();
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event) {
    QPointF delta = event->position() - lastMousePos;
    lastMousePos = event->position();

    if (event->buttons() & Qt::LeftButton) {
        camera.orbit(delta.x(), delta.y());
        update();
    }

    if (event->buttons() & Qt::RightButton) {
        camera.pan(delta.x(), delta.y());
        update();
    }
}

void OpenGLWidget::wheelEvent(QWheelEvent *event) {
    camera.zoom(event->angleDelta().y() / 120.0f);
    update();
}

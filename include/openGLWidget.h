#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMouseEvent>
#include <QWheelEvent>
#include <memory>

#include "camera.h"
#include "mesh.h"

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT

signals:
    void verticesChanged(int value);
    void trianglesChanged(int value);
    void textureChanged(QImage currentTexture);


public:
    explicit OpenGLWidget(QWidget *parent = nullptr);
    ~OpenGLWidget();

    int loadMesh(const char* link);
    void loadTexture(const QString& textureFilePath);
    int saveMesh(const char* link);
    void updateMeshBuffers();
    void deleteTexture();

public slots:
    void setWireframe(bool enabled);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;


    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    std::unique_ptr<QOpenGLShaderProgram> shaderLight;
    std::unique_ptr<QOpenGLShaderProgram> shaderTexture;
    QOpenGLShaderProgram *shaderCurrent;
    std::unique_ptr<QOpenGLTexture> texture;

    Camera camera;
    QPointF lastMousePos;
    bool leftPressed;
    bool middlePressed;

    Mesh mesh;
    bool wireframe;
    bool useTexCoords;

};

#endif // OPENGLWIDGET_H

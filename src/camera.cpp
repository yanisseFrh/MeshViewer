#include "camera.h"

Camera::Camera(QVector3D startPosition, QVector3D startUp, float startYaw, float startPitch)
    : position(startPosition), worldUp(startUp), yaw(startYaw), pitch(startPitch), fov(45.0f), aspect(1.0f), angleX(0.0f), angleY(0.0f), distance(5.0f) {
    updateCameraVectors();
}

QVector3D Camera::getPosition() const{
    return position;
}

QVector3D Camera::getUp() const{
    return up;
}

QVector3D Camera::getRight() const{
    return right;
}

QVector3D Camera::getTarget() const{
    return target;
}

QMatrix4x4 Camera::getView() {
    QMatrix4x4 view;
    view.lookAt(position, target, up);
    return view;
}

QMatrix4x4 Camera::getProjection() const{
    QMatrix4x4 proj;

    proj.perspective(fov, aspect, 1.0f, 100.0f);
    return proj;
}

float Camera::getAngleX() const {
    return angleX;
}

float Camera::getAngleY() const {
    return angleY;
}

void Camera::setPosition(QVector3D pos) {
    position = pos;
    updateCameraVectors();
}

void Camera::setTarget(QVector3D target) {
    target = target;
    updateCameraVectors();
}

void Camera::setAspect(float width, float height) {
    aspect = width / height;
}

void Camera::setAspect(float aspect) {
    aspect = aspect;
}

void Camera::setAngleX(float x) {
    angleX = x;
}

void Camera::setAngleY(float y) {
    angleY = y;
}

void Camera::initialize(QVector3D center, float radius) {
    setTarget(center);
    distance = radius * 2.0f + 1.0f;

    yaw = -90.0f;
    pitch = 0.0f;

    updateCameraVectors();

    position = target - front * distance;
}

void Camera::orbit(float xoffset, float yoffset, bool constrainPitch) {
    float sensitivity = 0.5f;
    angleX += xoffset * sensitivity;
    angleY -= yoffset * sensitivity;

    if (constrainPitch) {
        if (angleY > 89.0f) angleY = 89.0f;
        if (angleY < -89.0f) angleY = -89.0f;
    }

    float radX = qDegreesToRadians(angleX);
    float radY = qDegreesToRadians(angleY);

    QVector3D cameraPos(
        distance * cos(radY) * sin(radX),
        distance * sin(radY),
        distance * cos(radY) * cos(radX)
        );

    position = cameraPos;
    updateCameraVectors();
}

void Camera::pan(float xoffset, float yoffset) {
    float sensitivity = 0.01f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    QVector3D panOffset = -xoffset * right + yoffset * up;

    position += panOffset;
    target   += panOffset;

    updateCameraVectors();
}


void Camera::zoom(float yoffset) {
    fov -= yoffset;
    if(fov < 1.0f) fov = 1.0f;
    if(fov > 90.0f) fov = 90.0f;
}

void Camera::updateCameraVectors() {
    QVector3D f;
    f.setX(cos(qDegreesToRadians(yaw)) * cos(qDegreesToRadians(pitch)));
    f.setY(sin(qDegreesToRadians(pitch)));
    f.setZ(sin(qDegreesToRadians(yaw)) * cos(qDegreesToRadians(pitch)));

    front = f.normalized();
    right = QVector3D::crossProduct(front, worldUp).normalized();
    up = QVector3D::crossProduct(right, front).normalized();
}

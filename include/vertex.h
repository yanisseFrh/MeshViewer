#ifndef VERTEX_H
#define VERTEX_H

#include <QVector3D>
#include <QVector2D>

#include <concepts>

template<typename T>
concept arithmetic = std::floating_point<T>;

template<arithmetic T>
struct Vertex
{
    Vertex(const Vertex<T> &v);
    Vertex(const T &x, const T &y, const T &z);
    Vertex(const QVector3D &pos);

    QVector3D position;
    QVector3D normal;
    QVector2D texCoords;
};

template<arithmetic T>
Vertex<T>::Vertex(const Vertex<T> &v) : position(v.position), normal(v.normal), texCoords(v.texCoords) {

}

template<arithmetic T>
Vertex<T>::Vertex(const T &x, const T &y, const T &z) : position(QVector3D(x, y, z)) {
    normal = QVector3D(0.0f, 0.0f, 0.0f);
    texCoords = QVector2D(0.0f, 0.0f);
}

template<arithmetic T>
Vertex<T>::Vertex(const QVector3D &pos) : position(pos) {
    normal = QVector3D(0.0f, 0.0f, 0.0f);
    texCoords = QVector2D(0.0f, 0.0f);
}


#endif // VERTEX_H

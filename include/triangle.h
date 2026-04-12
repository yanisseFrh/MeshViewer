#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <vector>
#include <array>

// ---------------------------------------------------------------------------------
// Concepts
// ---------------------------------------------------------------------------------

template<typename T>
concept HasVertices = requires(T t) {
    { t.idVertices.size() } -> std::same_as<size_t>;
    requires t.idVertices.size() == 3;
    { t.idVertices[0] } -> std::convertible_to<unsigned int>;
};

template<typename T>
concept HasTriangleOps = requires(T t, unsigned int i, T other) {
    { t.localIndex(i) } -> std::convertible_to<int>;
    { t.findCommonEdge(other) } -> std::same_as<std::pair<int,int>>;
};

template<typename T>
concept TriangleLike = HasVertices<T> && HasTriangleOps<T>;

// ---------------------------------------------------------------------------------
struct Triangle
{

    Triangle();
    Triangle(const unsigned int &, const unsigned int &, const unsigned int &);

    int localIndex(unsigned int indice) const;
    std::pair<int, int> findCommonEdge(const Triangle &t) const;

    std::array<unsigned int, 3> idVertices;
    std::vector<unsigned int> idFaces;
};

#endif // TRIANGLE_H

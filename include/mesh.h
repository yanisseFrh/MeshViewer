#ifndef MESH_H
#define MESH_H

#include <vector>

#include "vertex.h"
#include "triangle.h"

typedef Vertex<float> Node;

/**
 * @brief The MeshError enum who returns error int type.
 */
enum MeshError {
    OK=0,
    FORMAT=1,
    READ=2,
    SAVE=3,
    UNKNOWN
};

/**
 * @brief The Mesh class, used for mesh loading and processing.
 */
class Mesh
{
public:
    Mesh();

    const std::vector<Node> &getVertices() const;
    const std::vector<unsigned int> getIndices() const;
    const bool &hasTexture() const;

    /**
     * @brief Clear vertices and triangles vectors.
     */
    void clear();

    /**
     * @brief Connect all triangles with its neighbors.
     */
    void sew();

    /**
     * @brief Loading function who handle the file type
     * @param link
     * @return MeshError::OK if the function terminates correctly, other else.
     */
    int loadFile(const char* link);

    /**
     * @brief Loading .off file function.
     * @param link
     * @return MeshError::OK if the function terminates correctly, other else.
     */
    int loadOFF(const char* link);

    /**
     * @brief Loading .obj file function.
     * @param link
     * @return MeshError::OK if the function terminates correctly, other else.
     */
    int loadOBJ(const char* link);

    /**
     * @brief Loading .txt file function.
     * @param link
     * @return MeshError::OK if the function terminates correctly, other else.
     */
    int loadTXT(const char* link);

    /**
     * @brief Loading function who handle the file type
     * @param link
     * @return MeshError::OK if the function terminates correctly, other else.
     */
    int saveFile(const char* link);

    /**
     * @brief Loading .off file function.
     * @param link
     * @return MeshError::OK if the function terminates correctly, other else.
     */
    int saveOFF(const char* link) const;

    /**
     * @brief Loading .off file function.
     * @param link
     * @return MeshError::OK if the function terminates correctly, other else.
     */
    int saveOBJ(const char* link) const;

    /**
     * @brief Loading .off file function.
     * @param link
     * @return MeshError::OK if the function terminates correctly, other else.
     */
    int saveTXT(const char* link) const;

    /**
     * @brief Get the point corresponding to the center of the mesh.
     * @return The center point of the mesh.
     */
    QVector3D getCenter() const;

    /**
     * @brief Get the bounding raduis of the mesh.
     * @return The radius of the mesh.
     */
    float getBoundingRadius() const;

    /**
     * @brief Center the mesh to the origin (to avoid camera issues)
     */
    void centerToOrigin();

    /**
     * @brief Normalize the mesh, in case of the radius is too large for the camera.
     */
    void normalize();

    /**
     * @brief Normalize the mesh, in case of the radius is too large for the camera.
     */
    void deNormalize();

protected:

    /**
     * @brief Function who find the neighbor of a triangle.
     * @param triIndex : Indice of the first triangle.
     * @param a : First point of the edge.
     * @param b : Second point of the edge.
     * @return The indice of the neighbor, else -1.
     */
    int findNeighbor(unsigned int triIndex, unsigned int a, unsigned int b) const;

    /**
     * @brief Compute the area of a face.
     * @param faceIndex : Indice of the triangle.
     * @return The face area.
     */
    float faceArea(int faceIndex) const;

    /**
     * @brief Compute the normals of each vertex.
     */
    void computeNormals();

    /**
     * @brief Split a triangle by 3.
     * @param p : The point inside the triangle all the 3 triangles will contain this point.
     * @param triIndex : The indice of the splited triangle.
     */
    void triangleSplit(int p, int triIndex);

    /**
     * @brief Flip the common edge between 2 triangles.
     * @param t1 : The indice of the first triangle.
     * @param t2 : the indice if the second triangle.
     */
    void edgeFlip(int t1, int t2);

    /**
     * @brief Split the common edge between 2 triangles.
     * @param p : The indice of the point inside the edge.
     * @param t1 : The indice of the first triangle.
     * @param t2 : the indice if the second triangle.
     */
    void edgeSplit(int p, int t1, int t2);

    /**
     * @brief Compute the orientation test of a triangle.
     * @param p
     * @param q
     * @param r
     * @return Positive value if counterclockwise, else negative value.
     */
    float orientationTest(int p, int q, int r) const;

    /**
     * @brief Check if a point is inside a triangle.
     * @param p : The indice of the tested point.
     * @param triIndex : The indice of the triangle.
     * @return Positive value if the point is inside a triangle, else negative value.
     */
    int pointInTriangle(int p, int triIndex) const;

    /**
     * @brief Insert a point inside a mesh.
     * @param x : X coordinate of the point.
     * @param y : Y coordinate of the point.
     * @param z : Z coordinate of the point.
     * @return The index of the new point if the point is inserted, else -1.
     */
    int insert(float x, float y, float z);

    /**
     * @brief Check if 2 triangles are locally de Delaunay.
     * @param t1 : The indice of the first triangle.
     * @param t2 : The indice of the second triangle.
     * @return True if the triangles are de Delaunay, else false.
     */
    bool isLocallyDelaunay(int t1, int t2) const;

    /**
     * @brief Check if a point is inside the Circumcircle of a triangle.
     * @param a : The index of the first point.
     * @param b : The index of the second point.
     * @param c : The index of the third point.
     * @param d : The index of the tested point.
     * @return True if the point is inside the circumcircle, else false.
     */
    bool isInCircumcircleNorm(int a, int b, int c, int d) const;

    /**
     * @brief Performs the Lawsons' algorithm on he whole mesh.
     */
    void lawsonAlgorithm();

    /**
     * @brief Performs the Lawsons' algorithm locally in the incident faces around a point.
     * @param p : The indice of the point where the algorithm starts.
     */
    void lawsonLocalUpdate(int p);

    /**
     * @brief Initialize a super bounding triangle, useful for the construction of the Delaunay mesh.
     */
    void initializeSuperTriangle();

    /**
     * @brief Remove the super bounding triangle and reconnect correctly the mesh.
     */
    void removeSuperTriangle();

    std::vector<Node> vertices;
    std::vector<Triangle> faces;
    float normCoeff;
    bool hasTexCoords;
};

#endif // MESH_H

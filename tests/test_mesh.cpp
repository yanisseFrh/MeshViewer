#include <gtest/gtest.h>
#include "mesh.h"

class MeshTestable : public Mesh {
public:
    using Mesh::findNeighbor;
    using Mesh::faceArea;
    using Mesh::orientationTest;
    using Mesh::pointInTriangle;
    using Mesh::isInCircumcircleNorm;
    using Mesh::isLocallyDelaunay;
    using Mesh::triangleSplit;
    using Mesh::edgeFlip;
    using Mesh::edgeSplit;

    using Mesh::vertices;
    using Mesh::faces;
    using Mesh::normCoeff;
    using Mesh::hasTexCoords;
};

class MeshTest : public ::testing::Test {

protected:
    MeshTestable mesh;
};

TEST_F(MeshTest, EmptyMeshHasNoVertices) {
    auto& vertices = mesh.getVertices();
    EXPECT_EQ(vertices.size(), 0);
}

TEST_F(MeshTest, ClearMesh) {
    mesh.vertices.push_back(Node(0.0f,0.0f,0.0f));
    mesh.vertices.push_back(Node(1.0f,0.0f,0.0f));
    mesh.vertices.push_back(Node(0.0f,1.0f,0.0f));
    mesh.vertices.push_back(Node(0.0f,0.0f,1.0f));
    mesh.clear();
    EXPECT_TRUE(mesh.vertices.empty()) << "Vertices vector not empty\n";
}

TEST_F(MeshTest, GetIndices) {
    mesh.vertices.push_back(Node(0.0f,0.0f,0.0f));
    mesh.vertices.push_back(Node(1.0f,0.0f,0.0f));
    mesh.vertices.push_back(Node(0.0f,1.0f,0.0f));
    mesh.vertices.push_back(Node(0.0f,0.0f,1.0f));
    mesh.faces.push_back(Triangle(0,1,2));
    mesh.faces.push_back(Triangle(2,1,3));

    auto &indices = mesh.getIndices();

    EXPECT_EQ(indices.size(), mesh.faces.size()*3) << "Sizes doesn't match\n";

    std::size_t i = 0;
    for (auto &f: mesh.faces) {
        EXPECT_EQ(indices[i], f.idVertices[0]) << "Indices " << i << "doesn't match\n";
        EXPECT_EQ(indices[i+1], f.idVertices[1]) << "Indices " << i+1 << "doesn't match\n";
        EXPECT_EQ(indices[i+2], f.idVertices[2]) << "Indices " << i+2 << "doesn't match\n";
        i += 3;
    }
}

TEST_F(MeshTest, HasTextures) {
    EXPECT_FALSE(mesh.hasTexture()) << "Variable not false by default";
    mesh.hasTexCoords = !mesh.hasTexCoords;
    EXPECT_TRUE(mesh.hasTexture()) << "Variable didn't change";
}

TEST_F(MeshTest, Sew) {
    mesh.vertices = {
        Node(0.0f, 0.0f, 0.0f),
        Node(1.0f, 0.0f, 0.0f),
        Node(1.0f, 1.0f, 0.0f),
        Node(0.0f, 1.0f, 0.0f),
        Node(0.0f, 0.0f, 1.0f),
        Node(1.0f, 0.0f, 1.0f),
        Node(1.0f, 1.0f, 1.0f),
        Node(0.0f, 1.0f, 1.0f)
    };

    mesh.faces = {
        Triangle(0, 1, 2),
        Triangle(0, 2, 3),

        Triangle(4, 6, 5),
        Triangle(4, 7, 6),

        Triangle(0, 3, 7),
        Triangle(0, 7, 4),

        Triangle(1, 5, 6),
        Triangle(1, 6, 2),

        Triangle(0, 4, 5),
        Triangle(0, 5, 1),

        Triangle(3, 2, 6),
        Triangle(3, 6, 7)
    };

    mesh.sew();

    // All triangles have at least 1 neighbor
    for (size_t i = 0; i < mesh.faces.size(); ++i) {
        const auto& tri = mesh.faces[i];
        EXPECT_GT(tri.idFaces.size(), 0) << "Triangle " << i << " has no neighbors";
    }

    // If A has B as a neighbor, B has A as a neighbor too
    for (size_t i = 0; i < mesh.faces.size(); ++i) {
        for (auto &idB: mesh.faces[i].idFaces) {
            bool neighbor = false;
            for (auto &idA: mesh.faces[idB].idFaces) {
                if(idA == i) {
                    neighbor = true;
                    break;
                }
            }
            EXPECT_TRUE(neighbor) << "Triangles " << i << " and " << idB << " are not common neighbors\n";
        }
    }
}

TEST_F(MeshTest, LoadFileWrongFormat) {
    auto ok = mesh.loadFile("unknown_format.no");
    EXPECT_EQ(ok, MeshError::FORMAT);
}

/* octahedron.off
OFF
6 8 12
0.0 0.0 2.0
2.000000 0.000000 0.000000
0.000000 2.000000 0.000000
-2.000000 0.000000 0.000000
0.000000 -2.000000 0.000000
0.0 0.0 -2.0
3 1 0 4
3 4 0 3
3 3 0 2
3 2 0 1
3 1 5 2
3 2 5 3
3 3 5 4
3 4 5 1
*/
TEST_F(MeshTest, LoadOffCorrectly) {
    auto ok = mesh.loadFile("./data/test/octahedron.off");
    EXPECT_EQ(ok, MeshError::OK);
    EXPECT_EQ(mesh.vertices.size(), 6);
    EXPECT_EQ(mesh.faces.size(), 8);
    for (const auto &face : mesh.faces) {
        for (auto vid : face.idVertices) {
            EXPECT_GE(vid,0);
            EXPECT_LT(vid, mesh.vertices.size());
        }
    }
}

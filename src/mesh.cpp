#include "mesh.h"
#include "edgeKeyHash.h"

#include <iostream>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <exception>
#include <set>
#include <queue>

Mesh::Mesh() : normCoeff(0.0f), hasTexCoords(false) {}

const std::vector<Node> &Mesh::getVertices() const {
    return vertices;
}

const std::vector<unsigned int> Mesh::getIndices() const {
    std::vector<unsigned int> indices;
    for (auto &f: faces) {
        indices.push_back(f.idVertices[0]);
        indices.push_back(f.idVertices[1]);
        indices.push_back(f.idVertices[2]);
    }

    return indices;
}

const bool &Mesh::hasTexture() const {
    return hasTexCoords;
}

void Mesh::clear() {
    vertices.clear();
    faces.clear();
    hasTexCoords = false;
}

void Mesh::sew() {
    for (auto& f : faces) f.idFaces.clear();

    std::unordered_map<std::pair<int,int>, std::pair<int,int>, EdgeKeyHash> halfedge;

    for (std::size_t fi = 0; fi < faces.size(); ++fi) {
        faces[fi].idFaces.resize(3, -1);
        auto& tri = faces[fi];
        for (std::size_t e = 0; e < 3; ++e) {
            int u = tri.idVertices[e];
            int v = tri.idVertices[(e+1)%3];
            auto key = std::make_pair(u, v);
            halfedge.emplace(key, std::make_pair(fi, e));
        }
    }

    for (std::size_t fi = 0; fi < faces.size(); ++fi) {
        auto& tri = faces[fi];
        for (int e = 0; e < 3; ++e) {
            if (tri.idFaces[e] != -1) continue;
            int u = tri.idVertices[e];
            int v = tri.idVertices[(e+1)%3];
            auto oppKey = std::make_pair(v, u);
            auto it = halfedge.find(oppKey);
            if (it != halfedge.end()) {
                int fj = it->second.first;
                int ej = it->second.second;
                tri.idFaces[e] = fj;
                faces[fj].idFaces[ej] = fi;
            }
        }
    }
}

int Mesh::loadFile(const char* link) {
    std::string filename(link);
    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
    int ok;

    if (filename.size() >= 4 && filename.substr(filename.size() - 4) == ".off") {
        ok = loadOFF(link);
        return ok;
    } else if (filename.size() >= 4 && filename.substr(filename.size() - 4) == ".obj") {
        ok = loadOBJ(link);
        return ok;
    } else if (filename.size() >= 4 && filename.substr(filename.size() - 4) == ".txt") {
        ok = loadTXT(link);
        return ok;
    } else {
        return MeshError::FORMAT;
    }
}

int Mesh::loadOFF(const char* link) {
    std::ifstream meshFile(link);
    if (!meshFile.is_open()) {
        return MeshError::READ;
    }

    auto nextToken = [&](std::string &token) {
        while (meshFile >> token) {
            if (token[0] == '#') {
                std::string line;
                std::getline(meshFile, line);
                continue;
            }
            return true;
        }
        return false;
    };

    std::string header;
    if (!nextToken(header) || header != "OFF") {
        return MeshError::FORMAT;
    }

    int numVertices = 0, numFaces = 0, numEdges = 0;
    std::string token;

    if (!nextToken(token)) return MeshError::READ;
    numVertices = std::stoul(token);
    if (!nextToken(token)) return MeshError::READ;
    numFaces = std::stoul(token);
    if (!nextToken(token)) return MeshError::READ;
    numEdges = std::stoul(token);

    clear();

    for (int i = 0; i < numVertices; ++i) {
        float x, y, z;
        if (!(meshFile >> x >> y >> z)) {
            return MeshError::READ;
        }
        vertices.push_back(Node(x, y, z));
    }

    for (int i = 0; i < numFaces; ++i) {
        int nVerts;
        if (!(meshFile >> nVerts)) {
            return MeshError::READ;
        }

        std::vector<unsigned int> faceIndices(nVerts);
        for (int j = 0; j < nVerts; ++j) {
            meshFile >> faceIndices[j];
        }

        for (int j = 1; j < nVerts - 1; ++j) {
            faces.push_back(Triangle(faceIndices[0], faceIndices[j], faceIndices[j + 1]));
        }
    }

    meshFile.close();
    centerToOrigin();
    sew();
    computeNormals();

    return MeshError::OK;
}

int Mesh::loadOBJ(const char* link) {
    std::ifstream meshFile(link);
    if (!meshFile.is_open()) {
        return MeshError::READ;
    }

    clear();

    std::vector<QVector3D> tempPositions;
    std::vector<QVector3D> tempNormals;
    std::vector<QVector2D> tempTexCoords;

    std::string line;
    while (std::getline(meshFile, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            tempPositions.emplace_back(x, y, z);
        } else if (prefix == "vn") {
            float x, y, z;
            iss >> x >> y >> z;
            tempNormals.emplace_back(x, y, z);
        } else if (prefix == "vt") {
            float x, y;
            iss >> x >> y;
            tempTexCoords.emplace_back(x, y);
        } else if (prefix == "f") {
            std::string token;
            std::vector<unsigned int> faceIndices;

            while (iss >> token) {
                unsigned int vi = 0, ni = 0, ti = 0;
                // format v//n or v/t/n
                if (sscanf(token.c_str(), "%u//%u", &vi, &ni) == 2) {
                    // v//n
                }
                else if (sscanf(token.c_str(), "%u/%u/%u", &vi, &ti, &ni) == 3) {
                    // v/t/n
                }
                else if (sscanf(token.c_str(), "%u", &vi) == 1) {
                    // only v
                }

                vi--; ni--; ti--;

                Node v(tempPositions[vi]);
                if (ni < tempNormals.size() && ni >= 0) {
                    v.normal = tempNormals[ni];
                }
                if (ti < tempTexCoords.size() && ti >= 0) {
                    v.texCoords = tempTexCoords[ti];
                }

                vertices.push_back(v);
                faceIndices.push_back(vertices.size() - 1);
            }

            int nVerts = (int)faceIndices.size();
            for (int j = 1; j < nVerts - 1; ++j) {
                faces.push_back(Triangle(faceIndices[0], faceIndices[j], faceIndices[j + 1]));
            }
        }
    }

    meshFile.close();
    centerToOrigin();
    sew();
    if (tempNormals.empty()) computeNormals();
    if (!tempTexCoords.empty()) hasTexCoords = true;

    return MeshError::OK;
}

int Mesh::loadTXT(const char* link) {
    std::ifstream meshFile(link);
    if (!meshFile.is_open()) {
        return MeshError::READ;
    }

    auto nextToken = [&](std::string &token) {
        while (meshFile >> token) {
            if (token[0] == '#') {
                std::string line;
                std::getline(meshFile, line);
                continue;
            }
            return true;
        }
        return false;
    };

    int numVertices = 0;
    std::string token;
    if (!nextToken(token)) return MeshError::READ;
    try {
        numVertices = std::stoul(token);
    } catch (const std::invalid_argument&) {
        return MeshError::FORMAT;
    } catch (const std::out_of_range&) {
        return MeshError::FORMAT;
    }

    clear();

    initializeSuperTriangle();

    for (int i = 0; i < numVertices; ++i) {
        float x, y, z;
        if (!(meshFile >> x >> y >> z)) {
            return MeshError::READ;
        }

        int newPointIndex = insert(x, y, z);
        if (newPointIndex == -1) {
            std::cerr << "Failed to insert point " << i << ": (" << x << ", " << y << ", " << z << ")\n";
        }
    }

    meshFile.close();

    removeSuperTriangle();
    centerToOrigin();
    sew();
    computeNormals();

    return MeshError::OK;
}

int Mesh::saveFile(const char *link) {
    std::string filename(link);
    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
    int ok;

    if (normCoeff != 0.0f) deNormalize();

    if (filename.size() >= 4 && filename.substr(filename.size() - 4) == ".off") {
        ok = saveOFF(link);
        return ok;
    } else if (filename.size() >= 4 && filename.substr(filename.size() - 4) == ".obj") {
        ok = saveOBJ(link);
        return ok;
    } else if (filename.size() >= 4 && filename.substr(filename.size() - 4) == ".txt") {
        ok = saveTXT(link);
        return ok;
    } else {
        return MeshError::FORMAT;
    }
}

int Mesh::saveOFF(const char* link) const {
    std::ofstream meshFile;
    meshFile.open(link);
    if(!meshFile.is_open()) {
        std::cerr << "Can't open file \"" << link << "\"\n";
        return MeshError::SAVE;
    }

    meshFile << "OFF" << std::endl;
    meshFile << vertices.size() << " " << faces.size() << " " << 0 << std::endl;

    for(auto& v : vertices) {
        meshFile << v.position.x() << " " << v.position.y() << " " << v.position.z() << std::endl;
    }

    for(auto&f : faces) {
        meshFile << 3 << " " << f.idVertices[0] << " " << f.idVertices[1] << " " << f.idVertices[2] << std::endl;
    }

    meshFile.close();
    return MeshError::OK;
}

int Mesh::saveOBJ(const char *link) const {
    std::ofstream meshFile(link);
    if (!meshFile.is_open()) {
        std::cerr << "Can't open file \"" << link << "\"\n";
        return MeshError::SAVE;
    }

    for (const Node& v : vertices) {
        meshFile << "v " << v.position.x() << " " << v.position.y() << " " << v.position.z() << "\n";
    }

    bool hasTexCoords = !vertices.empty() && (vertices[0].texCoords != QVector2D());
    if (hasTexCoords) {
        for (const Node& v : vertices) {
            meshFile << "vt " << v.texCoords.x() << " " << v.texCoords.y() << "\n";
        }
    }

    bool hasNormals = !vertices.empty() && (vertices[0].normal != QVector3D());
    if (hasNormals) {
        for (const Node& v : vertices) {
            meshFile << "vn " << v.normal.x() << " " << v.normal.y() << " " << v.normal.z() << "\n";
        }
    }

    for (const Triangle& tri : faces) {
        unsigned int idx0 = tri.idVertices[0] + 1;
        unsigned int idx1 = tri.idVertices[1] + 1;
        unsigned int idx2 = tri.idVertices[2] + 1;

        if (hasTexCoords && hasNormals) {
            // format v/t/n
            meshFile << "f "
                     << idx0 << "/" << idx0 << "/" << idx0 << " "
                     << idx1 << "/" << idx1 << "/" << idx1 << " "
                     << idx2 << "/" << idx2 << "/" << idx2 << "\n";
        }
        else if (hasTexCoords) {
            // format v/t
            meshFile << "f "
                     << idx0 << "/" << idx0 << " "
                     << idx1 << "/" << idx1 << " "
                     << idx2 << "/" << idx2 << "\n";
        }
        else if (hasNormals) {
            // format v//n (no texture)
            meshFile << "f "
                     << idx0 << "//" << idx0 << " "
                     << idx1 << "//" << idx1 << " "
                     << idx2 << "//" << idx2 << "\n";
        }
        else {
            // format v only
            meshFile << "f " << idx0 << " " << idx1 << " " << idx2 << "\n";
        }
    }

    meshFile.close();
    return MeshError::OK;
}

int Mesh::saveTXT(const char *link) const {
    std::ofstream meshFile;
    meshFile.open(link);
    if(!meshFile.is_open()) {
        std::cerr << "Can't open file \"" << link << "\"\n";
        return MeshError::SAVE;
    }

    meshFile << vertices.size() << std::endl;

    for (auto &v : vertices) {
        meshFile << v.position.x() << " " << v.position.y() << " " << v.position.z() << std::endl;
    }

    meshFile.close();
    return MeshError::OK;
}

int Mesh::findNeighbor(unsigned int triIndex, unsigned int a, unsigned int b) const {
    int res = -1;

    for (int i: faces[triIndex].idFaces) {
        const Triangle &t = faces[i];
        if ((a == t.idVertices[1] && b == t.idVertices[0]) ||
            (a == t.idVertices[2] && b == t.idVertices[1]) ||
            (a == t.idVertices[0] && b == t.idVertices[2])) {
            res = i;
            break;
        }
    }
    return res;
}

float Mesh::faceArea(int faceIndex) const {
    QVector3D a = vertices[faces[faceIndex].idVertices[0]].position;
    QVector3D b = vertices[faces[faceIndex].idVertices[1]].position;
    QVector3D c = vertices[faces[faceIndex].idVertices[2]].position;
    QVector3D ba = QVector3D(a - b);
    QVector3D ca = QVector3D(a - c);
    return QVector3D::dotProduct(ba, ca) / 2;
}

void Mesh::computeNormals() {
    for (auto &v : vertices) {
        v.normal = QVector3D(0,0,0);
    }

    for (std::size_t i = 0; i < faces.size(); i ++) {
        unsigned int i0 = faces[i].idVertices[0];
        unsigned int i1 = faces[i].idVertices[1];
        unsigned int i2 = faces[i].idVertices[2];

        QVector3D &v0 = vertices[i0].position;
        QVector3D &v1 = vertices[i1].position;
        QVector3D &v2 = vertices[i2].position;

        QVector3D normal = QVector3D::crossProduct(v1 - v0, v2 - v0).normalized();

        vertices[i0].normal += normal;
        vertices[i1].normal += normal;
        vertices[i2].normal += normal;
    }

    for (auto &v : vertices) {
        v.normal.normalize();
    }
}


QVector3D Mesh::getCenter() const {
    if (vertices.empty()) return QVector3D(0,0,0);
    QVector3D min = vertices[0].position;
    QVector3D max = vertices[0].position;
    for (const auto &v : vertices) {
        min.setX(std::min(min.x(), v.position.x()));
        min.setY(std::min(min.y(), v.position.y()));
        min.setZ(std::min(min.z(), v.position.z()));
        max.setX(std::max(max.x(), v.position.x()));
        max.setY(std::max(max.y(), v.position.y()));
        max.setZ(std::max(max.z(), v.position.z()));
    }
    return (min + max) * 0.5f;
}

float Mesh::getBoundingRadius() const {
    QVector3D center = getCenter();
    float maxDist = 0.0f;
    for (const auto &v : vertices) {
        maxDist = std::max(maxDist, (v.position - center).length());
    }
    return maxDist;
}

void Mesh::centerToOrigin() {
    if (vertices.empty()) return;

    QVector3D min = vertices[0].position;
    QVector3D max = vertices[0].position;

    for (const auto& v : vertices) {
        QVector3D pos = v.position;
        min.setX(std::min(min.x(), pos.x()));
        min.setY(std::min(min.y(), pos.y()));
        min.setZ(std::min(min.z(), pos.z()));
        max.setX(std::max(max.x(), pos.x()));
        max.setY(std::max(max.y(), pos.y()));
        max.setZ(std::max(max.z(), pos.z()));
    }

    QVector3D center = (min + max) * 0.5f;

    for (auto& v : vertices) {
        v.position -= center;
    }
}

void Mesh::initializeSuperTriangle() {
    vertices.push_back(Node(-100000.0f, -100000.0f, 0.0f));
    vertices.push_back(Node(100000.0f, -100000.0f, 0.0f));
    vertices.push_back(Node(0.0f, 100000.0f, 0.0f));
    faces.push_back(Triangle(0, 1, 2));
}

void Mesh::removeSuperTriangle() {
    const int superVertex1 = 0;
    const int superVertex2 = 1;
    const int superVertex3 = 2;

    std::vector<Triangle> validFaces;
    validFaces.reserve(faces.size());

    for (const Triangle& face : faces) {
        bool containsSuperVertex = false;

        for (int i = 0; i < 3; i++) {
            int vertexId = face.idVertices[i];
            if (vertexId == superVertex1 || vertexId == superVertex2 || vertexId == superVertex3) {
                containsSuperVertex = true;
                break;
            }
        }

        if (!containsSuperVertex) {
            validFaces.push_back(face);
        }
    }

    vertices.erase(vertices.begin(), vertices.begin() + 3);

    for (Triangle& face : validFaces) {
        for (int i = 0; i < 3; i++) {
            if (face.idVertices[i] >= 3) {
                face.idVertices[i] -= 3;
            } else {
                std::cerr << "Error: Triangle still references super-triangle vertex!\n";
            }
        }

        face.idFaces.clear();
    }

    faces = std::move(validFaces);
}

void Mesh::triangleSplit(int p, int triIndex) {
    if (triIndex<0 || triIndex>=faces.size() || p<0 || p>=vertices.size()) return;
    Triangle tri = faces[triIndex];
    int u = tri.idVertices[0];
    int v = tri.idVertices[1];
    int w = tri.idVertices[2];

    int nei1Index = findNeighbor(triIndex, u, v);
    int nei2Index = findNeighbor(triIndex, v, w);
    int nei3Index = findNeighbor(triIndex, w, u);

    faces[triIndex] = Triangle(u,v,p);
    faces.push_back(Triangle(v,w,p));
    faces.push_back(Triangle(w,u,p));
    int tri2Index = faces.size() - 2;
    int tri3Index = faces.size() - 1;

    if (nei1Index != -1) faces[triIndex].idFaces.push_back(nei1Index);

    if (nei2Index != -1) {
        auto& nei2 = faces[nei2Index].idFaces;
        auto it = std::find(nei2.begin(), nei2.end(), triIndex);
        if (it != nei2.end()) nei2.erase(it);
        nei2.push_back(tri2Index);
        faces[tri2Index].idFaces.push_back(nei2Index);
    }

    if (nei3Index != -1) {
        auto& nei3 = faces[nei3Index].idFaces;
        auto it = std::find(nei3.begin(), nei3.end(), triIndex);
        if (it != nei3.end()) nei3.erase(it);
        nei3.push_back(tri3Index);
        faces[tri3Index].idFaces.push_back(nei3Index);
    }

    faces[triIndex].idFaces.push_back(tri2Index);
    faces[triIndex].idFaces.push_back(tri3Index);
    faces[tri2Index].idFaces.push_back(triIndex);
    faces[tri2Index].idFaces.push_back(tri3Index);
    faces[tri3Index].idFaces.push_back(triIndex);
    faces[tri3Index].idFaces.push_back(tri2Index);

}

void Mesh::edgeFlip(int t1, int t2) {
    if (t1<0 || t1>=faces.size() || t2<0 || t2>=faces.size()) return;
    auto tri1 = faces[t1];
    auto tri2 = faces[t2];

    std::pair<int, int> commonEdge = tri1.findCommonEdge(tri2);
    if (commonEdge.first == -1 || commonEdge.second == -1) {
        std::cerr << "Common edge not found\n";
        return;
    }

    int b = commonEdge.first;
    int c = commonEdge.second;

    int cLocal = tri1.localIndex(c);
    int cLocal_t2 = tri2.localIndex(b);

    int aLocal = (cLocal + 1) % 3;
    int dLocal = (cLocal_t2 + 1) % 3;


    faces[t1].idVertices[cLocal] = tri2.idVertices[dLocal];
    faces[t2].idVertices[cLocal_t2] = tri1.idVertices[aLocal];

    int nei1To2Index = findNeighbor(t1, c, faces[t1].idVertices[aLocal]);
    int nei2To1Index = findNeighbor(t2, b, faces[t2].idVertices[dLocal]);

    if (nei1To2Index != -1) {
        auto& nei1To2 = faces[nei1To2Index].idFaces;
        auto it = std::find(nei1To2.begin(), nei1To2.end(), t1);
        if (it != nei1To2.end()) nei1To2.erase(it);
        nei1To2.push_back(t2);
        faces[t2].idFaces.push_back(nei1To2Index);
    }

    if (nei2To1Index != -1) {
        auto& nei2To1 = faces[nei2To1Index].idFaces;
        auto it = std::find(nei2To1.begin(), nei2To1.end(), t2);
        if (it != nei2To1.end()) nei2To1.erase(it);
        nei2To1.push_back(t1);
        faces[t1].idFaces.push_back(nei2To1Index);
    }
}

void Mesh::edgeSplit(int p, int t1, int t2) {
    if (t1<0 || t1>=faces.size() || t2<0 || t2>=faces.size() || p<0 || p>=vertices.size()) return;

    auto tri1 = faces[t1];
    auto tri2 = faces[t2];

    std::pair<int, int> commonEdge = tri1.findCommonEdge(tri2);
    if (commonEdge.first == -1 || commonEdge.second == -1) {
        std::cerr << "Common edge not found\n";
        return;
    }

    int b = commonEdge.first;
    int c = commonEdge.second;

    int a = -1;
    int d = -1;

    for (int i = 0; i < 3; i++) {
        if (tri1.idVertices[i] != b && tri1.idVertices[i] != c) {
            a = tri1.idVertices[i];
            break;
        }
    }

    for (int i = 0; i < 3; i++) {
        if (tri2.idVertices[i] != b && tri2.idVertices[i] != c) {
            d = tri2.idVertices[i];
            break;
        }
    }

    if (a == -1 || d == -1) return;

    faces[t1] = Triangle(a, b, p);
    faces[t2] = Triangle(b, p, d);

    faces.push_back(Triangle(a, p, c));
    faces.push_back(Triangle(p, c, d));

    sew();
    computeNormals();
}

float Mesh::orientationTest(int p, int q, int r) const {
    if (p < 0 || p >= vertices.size() || q < 0 || q >= vertices.size() || r < 0 || r >= vertices.size()) {
        return 0.0f;
    }

    QVector3D P = vertices[p].position;
    QVector3D Q = vertices[q].position;
    QVector3D R = vertices[r].position;

    float orientation = (Q.x() - P.x()) * (R.y() - P.y()) - (Q.y() - P.y()) * (R.x() - P.x());

    return orientation;
}

int Mesh::pointInTriangle(int p, int triIndex) const {
    if (triIndex < 0 || triIndex >= faces.size() || p < 0 || p >= vertices.size()) {
        return -1;
    }

    const Triangle& tri = faces[triIndex];
    int a = tri.idVertices[0];
    int b = tri.idVertices[1];
    int c = tri.idVertices[2];

    float o1 = orientationTest(a, b, p);
    float o2 = orientationTest(b, c, p);
    float o3 = orientationTest(c, a, p);

    float epsilon = std::numeric_limits<float>::epsilon();

    bool allPositive = (o1 >= epsilon && o2 >= epsilon && o3 >= epsilon);
    bool allNegative = (o1 <= epsilon && o2 <= epsilon && o3 <= epsilon);

    if (allPositive || allNegative) {
        if (o1 == 0.0f || o2 == 0.0f || o3 == 0.0f) {
            return 0;
        }
        return 1;
    }

    return -1;
}

int Mesh::insert(float x, float y, float z) {
    vertices.push_back(Node(x, y, z));
    int pIndex = vertices.size() - 1;

    int containingTriangle = -1;
    for (std::size_t i = 0; i < faces.size(); i++) {
        int result = pointInTriangle(pIndex, i);
        if (result >= 0) {
            containingTriangle = i;
            break;
        }
    }

    if (containingTriangle == -1) {
        std::cerr << "Warning: Point (" << x << ", " << y << ", " << z
                  << ") not inside any triangle during insertion\n";
        vertices.pop_back();
        return -1;
    }

    triangleSplit(pIndex, containingTriangle);
    lawsonLocalUpdate(pIndex);

    return pIndex;
}


bool Mesh::isInCircumcircleNorm(int a, int b, int c, int d) const {
    if (a < 0 || a >= vertices.size() || b < 0 || b >= vertices.size() ||
        c < 0 || c >= vertices.size() || d < 0 || d >= vertices.size()) {
        return false;
    }

    float ax = vertices[a].position.x(), ay = vertices[a].position.y();
    float bx = vertices[b].position.x(), by = vertices[b].position.y();
    float cx = vertices[c].position.x(), cy = vertices[c].position.y();
    float dx = vertices[d].position.x(), dy = vertices[d].position.y();

    float adx = ax - dx, ady = ay - dy;
    float bdx = bx - dx, bdy = by - dy;
    float cdx = cx - dx, cdy = cy - dy;

    float det = adx * (bdy * (cdx*cdx + cdy*cdy) - (bdx*bdx + bdy*bdy) * cdy) -
                ady * (bdx * (cdx*cdx + cdy*cdy) - (bdx*bdx + bdy*bdy) * cdx) +
                (adx*adx + ady*ady) * (bdx * cdy - bdy * cdx);

    float epsilon = std::numeric_limits<float>::epsilon();

    return det > epsilon;
}

bool Mesh::isLocallyDelaunay(int t1, int t2) const {
    if (t1 < 0 || t1 >= faces.size() || t2 < 0 || t2 >= faces.size() || t1 == t2) {
        return true;
    }

    const Triangle& tri1 = faces[t1];
    const Triangle& tri2 = faces[t2];

    std::pair<int, int> commonEdge = tri1.findCommonEdge(tri2);
    if (commonEdge.first == -1 || commonEdge.second == -1) {
        return true;
    }

    int a = commonEdge.first;
    int b = commonEdge.second;

    int c = -1;
    for (int i = 0; i < 3; i++) {
        int vertex = tri1.idVertices[i];
        if (vertex != a && vertex != b) {
            c = vertex;
            break;
        }
    }

    int d = -1;
    for (int i = 0; i < 3; i++) {
        int vertex = tri2.idVertices[i];
        if (vertex != a && vertex != b) {
            d = vertex;
            break;
        }
    }

    if (c == -1 || d == -1) {
        return true;
    }

    return !isInCircumcircleNorm(a, b, c, d);
}

void Mesh::lawsonAlgorithm() {
    std::queue<std::pair<int, int>> edgeQueue;
    std::set<std::pair<int, int>> processed;
    int maxIterations = faces.size() * faces.size();
    int iterations = 0;

    for (std::size_t i = 0; i < faces.size(); i++) {
        for (int j : faces[i].idFaces) {
            if (i < j) {
                edgeQueue.push({i, j});
            }
        }
    }

    while (!edgeQueue.empty() && iterations < maxIterations) {
        iterations++;

        auto [t1, t2] = edgeQueue.front();
        edgeQueue.pop();

        if (t1 >= (int)faces.size() || t2 >= (int)faces.size()) continue;

        std::pair<int, int> edgePair = {std::min(t1, t2), std::max(t1, t2)};
        if (processed.find(edgePair) != processed.end()) continue;
        processed.insert(edgePair);

        if (!isLocallyDelaunay(t1, t2)) {
            std::vector<unsigned int> oldNeighbors_t1 = faces[t1].idFaces;
            std::vector<unsigned int> oldNeighbors_t2 = faces[t2].idFaces;

            edgeFlip(t1, t2);

            for (int neighbor : faces[t1].idFaces) {
                if (neighbor != t2) {
                    std::pair<int, int> newEdge = {std::min(t1, neighbor), std::max(t1, neighbor)};
                    if (processed.find(newEdge) == processed.end()) {
                        edgeQueue.push({t1, neighbor});
                    }
                }
            }

            for (int neighbor : faces[t2].idFaces) {
                if (neighbor != t1) {
                    std::pair<int, int> newEdge = {std::min(t2, neighbor), std::max(t2, neighbor)};
                    if (processed.find(newEdge) == processed.end()) {
                        edgeQueue.push({t2, neighbor});
                    }
                }
            }

            processed.clear();
        }
    }

    if (iterations >= maxIterations) {
        std::cerr << "Lawson algorithm reached maximum iterations limit\n";
    }
}

void Mesh::lawsonLocalUpdate(int p) {
    if (p < 0 || p >= (int)vertices.size()) return;

    std::queue<std::pair<int, int>> edgeQueue;
    std::set<std::pair<int, int>> processed;
    int maxIterations = 100;
    int iterations = 0;

    std::vector<int> incidentTriangles;
    for (int i = 0; i < (int)faces.size(); i++) {
        const Triangle& tri = faces[i];
        for (int j = 0; j < 3; j++) {
            if (tri.idVertices[j] == p) {
                incidentTriangles.push_back(i);
                break;
            }
        }
    }

    for (int triIndex : incidentTriangles) {
        const Triangle& tri = faces[triIndex];

        int oppositeVertex1 = -1, oppositeVertex2 = -1;
        for (int j = 0; j < 3; j++) {
            if (tri.idVertices[j] != p) {
                if (oppositeVertex1 == -1) {
                    oppositeVertex1 = tri.idVertices[j];
                } else {
                    oppositeVertex2 = tri.idVertices[j];
                    break;
                }
            }
        }

        for (int adjTriIndex : faces[triIndex].idFaces) {
            if (adjTriIndex != triIndex) {
                const Triangle& adjTri = faces[adjTriIndex];
                bool sharesOppositeEdge = false;
                int sharedCount = 0;

                for (int k = 0; k < 3; k++) {
                    int adjVertex = adjTri.idVertices[k];
                    if (adjVertex == oppositeVertex1 || adjVertex == oppositeVertex2) {
                        sharedCount++;
                    }
                }

                if (sharedCount == 2) {
                    std::pair<int, int> edgePair = {std::min(triIndex, adjTriIndex),
                                                    std::max(triIndex, adjTriIndex)};
                    if (processed.find(edgePair) == processed.end()) {
                        edgeQueue.push({triIndex, adjTriIndex});
                    }
                }
            }
        }
    }

    while (!edgeQueue.empty() && iterations < maxIterations) {
        iterations++;

        auto [t1, t2] = edgeQueue.front();
        edgeQueue.pop();

        if (t1 >= (int)faces.size() || t2 >= (int)faces.size()) continue;

        std::pair<int, int> edgePair = {std::min(t1, t2), std::max(t1, t2)};
        if (processed.find(edgePair) != processed.end()) continue;
        processed.insert(edgePair);

        if (!isLocallyDelaunay(t1, t2)) {

            edgeFlip(t1, t2);

            for (int neighbor : faces[t1].idFaces) {
                if (neighbor != t2) {
                    bool inInfluenceZone = false;
                    const Triangle& neighborTri = faces[neighbor];
                    for (int k = 0; k < 3; k++) {
                        if (neighborTri.idVertices[k] == p) {
                            inInfluenceZone = true;
                            break;
                        }
                    }

                    if (!inInfluenceZone) {
                        for (int incidentTri : incidentTriangles) {
                            if (incidentTri < (int)faces.size()) {
                                for (int adj : faces[incidentTri].idFaces) {
                                    if (adj == neighbor) {
                                        inInfluenceZone = true;
                                        break;
                                    }
                                }
                                if (inInfluenceZone) break;
                            }
                        }
                    }

                    if (inInfluenceZone) {
                        std::pair<int, int> newEdge = {std::min(t1, neighbor), std::max(t1, neighbor)};
                        if (processed.find(newEdge) == processed.end()) {
                            edgeQueue.push({t1, neighbor});
                        }
                    }
                }
            }

            for (int neighbor : faces[t2].idFaces) {
                if (neighbor != t1 && neighbor > 0) {
                    bool inInfluenceZone = false;
                    const Triangle& neighborTri = faces[neighbor];
                    for (int k = 0; k < 3; k++) {
                        if (neighborTri.idVertices[k] == p) {
                            inInfluenceZone = true;
                            break;
                        }
                    }

                    if (!inInfluenceZone) {
                        for (int incidentTri : incidentTriangles) {
                            if (incidentTri < (int)faces.size()) {
                                for (int adj : faces[incidentTri].idFaces) {
                                    if (adj == neighbor) {
                                        inInfluenceZone = true;
                                        break;
                                    }
                                }
                                if (inInfluenceZone) break;
                            }
                        }
                    }

                    if (inInfluenceZone) {
                        std::pair<int, int> newEdge = {std::min(t2, neighbor), std::max(t2, neighbor)};
                        if (processed.find(newEdge) == processed.end()) {
                            edgeQueue.push({t2, neighbor});
                        }
                    }
                }
            }
        }
    }

    if (iterations >= maxIterations) {
        std::cerr << "Local Lawson update reached iteration limit for point " << p << "\n";
    }
}

void Mesh::normalize() {
    normCoeff = getBoundingRadius();
    float scale = 30.0f / normCoeff;

    for (auto& vertex : vertices) {
        vertex.position = vertex.position * scale;
    }
}

void Mesh::deNormalize() {
    float scale = normCoeff / 30.0f;

    for (auto& vertex : vertices) {
        vertex.position = vertex.position * scale;
    }

    normCoeff = 0.0f;
}

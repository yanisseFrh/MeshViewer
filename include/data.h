#ifndef DATA_H
#define DATA_H

#include "mesh.h"

class Data {
public:
    static Data* TheInstance();
    static void DestroyTheInstance();

private:
    Mesh m_mesh;
};

#endif // DATA_H

#include "data.h"

static Data* s_instance = nullptr;

Data* Data::TheInstance() {
    if(!s_instance) {
        s_instance = new Data();
    }

    return s_instance;
}

void Data::DestroyTheInstance() {
    if(s_instance) {
        delete s_instance;
        s_instance = nullptr;
    }

}

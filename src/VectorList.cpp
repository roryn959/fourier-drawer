#include "VectorList.h"

VectorList::VectorList() :
    m_list{}
{
}

ComplexPoint VectorList::operator()(float t) {
    ComplexPoint result{};

    for (Vector& v : m_list) {
        result = result + v(t);
    }

    return result;
}
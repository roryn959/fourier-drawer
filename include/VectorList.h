#ifndef VECTOR_LIST_H
#define VECTOR_LIST_H

#include <vector>
#include <iostream>
#include <cmath>

#include "Vector.h"
#include "ComplexPoint.h"

class VectorList {
public:
    VectorList();

    inline void push_back(Vector v) { m_list.push_back(v); }
    inline void sort_by_amp() { sort(m_list.begin(), m_list.end()); }

    ComplexPoint operator()(float t);

    auto begin(){ return m_list.begin(); };
    auto end(){ return m_list.end(); };

    auto begin() const { return m_list.begin(); };
    auto end() const { return m_list.end(); };

private:
    std::vector<Vector> m_list;
};

#endif
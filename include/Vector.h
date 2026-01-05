#ifndef VECTOR_H
#define VECTOR_H

#include <cmath>

#include "ComplexPoint.h"

class Vector {
public:
    Vector(float amplitude, float frequency, float phase);
    
    ComplexPoint operator()(float t) const;
    bool operator<(const Vector& other) const {
        return abs(m_amplitude) > abs(other.m_amplitude);
    }

private:
    float m_amplitude;
    float m_frequency;
    float m_phase;
};

#endif
#include "Vector.h"

Vector::Vector(float amplitude, float frequency, float phase) :
    m_amplitude{amplitude},
    m_frequency{frequency},
    m_phase{phase} {}

ComplexPoint Vector::operator()(float t) const {
    float real_part = m_amplitude * cos(2 * M_PI * m_frequency * t + m_phase);
    float imaginary_part = m_amplitude * sin(2 * M_PI * m_frequency * t + m_phase);
    return ComplexPoint{ real_part, imaginary_part };
}
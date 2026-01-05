#include "ComplexPoint.h"

ComplexPoint ComplexPoint::operator+(const ComplexPoint& other) const {
    return ComplexPoint{ real_part + other.real_part, imaginary_part + other.imaginary_part };
}
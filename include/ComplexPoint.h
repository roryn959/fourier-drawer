#ifndef COMPLEX_POINT_H
#define COMPLEX_POINT_H

struct ComplexPoint {
    float real_part;
    float imaginary_part;

    ComplexPoint operator+(const ComplexPoint& other) const;
};

#endif
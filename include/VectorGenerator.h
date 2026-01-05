#ifndef VECTOR_GENERATOR_H
#define VECTOR_GENERATOR_H

#include <vector>
#include <fstream>
#include <regex>

#include "Vector.h"
#include "VectorList.h"
#include "ComplexPoint.h"

#define GRAPH_W 80.0
#define GRAPH_H 100.0

#define NUM_SAMPLES 10


class VectorGenerator {
public:
    static void GenerateVectors(VectorList& vector_list);
private:
    static std::vector<ComplexPoint> GeneratePointSamplesFromSvg();
};

#endif
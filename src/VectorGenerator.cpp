#include "VectorGenerator.h"

void VectorGenerator::GenerateVectors(VectorList& vector_list) {

    std::vector<ComplexPoint> samples = GeneratePointSamplesFromSvg();

    int N = samples.size();
    int vector_halfrange = 100;

    for (int n = -vector_halfrange; n <= vector_halfrange; ++n) {
        float real_part{0};
        float imaginary_part{0};

        // Count up C_k = integral -> f(t) * e ^ (-2 * pi * i * k * t)
        for (int t = 0; t < N; ++t) {
            ComplexPoint point = samples[t];
            float theta = -2 * M_PI * n * (t / (float) N);
            real_part += point.real_part * cos(theta) + point.imaginary_part * sin(theta);
            imaginary_part += point.imaginary_part * cos(theta) - point.real_part * sin(theta);
        }

        real_part /= N;
        imaginary_part /= N;

        float amplitude = sqrt(real_part*real_part + imaginary_part * imaginary_part);
        float phase = atan2(imaginary_part, real_part);
        vector_list.push_back(Vector{ amplitude, (float) n / 35.0f, phase });
    }

    vector_list.sort_by_amp();
}

std::vector<ComplexPoint> VectorGenerator::GeneratePointSamplesFromSvg() {
    std::vector<ComplexPoint> samples;

    std::ifstream file("../drawing.svg");
    if (!file) {
        std::cout << "File not found...\n";
        std::exit(-1);
    }

    std::string svg((std::istreambuf_iterator<char>(file)), {});

    size_t start = svg.find("d=\"");
    if (start == std::string::npos) {
        std::cerr << "No path found.\n";
        return samples;
    }

    start += 3;
    size_t end = svg.find("\"", start);
    std::string path = svg.substr(start, end - start);

    std::string pathData = path;
    std::replace(pathData.begin(), pathData.end(), ',', ' ');

    std::regex tokenRegex("([MLCQTAZHV])|(-?\\d*\\.?\\d+(?:e[-+]?\\d+)?)");

    std::vector<std::string> tokens;
    for (std::sregex_iterator it(pathData.begin(), pathData.end(), tokenRegex), end; it != end; ++it) {
        tokens.push_back(it->str());
    }

    ComplexPoint last{0.0f, 0.0f};

    char currentCmd = 0;
    for (size_t i = 0; i < tokens.size();) {
        if (isalpha(tokens[i][0])) {
            currentCmd = tokens[i][0];
            ++i;
        }

        if (currentCmd == 'M') {
            float x = std::stof(tokens[i++]);
            float y = std::stof(tokens[i++]);

            last = ComplexPoint{ x, y };
            samples.push_back(last);
        } else if (currentCmd == 'L') {
            float x = std::stof(tokens[i++]);
            float y = std::stof(tokens[i++]);

            ComplexPoint target{ x, y };

            float dx = (x - last.real_part) / NUM_SAMPLES;
            float dy = (y - last.imaginary_part) / NUM_SAMPLES;

            float nx = last.real_part;
            float ny = last.imaginary_part;

            for (int i = 0; i < NUM_SAMPLES; ++i) {
                nx += dx;
                ny += dy;
                samples.push_back(ComplexPoint{nx, ny});
            }

            last = target;
        } else if (currentCmd == 'C') {
            float x1 = std::stof(tokens[i++]);
            float y1 = std::stof(tokens[i++]);
            float x2 = std::stof(tokens[i++]);
            float y2 = std::stof(tokens[i++]);
            float x = std::stof(tokens[i++]);
            float y = std::stof(tokens[i++]);

            const int STEPS = NUM_SAMPLES;
            for (int s = 1; s <= STEPS; ++s) {
                float t = (float) s / STEPS;
                float xt = pow(1 - t, 3)*last.real_part + 3*pow(1 - t, 2)*t*x1 + 3*(1 - t)*t*t*x2 + pow(t, 3)*x;
                float yt = pow(1 - t, 3)*last.imaginary_part + 3*pow(1 - t, 2)*t*y1 + 3*(1 - t)*t*t*y2 + pow(t, 3)*y;
                samples.push_back(ComplexPoint{xt, yt});
            }
            last = ComplexPoint{ x, y };
        } else if (currentCmd == 'Q') {
            float x1 = std::stof(tokens[i++]);
            float y1 = std::stof(tokens[i++]);
            float x = std::stof(tokens[i++]);
            float y = std::stof(tokens[i++]);

            const int STEPS = NUM_SAMPLES;
            for (int s = 1; s <= STEPS; ++s) {
                float t = (float)s / STEPS;
                float xt = (1 - t)*(1 - t)*last.real_part + 2*(1 - t)*t*x1 + t*t*x;
                float yt = (1 - t)*(1 - t)*last.imaginary_part + 2*(1 - t)*t*y1 + t*t*y;
                samples.push_back({xt, yt});
            }
            last.real_part = x;
            last.imaginary_part = y;

        } else if (currentCmd == 'A') {
            float rx = std::stof(tokens[i++]);
            float ry = std::stof(tokens[i++]);
            float xAxisRot = std::stof(tokens[i++]);
            float largeArc = std::stof(tokens[i++]);
            float sweep = std::stof(tokens[i++]);
            float x = std::stof(tokens[i++]);
            float y = std::stof(tokens[i++]);

            // crude fallback: just linearly interpolate
            const int STEPS = NUM_SAMPLES;
            for (int s = 1; s <= STEPS; ++s) {
                float t = (float)s / STEPS;
                float xt = last.real_part + t * (x - last.real_part);
                float yt = last.imaginary_part + t * (y - last.imaginary_part);
                samples.push_back({xt, yt});
            }
            last.real_part = x;
            last.imaginary_part = y;
        } else if (currentCmd == 'H') {
            float x = std::stof(tokens[i++]);
            samples.push_back({x, last.imaginary_part});
            last.real_part = x;
        } else if (currentCmd == 'V') {
            float y = std::stof(tokens[i++]);
            samples.push_back({last.real_part, y});
            last.imaginary_part = y;
        } else {
            ++i;
        }
    }

    // scale

    float minX = samples[0].real_part, maxX = samples[0].real_part;
    float minY = samples[0].imaginary_part, maxY = samples[0].imaginary_part;
    for (auto& p : samples) {
        minX = std::min(minX, p.real_part);
        maxX = std::max(maxX, p.real_part);
        minY = std::min(minY, p.imaginary_part);
        maxY = std::max(maxY, p.imaginary_part);
    }
    float width = maxX - minX;
    float height = maxY - minY;
    for (auto& p : samples) {
        p.real_part = (p.real_part - minX) / width * 2 - 1;
        p.imaginary_part = (p.imaginary_part - minY) / height * 2 - 1;
    }

    for (auto& p : samples) {
        p.real_part *= GRAPH_W / 2.0f - 10;
        p.imaginary_part *= (GRAPH_H / 2.0f - 10) * -1;
    }

    return samples;
}
#include <SDL2/SDL.h>
#include <iostream>
#include <cmath>
#include <vector>

#include "CircularBuffer.h"
#include "Vector.h"
#include "VectorList.h"
#include "VectorGenerator.h"

#define WINDOW_W 700
#define WINDOW_H 900

#define GRAPH_W 80.0
#define GRAPH_H 100.0

constexpr float X_HALFRANGE = (GRAPH_W - 1) / 2;
constexpr float Y_HALFRANGE = (GRAPH_H - 1) / 2;
constexpr float HORIZONTAL_NOTCH_HALFLENGTH = (GRAPH_W / (2 * 40));
constexpr float VERTICAL_NOTCH_HALFLENGTH = (GRAPH_H / (2 * 40));
constexpr float WIN_TO_GRAPH_W_FACTOR = WINDOW_W / GRAPH_W; 
constexpr float WIN_TO_GRAPH_H_FACTOR = WINDOW_H / GRAPH_H;

#define TRAIL_TIME 30
#define TRAIL_INTERVAL 0.005

constexpr size_t TRAIL_LENGTH = TRAIL_TIME / TRAIL_INTERVAL;
constexpr float OPACITY_DIFF = SDL_ALPHA_OPAQUE / (float) TRAIL_LENGTH;

constexpr inline int graph_to_window_x(float x) {
    return round( (x + (GRAPH_W / 2) ) * (WIN_TO_GRAPH_W_FACTOR) );
}

constexpr inline int graph_to_window_y(float y) {
    return WINDOW_H - round( (y + (GRAPH_H / 2) ) * (WIN_TO_GRAPH_H_FACTOR) );
}

void draw_background(SDL_Renderer*& renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
}

void draw_axes(SDL_Renderer*& renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);

    // x-axis
    SDL_RenderDrawLine(renderer, graph_to_window_x(-X_HALFRANGE), graph_to_window_y(0.0), graph_to_window_x(X_HALFRANGE), graph_to_window_y(0.0));

    //y-axis
    SDL_RenderDrawLine(renderer, graph_to_window_x(0.0), graph_to_window_y(Y_HALFRANGE), graph_to_window_x(0.0), graph_to_window_y(-Y_HALFRANGE));

    // axis notches
    SDL_RenderDrawLine(renderer, graph_to_window_x(-X_HALFRANGE), graph_to_window_y(VERTICAL_NOTCH_HALFLENGTH), graph_to_window_x(-X_HALFRANGE), graph_to_window_y(-VERTICAL_NOTCH_HALFLENGTH));
    SDL_RenderDrawLine(renderer, graph_to_window_x(X_HALFRANGE), graph_to_window_y(VERTICAL_NOTCH_HALFLENGTH), graph_to_window_x(X_HALFRANGE), graph_to_window_y(-VERTICAL_NOTCH_HALFLENGTH));
    SDL_RenderDrawLine(renderer, graph_to_window_x(-HORIZONTAL_NOTCH_HALFLENGTH), graph_to_window_y(Y_HALFRANGE), graph_to_window_x(HORIZONTAL_NOTCH_HALFLENGTH), graph_to_window_y(Y_HALFRANGE));
    SDL_RenderDrawLine(renderer, graph_to_window_x(-HORIZONTAL_NOTCH_HALFLENGTH), graph_to_window_y(-Y_HALFRANGE), graph_to_window_x(HORIZONTAL_NOTCH_HALFLENGTH), graph_to_window_y(-Y_HALFRANGE));
}

bool startup(SDL_Window*& window, SDL_Renderer*& renderer) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
        return false;
    }

    window = SDL_CreateWindow("Fourier Drawer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN);
    if (NULL == window)
        return false;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (NULL == renderer)
        return false;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    return true;
}

void draw_arrow(SDL_Renderer*& renderer, const ComplexPoint& p1, const ComplexPoint& p2) {
    SDL_RenderDrawLine(
        renderer,
        graph_to_window_x(p1.real_part),
        graph_to_window_y(p1.imaginary_part),
        graph_to_window_x(p2.real_part),
        graph_to_window_y(p2.imaginary_part)
    );

    // Drawing the arrow notches
    float dr = p2.real_part - p1.real_part;
    float di = p2.imaginary_part - p1.imaginary_part;
    float amp = sqrt(dr*dr + di*di);

    if (amp < 0.001)
        return;

    dr /= amp;
    di /= amp;

    float arrow_len = amp / 10.0f;
    float angle = 25.0f * M_PI / 180.0f;

    float lr = cos(angle) * dr - sin(angle) * di;
    float li = sin(angle) * dr + cos(angle) * di;

    float rr = cos(-angle) * dr - sin(-angle) * di;
    float ri = sin(-angle) * dr + cos(-angle) * di;

    float lnr = p2.real_part - arrow_len * lr;
    float lni = p2.imaginary_part - arrow_len * li;

    float rnr = p2.real_part - arrow_len * rr;
    float rni = p2.imaginary_part - arrow_len * ri;

    SDL_Point arrowhead[] = {
        SDL_Point{ graph_to_window_x(p2.real_part), graph_to_window_y(p2.imaginary_part) },
        SDL_Point{ graph_to_window_x(lnr), graph_to_window_y(lni) },
        SDL_Point{ graph_to_window_x(rnr), graph_to_window_y(rni) },
        SDL_Point{ graph_to_window_x(p2.real_part), graph_to_window_y(p2.imaginary_part) }
    };

    SDL_RenderDrawLines(renderer, arrowhead, 4);

}

void draw_vectors(SDL_Renderer*& renderer, const VectorList& vectors, float t) {
    ComplexPoint accumulated_point{ 0.0, 0.0 };

    for (const Vector& vec : vectors) {
        ComplexPoint new_point = accumulated_point + vec(t);
    
        draw_arrow(renderer, accumulated_point, new_point);
        accumulated_point = new_point;
    }
}

void mainloop(SDL_Window*& window, SDL_Renderer*& renderer) {

    VectorList vector_list;
    VectorGenerator::GenerateVectors(vector_list);

    float last_trail_time = SDL_GetTicks() / 1000.0;
    ComplexPoint first_point{ vector_list(last_trail_time) };
    CircularBuffer trail = CircularBuffer<ComplexPoint, TRAIL_LENGTH>(first_point);

    bool running = true;
    while (running) {

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        draw_background(renderer);
        draw_axes(renderer);

        float t = SDL_GetTicks() / 1000.0;

        ComplexPoint current_point{ vector_list(t) };

        if ( (t - last_trail_time) > TRAIL_INTERVAL )
        {
            trail.push(current_point);
            last_trail_time = t;
        }

        draw_vectors(renderer, vector_list, t);

        // draw trail
        float opacity = 0;

        ComplexPoint last_point = *trail.begin();
        for (const ComplexPoint& point : trail) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, (int) opacity);
            SDL_RenderDrawLine(
                renderer,
                graph_to_window_x(last_point.real_part),
                graph_to_window_y(last_point.imaginary_part),
                graph_to_window_x(point.real_part),
                graph_to_window_y(point.imaginary_part)
            );
            last_point = point;
            opacity += OPACITY_DIFF;
        }

        SDL_RenderPresent(renderer);
    }
}

void teardown(SDL_Window*& window, SDL_Renderer*& renderer) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main() {
    SDL_Window* window;
    SDL_Renderer* renderer;

    if (!startup(window, renderer))
        return 1;

    mainloop(window, renderer);

    teardown(window, renderer);

    return 0;
}

#pragma once

struct FluidCube {
    int size;
    float dt;
    float diff;
    float visc;

    float* s;
    float* density;

    float* Vx;
    float* Vy;
    float* Vz;

    float* Vx0;
    float* Vy0;
    float* Vz0;
}; typedef struct FluidCube FluidCube;

FluidCube* FluidCubeCreate(int size, float diffusion, float viscosity, float dt);
void FluidCubeFree(FluidCube* cube);
void FluidCubeStep(FluidCube* cube);


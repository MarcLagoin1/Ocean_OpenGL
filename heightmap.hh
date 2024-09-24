#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#include <complex>
#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>
#include <limits>
#include <ctime>
#include <random>
#include <map>
#include <string>
#include <iomanip>

#define M_PI 3.14159265358979323846

constexpr int RESOLUTION = 128;

struct Vector2
{
    double x, y;

    Vector2(double _x, double _y) : x(_x), y(_y) {}
};

typedef std::complex<double> Complex;
typedef std::vector<Complex> CVector;
typedef std::vector<std::vector<Complex>> CMatrix;

CMatrix make_heightmap(int nb_img, int iter = 0);
float heightmap_value(const float x, const float z, CMatrix heightmap);
void GenerateSpectra(std::vector<Complex> &spectrum0, std::vector<float> &angularSpeeds);
void UpdateHeights(float t, std::vector<Complex> &spectrum0, std::vector<Complex> &spectrum, std::vector<Complex> &choppinesses, std::vector<Complex> &choppinessDisplacements, std::vector<float> &heights, std::vector<float> &angularSpeeds);
void normalizeHeightMap(CMatrix &heightMap);

#endif // HEIGHTMAP_H

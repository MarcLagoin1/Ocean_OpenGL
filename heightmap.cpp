#include <complex>
#include <vector>
#include <math.h>
#include <random>
#include <iostream>
#include <fstream>
#include <iomanip>

typedef std::complex<double> Complex;
typedef std::vector<Complex> CVector;
typedef std::vector<std::vector<Complex>> CMatrix;

constexpr float WIND_SPEED_MIN = 2.0;
constexpr float WIND_SPEED_MAX = 12.0;
constexpr float PATCH_SIZE_MIN = 64.0;
constexpr float PATCH_SIZE_MAX = 256.0;
constexpr float CHOPPINESS_MIN = 0.5;
constexpr float CHOPPINESS_MAX = 1.5;
constexpr int RESOLUTION = 128;

constexpr float GRAVITY = 9.81f;
constexpr float WIND_SPEED = 12.4956;
constexpr float PATCH_SIZE = 128;
constexpr float CHOPPINESS = 1.01701;

struct Vector2
{
    double x, y;

    Vector2(double _x, double _y) : x(_x), y(_y) {}

    Vector2 operator*(double a) const { return Vector2(x * a, y * a); }
    double dot(const Vector2 &other) const { return x * other.x + y * other.y; }
    double magnitude() const { return sqrt(x * x + y * y); }

    Vector2 normalize() const
    {
        double mag = magnitude();
        return Vector2(x / mag, y / mag);
    }
};

Vector2 WIND_DIRECTION = Vector2(-1, -1).normalize();

float RandomGaussian()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> dist(0.0f, 1.0f);
    return dist(gen);
}

Complex ExpI(float theta)
{
    return Complex(cos(theta), sin(theta));
}

float PhillipsSpectrumCoefs(const Vector2 &k)
{
    float L = WIND_SPEED * WIND_SPEED / GRAVITY;
    float l = L / 300.0f;

    float kDotw = k.dot(WIND_DIRECTION);
    float k2 = k.dot(k);
    if (k2 < 0.000001f)
        return 0;

    float phillips = expf(-1.f / (k2 * L * L)) / (k2 * k2 * k2) * (kDotw * kDotw);
    // La vague se déplace dans le sens contraire du vent
    if (kDotw < 0)
        phillips *= 0.01f;

    return phillips * expf(-k2 * l * l);
}

void GenerateSpectra(std::vector<Complex> &spectrum0, std::vector<float> &angularSpeeds)
{
    for (int i = 0; i < RESOLUTION; i++)
    {
        for (int j = 0; j < RESOLUTION; j++)
        {
            Vector2 k = Vector2(M_PI / PATCH_SIZE * (RESOLUTION - 2 * i), M_PI / PATCH_SIZE * (RESOLUTION - 2 * j));
            float p = sqrt(PhillipsSpectrumCoefs(k) / 2);

            int index = i * RESOLUTION + j;
            spectrum0[index] = Complex(RandomGaussian() * p, RandomGaussian() * p);
            angularSpeeds[index] = sqrt(GRAVITY * k.magnitude());
        }
    }
}

void inverseFastFourierTransform(CVector &x)
{
    const int n = x.size();
    if (n <= 1)
    {
        return;
    }

    CVector even(n / 2);
    CVector odd(n / 2);
    for (int i = 0; 2 * i < n; ++i)
    {
        even[i] = x[2 * i];
        odd[i] = x[2 * i + 1];
    }

    inverseFastFourierTransform(even);
    inverseFastFourierTransform(odd);

    for (int k = 0; k < n / 2; ++k)
    {
        const double angle = 2.0 * M_PI * k / n;
        const std::complex<double> t = std::polar(1.0, angle) * odd[k];
        x[k] = even[k] + t;
        x[k + n / 2] = even[k] - t;
    }
}

void InverseFourierTransform2D(CMatrix &matrix)
{
    const int rows = matrix.size();
    const int cols = matrix[0].size();

    // Inverse transform along the columns
    for (int i = 0; i < cols; ++i)
    {
        CVector column(rows);
        for (int j = 0; j < rows; ++j)
        {
            column[j] = matrix[j][i];
        }
        inverseFastFourierTransform(column);
        for (int j = 0; j < rows; ++j)
        {
            matrix[j][i] = column[j];
        }
    }

    // Inverse transform along the rows
    for (int i = 0; i < rows; ++i)
    {
        CVector row(cols);
        for (int j = 0; j < cols; ++j)
        {
            row[j] = matrix[i][j];
        }
        inverseFastFourierTransform(row);
        for (int j = 0; j < cols; ++j)
        {
            matrix[i][j] = std::real(row[j]) / (rows * cols);
        }
    }
}

void UpdateHeights(float t, std::vector<Complex> &spectrum0, std::vector<Complex> &spectrum, std::vector<Complex> &choppinesses, std::vector<Complex> &choppinessDisplacements, std::vector<float> &heights, std::vector<float> &angularSpeeds)
{
    CMatrix spectrumMatrix(RESOLUTION, std::vector<Complex>(RESOLUTION));
    CMatrix choppinessMatrix(RESOLUTION, std::vector<Complex>(RESOLUTION));

    for (int x = 0; x < RESOLUTION; x++)
    {
        for (int y = 0; y < RESOLUTION; y++)
        {
            int i = y + x * RESOLUTION;
            float wt = angularSpeeds[i] * t;
            Complex h = spectrum0[i];
            Complex h1;
            if (y == 0 && x == 0)
                h1 = spectrum0[RESOLUTION * RESOLUTION - 1];
            else if (y == 0)
                h1 = spectrum0[RESOLUTION - 1 + (RESOLUTION - x) * RESOLUTION];
            else if (x == 0)
                h1 = spectrum0[RESOLUTION - y + (RESOLUTION - x - 1) * RESOLUTION];
            else
                h1 = spectrum0[(RESOLUTION - y) + (RESOLUTION - x) * RESOLUTION];

            Vector2 k = Vector2(RESOLUTION * .5f - x, RESOLUTION * .5f - y).normalize();
            Complex spec = h * ExpI(wt) + std::conj(h1) * ExpI(-wt);
            spectrumMatrix[x][y] = spec;
            choppinessMatrix[x][y] = Complex(k.y, -k.x) * spec;
        }
    }

    InverseFourierTransform2D(spectrumMatrix);
    InverseFourierTransform2D(choppinessMatrix);

    for (int i = 0; i < RESOLUTION; i++)
    {
        for (int j = 0; j < RESOLUTION; j++)
        {
            float sign = ((i + j) % 2) ? -1 : 1;
            int index = i * RESOLUTION + j;
            heights[index] = sign * spectrumMatrix[i][j].real();
            choppinessDisplacements[index] = Complex(sign * choppinessMatrix[i][j].real(), sign * choppinessMatrix[i][j].imag());
        }
    }
}

void normalizeHeightMap(CMatrix &heightMap)
{
    double min = std::numeric_limits<double>::max();
    double max = std::numeric_limits<double>::lowest();

    const int size = heightMap.size();
    for (int i = 0; i < size; ++i)
    {
        for (int j = 0; j < size; ++j)
        {
            const double value = std::abs(heightMap[i][j].real());
            if (value < min)
                min = value;
            if (value > max)
                max = value;
        }
    }

    const double range = max - min;
    const double newMin = 0.25;
    const double newMax = 0.75;
    const double newRange = newMax - newMin;

    for (int i = 0; i < size; ++i)
    {
        for (int j = 0; j < size; ++j)
        {
            const double value = std::abs(heightMap[i][j].real());
            const double normalizedValue = ((value - min) / range) * newRange + newMin;
            heightMap[i][j] = Complex(normalizedValue, 0.0);
        }
    }
}

void writePPM(const std::string &filename, const CMatrix &heightMap)
{
    const int size = heightMap.size();
    std::ofstream file(filename, std::ios::binary);

    // En-tête du fichier PPM
    file << "P6\n"
         << size << " " << size << "\n255\n";

    // Données de l'image
    for (int i = size - 1; i >= 0; --i)
    {
        for (int j = 0; j < size; ++j)
        {
            const unsigned char pixel = static_cast<unsigned char>(std::abs(heightMap[i][j].real()) * 255.0);
            file.put(pixel).put(pixel).put(pixel);
        }
    }

    std::cout << "Image PPM enregistrée : " << filename << std::endl;
}

CMatrix make_heightmap(int nb_img, int iter = 0)
{
    std::vector<Complex> spectrum0(RESOLUTION * RESOLUTION);
    std::vector<Complex> spectrum(RESOLUTION * RESOLUTION);
    std::vector<Complex> choppinesses(RESOLUTION * RESOLUTION);
    std::vector<Complex> choppinessDisplacements(RESOLUTION * RESOLUTION);
    std::vector<float> heights(RESOLUTION * RESOLUTION);
    std::vector<float> angularSpeeds(RESOLUTION * RESOLUTION);
    CMatrix heightMap(RESOLUTION, std::vector<Complex>(RESOLUTION));
    // Génère le spectre initial
    GenerateSpectra(spectrum0, angularSpeeds);

    // Génère les images pour chaque instant de temps
    for (int i = 0; i < nb_img; ++i)
    {
        // Met à jour les hauteurs après un certain temps
        float t = i * 0.1f;
        UpdateHeights(t, spectrum0, spectrum, choppinesses, choppinessDisplacements, heights, angularSpeeds);

        // Convertir les hauteurs en une matrice 2D
        for (int i = 0; i < RESOLUTION; ++i)
        {
            for (int j = 0; j < RESOLUTION; ++j)
            {
                heightMap[i][j] = Complex(heights[i * RESOLUTION + j], 0.0);
            }
        }

        normalizeHeightMap(heightMap);
        // Enregistrer l'image
        std::ostringstream filenameStream;
        if (nb_img == 1)
            filenameStream << "heightmap_" << std::setfill('0') << std::setw(4) << static_cast<int>(iter) << ".ppm";
        else
            filenameStream << "heightmap_" << std::setfill('0') << std::setw(4) << static_cast<int>(i) << ".ppm";
        std::string filename = filenameStream.str();
        writePPM(filename, heightMap);
    }

    return heightMap;
}
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <complex>
#include "heightmap.hh"
#include "Camera.hh"

typedef std::complex<double> Complex;
typedef std::vector<Complex> CVector;
typedef std::vector<std::vector<Complex>> CMatrix;

Camera camera(0.0f, 0.0f, 30.0f);
glm::vec3 sunPosition = glm::vec3(60.0f, 100.0f, 100.0f);
GLfloat sunIntensity = 1.0f;
glm::vec3 sunColor = glm::vec3(1.0f, 1.0f, 0.8f);

int frameCount = 0;   // Compteur d'images affichées
int currentTime = 0;  // Temps écoulé depuis le démarrage en millisecondes
int previousTime = 0; // Temps précédent en millisecondes
float fps = 0.0f;     // FPS (images par seconde)

std::vector<Complex> spectrum0(RESOLUTION *RESOLUTION);
std::vector<Complex> spectrum(RESOLUTION *RESOLUTION);
std::vector<Complex> choppinesses(RESOLUTION *RESOLUTION);
std::vector<Complex> choppinessDisplacements(RESOLUTION *RESOLUTION);
std::vector<float> heights(RESOLUTION *RESOLUTION);
std::vector<float> angularSpeeds(RESOLUTION *RESOLUTION);
CMatrix heightMap(RESOLUTION, std::vector<Complex>(RESOLUTION));

float t = 0.0f;

GLuint program_id;

void setupCamera()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, RESOLUTION, 0, RESOLUTION, -200, 200);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void setupLighting()
{
    glEnable(GL_LIGHTING); // Active l'éclairage
    glEnable(GL_LIGHT0);   // Active la première source de lumière (le soleil)
}

void setupSunLight()
{
    glm::vec4 direction = glm::vec4(-sunPosition, 0.0f);
    glLightfv(GL_LIGHT0, GL_POSITION, glm::value_ptr(direction));

    glm::vec4 diffuse = glm::vec4(sunColor * sunIntensity, 1.0f);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, glm::value_ptr(diffuse));

    glm::vec4 ambient = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
    glLightfv(GL_LIGHT0, GL_AMBIENT, glm::value_ptr(ambient));

    glm::vec4 specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    glLightfv(GL_LIGHT0, GL_SPECULAR, glm::value_ptr(specular));
}

void setupOceanMaterial()
{
    glm::vec4 ambient = glm::vec4(0.0f, 0.1f, 0.3f, 1.0f);
    glm::vec4 diffuse = glm::vec4(0.0f, 0.5f, 1.0f, 1.0f);
    glm::vec4 specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    GLfloat shininess = 32.0f;

    glMaterialfv(GL_FRONT, GL_AMBIENT, glm::value_ptr(ambient));
    glMaterialfv(GL_FRONT, GL_DIFFUSE, glm::value_ptr(diffuse));
    glMaterialfv(GL_FRONT, GL_SPECULAR, glm::value_ptr(specular));
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}

std::vector<GLfloat> convertToVertices(const CMatrix &heightmap)
{
    std::vector<GLfloat> vertices;
    for (size_t i = 0; i < heightmap.size(); ++i)
    {
        for (size_t j = 0; j < heightmap[i].size(); ++j)
        {
            vertices.push_back(i);                           // x
            vertices.push_back(heightmap[i][j].real() * 20); // y (hauteur)
            vertices.push_back(j);                           // z
        }
    }
    return vertices;
}

void drawVertices(const std::vector<GLfloat> &vertices)
{
    glBegin(GL_POINTS);
    for (size_t i = 0; i < vertices.size(); i += 3)
    {
        GLfloat x = vertices[i];
        GLfloat y = vertices[i + 1];
        GLfloat z = vertices[i + 2];

        // Définir les couleurs pour les différentes plages de hauteurs
        GLfloat minHeight = 0.0f;
        GLfloat maxHeight = 10;
        GLfloat darkColor[3] = {0.0f, 0.0f, 1.0f};
        GLfloat lightColor[3] = {0.0f, 0.0f, 0.2f};

        // Calculer la valeur normalisée de la hauteur
        GLfloat normalizedHeight = (y - minHeight) / (maxHeight - minHeight);

        // Interpoler linéairement entre les couleurs définies
        GLfloat color[3];
        for (int j = 0; j < 3; ++j)
        {
            color[j] = darkColor[j] + normalizedHeight * (lightColor[j] - darkColor[j]);
        }

        glColor3fv(color);
        glVertex3f(x, y, z);
    }
    glEnd();
}

std::vector<GLuint> generateIndices(const CMatrix &heightmap, size_t width, size_t height)
{
    std::vector<GLuint> indices;
    for (size_t i = 0; i < height - 1; ++i)
    {
        for (size_t j = 0; j < width - 1; ++j)
        {
            // Triangle 1
            indices.push_back(i * width + j);
            indices.push_back((i + 1) * width + j);
            indices.push_back(i * width + j + 1);

            // Triangle 2
            indices.push_back((i + 1) * width + j);
            indices.push_back((i + 1) * width + j + 1);
            indices.push_back(i * width + j + 1);
        }
    }
    return indices;
}

void drawTriangles(const std::vector<GLfloat> &vertices, const std::vector<GLuint> &indices)
{
    // Définir les couleurs pour les différentes plages de hauteurs
    GLfloat minHeight = 0.0f;
    GLfloat maxHeight = 15.0f;
    GLfloat darkColor[3] = {0.0f, 0.0f, 0.08f};
    GLfloat lightColor[3] = {0.0f, 0.0f, 0.8f};

    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < indices.size(); ++i)
    {
        size_t index = indices[i];
        GLfloat y = vertices[index * 3 + 1]; // y (hauteur)

        // Calculer la valeur normalisée de la hauteur
        GLfloat normalizedHeight = (y - minHeight) / (maxHeight - minHeight);

        // Interpoler linéairement entre les couleurs définies
        GLfloat color[3];
        for (int j = 0; j < 3; ++j)
        {
            color[j] = darkColor[j] + normalizedHeight * (lightColor[j] - darkColor[j]);
        }

        glColor3fv(color);
        glVertex3f(vertices[index * 3], y, vertices[index * 3 + 2]);
    }
    glEnd();
}

void keyboard(unsigned char key, int x, int y)
{
    const float movementSpeed = 0.1f; // Vitesse de déplacement de la caméra
    float angleIncrement = 0.1f;

    float zoomIncrement = 1.0f;

    switch (key)
    {
    case 'z':
        camera.moveUp(movementSpeed); // Déplacer la caméra vers le haut
        break;
    case 's':
        camera.moveDown(movementSpeed); // Déplacer la caméra vers le bas
        break;
    case 'q':
        camera.moveLeft(movementSpeed);
        break;
    case 'd':
        camera.moveRight(movementSpeed);
        break;
    case 'a':
        camera.rotate(0.0f, -angleIncrement, 0.0f); // Incliner la caméra vers le haut
        break;
    case 'e':
        camera.rotate(0.0f, angleIncrement, 0.0f); // Incliner la caméra vers le bas
        break;
    case 'r':
        camera.rotate(0.0f, 0.0f, angleIncrement); // Faire pivoter la caméra vers la droite
        break;
    case 'f':
        camera.rotate(0.0f, 0.0f, -angleIncrement); // Faire pivoter la caméra vers la gauche
        break;
    case '+':
        camera.zoomIn(zoomIncrement); // Zoomer la caméra en réduisant le facteur de zoom
        break;
    case '-':
        camera.zoomOut(zoomIncrement); // Dézoomer la caméra en augmentant le facteur de zoom
        break;
    }

    // Demandez à GLUT de redessiner la fenêtre
    glutPostRedisplay();
}

void update()
{
    // Mettez à jour les paramètres nécessaires pour la scène
    UpdateHeights(t, spectrum0, spectrum, choppinesses, choppinessDisplacements, heights, angularSpeeds);
    t += 0.1;

    // Convertir les hauteurs en une matrice 2D
    for (int i = 0; i < RESOLUTION; ++i)
    {
        for (int j = 0; j < RESOLUTION; ++j)
        {
            heightMap[i][j] = Complex(heights[i * RESOLUTION + j], 0.0);
        }
    }

    normalizeHeightMap(heightMap);

    // Demandez à GLUT de redessiner la fenêtre
    glutPostRedisplay();
}

void drawSun()
{
    glPushMatrix();
    glTranslatef(sunPosition.x, sunPosition.y, sunPosition.z);
    glColor3f(sunColor.r, sunColor.g, sunColor.b); // Définit la couleur du soleil
    glutSolidSphere(3.0, 100, 100);                // Dessine une sphère de rayon 10 et de précision 100
    glPopMatrix();
}

void display()
{
    glUseProgram(program_id);
    // Effacez le tampon de couleur et de profondeur
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_POINT_SMOOTH);

    glDisable(GL_DEPTH_TEST);

    camera.update();

    // Dessinez la scène
    std::vector<GLfloat> vertices = convertToVertices(heightMap);
    std::vector<GLuint> indices = generateIndices(heightMap, RESOLUTION, RESOLUTION);

    // glEnable(GL_LIGHTING);
    // glEnable(GL_LIGHT0);

    // glm::vec4 direction = glm::vec4(-sunPosition, 0.0f); // La direction est l'inverse de la position du soleil
    // glLightfv(GL_LIGHT0, GL_POSITION, glm::value_ptr(direction));

    // glm::vec4 diffuse = glm::vec4(sunColor * sunIntensity, 1.0f);
    // glLightfv(GL_LIGHT0, GL_DIFFUSE, glm::value_ptr(diffuse));

    // setupSunLight();

    // setupOceanMaterial();

    // drawSun();

    drawVertices(vertices);
    drawTriangles(vertices, indices);

    // Échangez les tampons avant et arrière
    glutSwapBuffers();

    // Comptez le nombre d'images affichées
    frameCount++;

    // Calculez le FPS
    currentTime = glutGet(GLUT_ELAPSED_TIME);
    float deltaTime = currentTime - previousTime;
    if (deltaTime > 1000) // Une seconde s'est écoulée
    {
        fps = frameCount / (deltaTime / 1000.0f);
        std::cout << "FPS: " << fps << std::endl;

        frameCount = 0;
        previousTime = currentTime;
    }
}

void init_glut(int &argc, char *argv[])
{
    // Initialisez GLUT et créez une fenêtre
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1024, 1024);
    glutCreateWindow("Ocean Simulator");

}

int main(int argc, char **argv)
{
    init_glut(argc, argv);
    // Initialisez GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Définir la fonction de rappel d'affichage
    GenerateSpectra(spectrum0, angularSpeeds);
    glutDisplayFunc(display);

    camera.init();
    setupCamera();

    glutKeyboardFunc(keyboard);

    // Définir la fonction de rappel d'inactivité
    glutIdleFunc(update);

    // Boucle principale de rendu
    glutMainLoop();

    return 0;
}
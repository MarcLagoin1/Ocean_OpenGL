#include "Camera.hh"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath>

Camera::Camera(float x, float y, float z)
    : posX(x), posY(y), posZ(z), yaw(0.0f), pitch(0.0f), zoomFactor(1.0f)
{
}

void Camera::init()
{
    // Initialisez la caméra avec un angle de vue légèrement vers le bas
    pitch = 20.0f;
}

void Camera::update()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(-pitch, 1.0f, 0.0f, 0.0f); // Inverser la rotation pitch
    glRotatef(-yaw, 0.0f, 1.0f, 0.0f);   // Inverser la rotation yaw
    glRotatef(-roll, 0.0f, 0.0f, 1.0f);  // Inverser la rotation roll
    glTranslatef(-posX, -posY, -posZ);
}

void Camera::moveUp(float distance)
{
    posY += distance;
}

void Camera::moveDown(float distance)
{
    posY -= distance;
}

void Camera::rotate(float x, float y, float z)
{
    yaw += x;
    pitch += y;
    roll += z; // Nouveau

    // Limitez l'angle de rotation vertical entre -90 et 90 degrés
    if (pitch > 90.0f)
        pitch = 90.0f;
    if (pitch < -90.0f)
        pitch = -90.0f;
}

void Camera::moveForward(float distance)
{
    float radianYaw = yaw * (3.14159f / 180.0f);
    posX += distance * sinf(radianYaw);
    posZ -= distance * cosf(radianYaw);
}

void Camera::moveBackward(float distance)
{
    float radianYaw = yaw * (3.14159f / 180.0f);
    posX -= distance * sinf(radianYaw);
    posZ += distance * cosf(radianYaw);
}

void Camera::moveLeft(float distance)
{
    float radianYaw = yaw * (3.14159f / 180.0f);
    posX -= distance * zoomFactor * cosf(radianYaw);
    posZ -= distance * zoomFactor * sinf(radianYaw);
}

void Camera::moveRight(float distance)
{
    float radianYaw = yaw * (3.14159f / 180.0f);
    posX += distance * zoomFactor * cosf(radianYaw);
    posZ += distance * zoomFactor * sinf(radianYaw);
}

void Camera::rotate(float x, float y)
{
    yaw += x;
    pitch += y;

    // Limitez l'angle de rotation vertical entre -90 et 90 degrés
    if (pitch > 90.0f)
        pitch = 90.0f;
    if (pitch < -90.0f)
        pitch = -90.0f;
}

float Camera::getX() const { return posX; }
float Camera::getY() const { return posY; }
float Camera::getZ() const { return posZ; }

void Camera::zoomIn(float factor)
{
    zoomFactor *= factor;
}

void Camera::zoomOut(float factor)
{
    zoomFactor /= factor;
}
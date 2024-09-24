class Camera
{
private:
    float posX;       // Position X de la caméra
    float posY;       // Position Y de la caméra
    float posZ;       // Position Z de la caméra
    float yaw;        // Angle de rotation horizontal (yaw)
    float pitch;      // Angle de rotation vertical (pitch)
    float roll;       // Angle de rotation latéral (roll)
    float zoomFactor; // Facteur de zoom

public:
    Camera(float x, float y, float z); // Constructeur
    void update();                     // Mettre à jour les transformations de la caméra
    void moveForward(float distance);  // Déplacer la caméra vers l'avant
    void moveBackward(float distance); // Déplacer la caméra vers l'arrière
    void moveLeft(float distance);     // Déplacer la caméra vers la gauche
    void moveRight(float distance);    // Déplacer la caméra vers la droite
    void moveUp(float distance);
    void moveDown(float distance);
    void rotate(float x, float y, float z);
    void rotate(float x, float y); // Faire pivoter la caméra
    void zoomIn(float factor);     // Zoomer la caméra
    void zoomOut(float factor);    // Dézoomer la caméra
    void init();
    float getX() const;
    float getY() const;
    float getZ() const;
};

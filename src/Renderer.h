#pragma once

#include "Collectible.h"
#include "Enemy.h"
#include "Player.h"
#include "Projectile.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vector>

class Renderer {
public:
    bool initialize();
    void shutdown();
    void beginFrame();
    void present(GLFWwindow* window);

    void drawWorld(std::vector<Player>& players, const std::vector<Enemy>& enemies,
                   const std::vector<Projectile>& enemyProjectiles,
                   const std::vector<Projectile>& playerProjectiles,
                   const std::vector<Collectible>& collectibles,
                   const bool (&selectedLanes)[3], int hoveredLane,
                   bool gameOver);
    void drawCutscene(const std::vector<Player>& heroes, const std::vector<Enemy>& enemies,
                      float elapsed, bool heshamAttacking, bool fahmyAiming,
                      float shikoShieldRaise, float bossWeaponRotation);
    void drawPlayerCharacter(float x, float y, PlayerType type, int health,
                             float jumpOffset = 0.0f, bool ducking = false,
                             bool attacking = false, float scale = 1.0f,
                             bool grayscale = false, float shieldRaise = 0.0f);
    void drawHealthBar(float x, float y, int health, int maxHealth, glm::vec3 color);
    void drawRectangle(float x, float y, float width, float height, glm::vec3 color);
    void drawCircle(float x, float y, float width, float height, glm::vec3 color);

private:
    struct Vertex { float x, y, r, g, b; };
    struct Shape { GLint first = 0; GLsizei count = 0; GLenum mode = GL_TRIANGLES; };

    GLuint compileShader(GLenum type, const char* source);
    GLuint createProgram();
    void addTriangle(std::vector<Vertex>& vertices, float x1, float y1,
                     float x2, float y2, float x3, float y3, glm::vec3 color);
    void createGeometry();
    void drawShape(const Shape& shape, float x, float y, float sx, float sy,
                   glm::vec3 color, float rotation = 0.0f);
    void drawEnemyPart(const Shape& shape, float centerX, float centerY,
                       float offsetX, float offsetY, float width, float height,
                       glm::vec3 color, float rotation = 0.0f);
    void drawEnemyCharacter(float x, float y, const Enemy& enemy,
                            float hardWeaponRotation = -0.25f);
    void drawTreasureMap(float x, float y);
    void drawPlayerPart(const Shape& shape, float x, float y, float offsetX,
                        float offsetY, float sx, float sy, glm::vec3 color,
                        float rotation, float jumpOffset, bool ducking);
    void drawPalmTree(float x, float groundY);
    void drawTorch(float x, float y);
    void drawEnvironment(const bool (&selectedLanes)[3], int hoveredLane);

    GLuint shaderProgram_ = 0;
    GLint transformLocation_ = -1;
    GLint damageTintLocation_ = -1;
    float currentDamageTint_ = 0.0f;
    float playerDrawingScale_ = 1.0f;
    bool grayscaleDrawing_ = false;
    float sceneScale_ = 1.0f;
    float sceneOffsetX_ = 0.0f;
    float sceneOffsetY_ = 0.0f;
    GLuint vao_ = 0, vbo_ = 0;
    Shape rectangleShape_, triangleShape_, circleShape_;
};

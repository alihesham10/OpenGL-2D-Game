// Temple Defense - OpenGL 3.3 starter project
// Built from the concepts in Graphics Labs 1-3: VAO/VBO, shaders, transforms,
// primitive shapes, GLFW input, and mouse-coordinate conversion.
// This is a foundation, not the complete assignment.

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Enemy.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>

constexpr int WINDOW_W = 1100;
constexpr int WINDOW_H = 700;
constexpr int LANE_COUNT = 3;
constexpr float LANE_HEIGHT = 0.32f;
constexpr float LANE_Y[LANE_COUNT] = {-0.125f, -0.48f, -0.835f};
constexpr float JUMP_DURATION = 0.55f;
constexpr float ENEMY_SPAWN_INTERVAL = 3.0f;

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
uniform mat4 u_Transform;
out vec3 vColor;
void main() {
    gl_Position = u_Transform * vec4(aPos, 0.0, 1.0);
    vColor = aColor;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec3 vColor;
out vec4 FragColor;
uniform vec3 u_Tint;
uniform float u_DamageTint;
void main() {
    vec3 baseColor = vColor * u_Tint;
    vec3 damageColor = mix(baseColor, vec3(0.95, 0.05, 0.04), u_DamageTint);
    FragColor = vec4(damageColor, 1.0);
}
)";

struct Vertex { float x, y, r, g, b; };
struct Shape { GLuint vao = 0, vbo = 0; GLsizei count = 0; GLenum mode = GL_TRIANGLES; };
enum class PlayerType { Melee, Defense, LongRange };
struct Player {
    PlayerType type;
    glm::vec2 pos;
    int lane;
    int health;
    bool alive;
    float jumpTimer;
    bool ducking;
};

GLuint shaderProgram = 0;
GLint transformLocation = -1;
GLint damageTintLocation = -1;
float currentDamageTint = 0.0f;
Shape rectangleShape, triangleShape, circleShape;
bool selectedLanes[LANE_COUNT] = {false, true, false};
int hoveredLane = -1;
std::vector<Player> players{
    {PlayerType::Melee, glm::vec2(-0.62f, LANE_Y[1]), 1, 5, true, 0.0f, false}
};
bool duckKeyHeld = false;
std::vector<Enemy> enemies;
float enemySpawnTimer = 0.0f;
int coins = 0;
bool paused = false;
bool gameOver = false;
float lastFrameTime = 0.0f;

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "Shader compilation failed:\n" << log << '\n';
    }
    return shader;
}

GLuint createProgram() {
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    GLint ok = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        std::cerr << "Shader link failed:\n" << log << '\n';
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

Shape makeShape(const std::vector<Vertex>& vertices, GLenum mode) {
    Shape s;
    s.count = static_cast<GLsizei>(vertices.size());
    s.mode = mode;
    glGenVertexArrays(1, &s.vao);
    glGenBuffers(1, &s.vbo);
    glBindVertexArray(s.vao);
    glBindBuffer(GL_ARRAY_BUFFER, s.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, r));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    return s;
}

void addTriangle(std::vector<Vertex>& v, float x1, float y1, float x2, float y2, float x3, float y3,
                 glm::vec3 c) {
    v.push_back({x1,y1,c.r,c.g,c.b}); v.push_back({x2,y2,c.r,c.g,c.b}); v.push_back({x3,y3,c.r,c.g,c.b});
}

void createGeometry() {
    // A unit rectangle centered on the origin.
    std::vector<Vertex> rect;
    glm::vec3 white(1.0f);
    addTriangle(rect, -0.5f,-0.5f, 0.5f,-0.5f, 0.5f,0.5f, white);
    addTriangle(rect, -0.5f,-0.5f, 0.5f,0.5f, -0.5f,0.5f, white);
    rectangleShape = makeShape(rect, GL_TRIANGLES);

    std::vector<Vertex> tri;
    addTriangle(tri, 0.0f,0.5f, -0.5f,-0.5f, 0.5f,-0.5f, white);
    triangleShape = makeShape(tri, GL_TRIANGLES);

    std::vector<Vertex> circle;
    constexpr int segments = 36;
    for (int i = 0; i < segments; ++i) {
        float a1 = 2.0f * 3.14159265f * i / segments;
        float a2 = 2.0f * 3.14159265f * (i + 1) / segments;
        addTriangle(circle, 0,0, std::cos(a1),std::sin(a1), std::cos(a2),std::sin(a2), white);
    }
    circleShape = makeShape(circle, GL_TRIANGLES);
}

void drawShape(const Shape& shape, float x, float y, float sx, float sy, glm::vec3 color,
               float rotation = 0.0f) {
    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(x, y, 0.0f));
    model = glm::rotate(model, rotation, glm::vec3(0,0,1));
    model = glm::scale(model, glm::vec3(sx, sy, 1.0f));
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
    // The vertex colors in the starter geometry are white, so tinting is done by
    // setting the global constant color through a tiny color-uniform-free trick:
    // update vertex colors once per shape would be wasteful. Instead this starter
    // geometry uses white vertices and a fragment tint uniform below.
    GLint tint = glGetUniformLocation(shaderProgram, "u_Tint");
    if (tint >= 0) glUniform3fv(tint, 1, &color[0]);
    if (damageTintLocation >= 0) glUniform1f(damageTintLocation, currentDamageTint);
    glBindVertexArray(shape.vao);
    glDrawArrays(shape.mode, 0, shape.count);
    glBindVertexArray(0);
}

// Convert GLFW cursor pixels to OpenGL-style normalized device coordinates.
glm::vec2 cursorToNdc(double x, double y) {
    return glm::vec2(static_cast<float>(2.0 * x / WINDOW_W - 1.0),
                     static_cast<float>(1.0 - 2.0 * y / WINDOW_H));
}

int laneFromY(float y) {
    for (int i = 0; i < LANE_COUNT; ++i) {
        float laneTop = LANE_Y[i] + LANE_HEIGHT * 0.5f;
        float laneBottom = LANE_Y[i] - LANE_HEIGHT * 0.5f;
        if (y >= laneBottom && y <= laneTop) return i;
    }
    return -1;
}

void cursorPositionCallback(GLFWwindow*, double x, double y) {
    glm::vec2 p = cursorToNdc(x, y);
    hoveredLane = laneFromY(p.y);
}

void spawnEnemy() {
    int lane = std::rand() % LANE_COUNT;
    enemies.push_back({EnemyType::EasyUpperRanged, glm::vec2(0.90f, LANE_Y[lane]),
                       lane, 0.10f, 5, true});
}

void resetEnemies() {
    enemies.clear();
    enemySpawnTimer = 0.0f;
    spawnEnemy();
}

void resetPlayers() {
    players.clear();
    players.push_back({PlayerType::Melee, glm::vec2(-0.62f, LANE_Y[1]), 1, 5, true, 0.0f, false});
    selectedLanes[0] = false;
    selectedLanes[1] = true;
    selectedLanes[2] = false;
    duckKeyHeld = false;
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int) {
    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) return;
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    glm::vec2 p = cursorToNdc(x, y);
    int lane = laneFromY(p.y);
    if (lane >= 0) selectedLanes[lane] = !selectedLanes[lane];
}

void keyCallback(GLFWwindow* window, int key, int, int action, int) {
    if (key == GLFW_KEY_S || key == GLFW_KEY_DOWN) {
        if (action == GLFW_PRESS || action == GLFW_REPEAT) duckKeyHeld = true;
        else if (action == GLFW_RELEASE) duckKeyHeld = false;
        return;
    }
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_ESCAPE) paused = !paused;
    if (key == GLFW_KEY_R) {
        resetPlayers();
        resetEnemies();
        coins = 0; gameOver = false; paused = false;
    }
    if (key == GLFW_KEY_UP || key == GLFW_KEY_W) {
        for (Player& player : players) {
            if (player.alive && player.type != PlayerType::Defense &&
                selectedLanes[player.lane]) {
                player.jumpTimer = JUMP_DURATION;
                player.ducking = false;
            }
        }
    }
    if (key == GLFW_KEY_SPACE && gameOver) {
        resetPlayers();
        resetEnemies();
        coins = 0; gameOver = false;
    }
    (void)window;
}

int maximumHealth(EnemyType type) {
    return type == EnemyType::HardMelee ? 7 : 5;
}

int maximumHealth(PlayerType type) {
    return type == PlayerType::Defense ? 7 : 5;
}

float damageTintForHealth(int health, int maxHealth) {
    float healthRatio = static_cast<float>(health) / static_cast<float>(maxHealth);
    healthRatio = std::clamp(healthRatio, 0.0f, 1.0f);
    return 0.75f * (1.0f - healthRatio);
}

void drawHealthBar(float x, float y, int health, int maxHealth, glm::vec3 fillColor) {
    constexpr float segmentWidth = 0.018f;
    constexpr float segmentGap = 0.004f;
    constexpr float barHeight = 0.022f;
    const float totalWidth = maxHealth * segmentWidth + (maxHealth - 1) * segmentGap;
    const float firstCenter = x - totalWidth * 0.5f + segmentWidth * 0.5f;

    for (int i = 0; i < maxHealth; ++i) {
        float segmentX = firstCenter + i * (segmentWidth + segmentGap);
        drawShape(rectangleShape, segmentX, y, segmentWidth, barHeight,
                  glm::vec3(0.16f, 0.12f, 0.10f));
        if (i < health) {
            drawShape(rectangleShape, segmentX, y, segmentWidth * 0.72f, barHeight * 0.62f,
                      fillColor);
        }
    }
}

void drawEnemyCharacter(float x, float y, EnemyType type, int health) {
    currentDamageTint = damageTintForHealth(health, maximumHealth(type));
    const glm::vec3 skin(0.66f, 0.40f, 0.24f);
    const glm::vec3 darkCloth(0.20f, 0.12f, 0.11f);

    switch (type) {
    case EnemyType::EasyUpperRanged:
        // Slim hooded thief with a raised sling for attacks from above.
        drawShape(rectangleShape, x, y - 0.075f, 0.075f, 0.15f, glm::vec3(0.40f,0.18f,0.14f));
        drawShape(circleShape, x, y + 0.055f, 0.05f, 0.05f, skin);
        drawShape(triangleShape, x, y + 0.085f, 0.12f, 0.10f, darkCloth);
        drawShape(rectangleShape, x, y + 0.035f, 0.07f, 0.018f, glm::vec3(0.14f,0.10f,0.09f));
        drawShape(rectangleShape, x - 0.025f, y - 0.17f, 0.025f, 0.10f, darkCloth);
        drawShape(rectangleShape, x + 0.025f, y - 0.17f, 0.025f, 0.10f, darkCloth);
        drawShape(rectangleShape, x - 0.075f, y + 0.12f, 0.10f, 0.016f,
                  glm::vec3(0.48f,0.30f,0.15f), 0.45f);
        drawShape(circleShape, x - 0.13f, y + 0.17f, 0.028f, 0.028f,
                  glm::vec3(0.28f,0.27f,0.23f));
        break;

    case EnemyType::MediumLowerRanged:
        // A low, wide crouch with a ground-level crossbow.
        drawShape(rectangleShape, x, y - 0.085f, 0.13f, 0.12f, glm::vec3(0.34f,0.25f,0.18f));
        drawShape(circleShape, x, y + 0.025f, 0.045f, 0.045f, skin);
        drawShape(rectangleShape, x, y + 0.025f, 0.09f, 0.022f, darkCloth);
        drawShape(rectangleShape, x - 0.04f, y - 0.16f, 0.05f, 0.07f, darkCloth, -0.3f);
        drawShape(rectangleShape, x + 0.04f, y - 0.16f, 0.05f, 0.07f, darkCloth, 0.3f);
        drawShape(rectangleShape, x - 0.09f, y - 0.10f, 0.12f, 0.025f,
                  glm::vec3(0.42f,0.27f,0.12f));
        drawShape(triangleShape, x - 0.15f, y - 0.10f, 0.05f, 0.05f,
                  glm::vec3(0.58f,0.54f,0.45f), -1.57f);
        break;

    case EnemyType::HardMelee:
        // Broad armored body, horned helmet, and a heavy close-range weapon.
        drawShape(rectangleShape, x, y - 0.075f, 0.14f, 0.18f, glm::vec3(0.30f,0.22f,0.17f));
        drawShape(circleShape, x, y + 0.065f, 0.055f, 0.055f, skin);
        drawShape(rectangleShape, x, y + 0.105f, 0.13f, 0.035f, glm::vec3(0.24f,0.23f,0.20f));
        drawShape(triangleShape, x - 0.075f, y + 0.11f, 0.055f, 0.085f,
                  glm::vec3(0.24f,0.23f,0.20f), -0.35f);
        drawShape(triangleShape, x + 0.075f, y + 0.11f, 0.055f, 0.085f,
                  glm::vec3(0.24f,0.23f,0.20f), 0.35f);
        drawShape(rectangleShape, x - 0.04f, y - 0.17f, 0.04f, 0.10f, darkCloth);
        drawShape(rectangleShape, x + 0.04f, y - 0.17f, 0.04f, 0.10f, darkCloth);
        drawShape(rectangleShape, x - 0.12f, y - 0.015f, 0.14f, 0.04f,
                  glm::vec3(0.62f,0.61f,0.56f), -0.25f);
        drawShape(rectangleShape, x - 0.055f, y - 0.005f, 0.04f, 0.055f,
                  glm::vec3(0.34f,0.20f,0.11f), -0.25f);
        break;
    }
    currentDamageTint = 0.0f;
}

void drawPlayerPart(const Shape& shape, float x, float y, float offsetX, float offsetY,
                    float sx, float sy, glm::vec3 color, float rotation,
                    float jumpOffset, bool ducking) {
    const float heightScale = ducking ? 0.65f : 1.0f;
    const float poseOffset = jumpOffset - (ducking ? 0.025f : 0.0f);
    drawShape(shape, x + offsetX, y + poseOffset + offsetY * heightScale,
              sx, sy * heightScale, color, rotation);
}

void drawPlayerCharacter(float x, float y, PlayerType playerType,
                         int health, float jumpOffset = 0.0f, bool ducking = false) {
    currentDamageTint = damageTintForHealth(health, maximumHealth(playerType));
    const glm::vec3 skin(0.82f,0.60f,0.40f);
    const glm::vec3 darkCloth(0.22f, 0.18f, 0.15f);
    switch (playerType) {
    case PlayerType::Melee:
        // Light tunic, separate legs, and a short sword.
        drawPlayerPart(rectangleShape, x, y, 0, -0.075f, 0.075f, 0.15f,
                       glm::vec3(0.78f,0.68f,0.48f), 0, jumpOffset, ducking);
        drawPlayerPart(circleShape, x, y, 0, 0.065f, 0.05f, 0.05f,
                       skin, 0, jumpOffset, ducking);
        drawPlayerPart(rectangleShape, x, y, 0, 0.105f, 0.075f, 0.022f,
                       glm::vec3(0.16f,0.10f,0.06f), 0, jumpOffset, ducking);
        drawPlayerPart(rectangleShape, x, y, -0.025f, -0.17f, 0.025f, 0.10f,
                       darkCloth, 0, jumpOffset, ducking);
        drawPlayerPart(rectangleShape, x, y, 0.025f, -0.17f, 0.025f, 0.10f,
                       darkCloth, 0, jumpOffset, ducking);
        drawPlayerPart(rectangleShape, x, y, 0.075f, -0.005f, 0.105f, 0.018f,
                       glm::vec3(0.75f,0.76f,0.72f), -0.35f, jumpOffset, ducking);
        drawPlayerPart(rectangleShape, x, y, 0.035f, -0.015f, 0.035f, 0.025f,
                       glm::vec3(0.42f,0.24f,0.10f), -0.35f, jumpOffset, ducking);
        break;

    case PlayerType::Defense:
        // Broad build, helmet, and a large shield make this type read as a blocker.
        drawPlayerPart(rectangleShape, x, y, 0, -0.075f, 0.14f, 0.17f,
                       glm::vec3(0.46f,0.50f,0.43f), 0, jumpOffset, ducking);
        drawPlayerPart(circleShape, x, y, 0, 0.065f, 0.052f, 0.052f,
                       skin, 0, jumpOffset, ducking);
        drawPlayerPart(rectangleShape, x, y, 0, 0.105f, 0.11f, 0.035f,
                       glm::vec3(0.53f,0.39f,0.18f), 0, jumpOffset, ducking);
        drawPlayerPart(rectangleShape, x, y, -0.035f, -0.17f, 0.035f, 0.10f,
                       darkCloth, 0, jumpOffset, ducking);
        drawPlayerPart(rectangleShape, x, y, 0.035f, -0.17f, 0.035f, 0.10f,
                       darkCloth, 0, jumpOffset, ducking);
        drawPlayerPart(rectangleShape, x, y, 0.11f, -0.045f, 0.075f, 0.19f,
                       glm::vec3(0.40f,0.25f,0.12f), 0, jumpOffset, ducking);
        drawPlayerPart(triangleShape, x, y, 0.11f, -0.16f, 0.075f, 0.07f,
                       glm::vec3(0.40f,0.25f,0.12f), 0, jumpOffset, ducking);
        break;

    case PlayerType::LongRange:
        // Lean clothing and a long bow with an arrow distinguish the ranged type.
        drawPlayerPart(rectangleShape, x, y, 0, -0.075f, 0.065f, 0.15f,
                       glm::vec3(0.43f,0.47f,0.30f), 0, jumpOffset, ducking);
        drawPlayerPart(circleShape, x, y, 0, 0.065f, 0.048f, 0.05f,
                       skin, 0, jumpOffset, ducking);
        drawPlayerPart(triangleShape, x, y, 0, 0.09f, 0.11f, 0.10f,
                       glm::vec3(0.31f,0.34f,0.24f), 0, jumpOffset, ducking);
        drawPlayerPart(rectangleShape, x, y, -0.022f, -0.17f, 0.024f, 0.10f,
                       darkCloth, 0, jumpOffset, ducking);
        drawPlayerPart(rectangleShape, x, y, 0.022f, -0.17f, 0.024f, 0.10f,
                       darkCloth, 0, jumpOffset, ducking);
        drawPlayerPart(rectangleShape, x, y, 0.09f, -0.025f, 0.025f, 0.19f,
                       glm::vec3(0.48f,0.28f,0.12f), -0.15f, jumpOffset, ducking);
        drawPlayerPart(rectangleShape, x, y, 0.085f, -0.02f, 0.105f, 0.012f,
                       glm::vec3(0.78f,0.70f,0.45f), 0, jumpOffset, ducking);
        break;
    }
    currentDamageTint = 0.0f;
}

void drawPalmTree(float x, float groundY) {
    const glm::vec3 leafColor(0.16f, 0.42f, 0.19f);
    const float trunkHeight = 0.34f;
    const float trunkCenterY = groundY + trunkHeight * 0.5f;
    drawShape(rectangleShape, x, trunkCenterY, 0.045f, trunkHeight,
              glm::vec3(0.48f, 0.31f, 0.16f), -0.12f);
    const float crownY = groundY + trunkHeight;
    drawShape(triangleShape, x - 0.10f, crownY + 0.02f, 0.18f, 0.10f, leafColor, -1.10f);
    drawShape(triangleShape, x - 0.04f, crownY + 0.09f, 0.18f, 0.10f, leafColor, -0.55f);
    drawShape(triangleShape, x + 0.04f, crownY + 0.09f, 0.18f, 0.10f, leafColor, 0.55f);
    drawShape(triangleShape, x + 0.10f, crownY + 0.02f, 0.18f, 0.10f, leafColor, 1.10f);
}

void drawTorch(float x, float y) {
    drawShape(rectangleShape, x, y - 0.045f, 0.035f, 0.16f,
              glm::vec3(0.35f, 0.20f, 0.11f));
    drawShape(rectangleShape, x, y + 0.035f, 0.055f, 0.025f,
              glm::vec3(0.70f, 0.48f, 0.22f));
    drawShape(triangleShape, x, y + 0.09f, 0.055f, 0.09f,
              glm::vec3(1.0f, 0.42f, 0.08f));
    drawShape(triangleShape, x, y + 0.08f, 0.025f, 0.055f,
              glm::vec3(1.0f, 0.82f, 0.20f));
}

void renderScene() {
    glUseProgram(shaderProgram);
    // Sky and distant hills.
    drawShape(rectangleShape, 0, 0.55f, 2.0f, 0.90f, glm::vec3(0.42f,0.70f,0.83f));
    drawShape(triangleShape, -0.35f,0.73f, 0.72f,0.52f, glm::vec3(0.82f,0.66f,0.43f));
    drawShape(triangleShape, 0.22f,0.74f, 0.82f,0.65f, glm::vec3(0.91f,0.75f,0.48f));
    // Ground.
    drawShape(rectangleShape, 0,-0.48f, 2.0f,1.05f, glm::vec3(0.78f,0.64f,0.40f));

    // Temple/base on the left.
    drawShape(rectangleShape, -0.88f,-0.05f, 0.20f,0.70f, glm::vec3(0.77f,0.69f,0.54f));
    drawShape(triangleShape, -0.88f,0.34f, 0.28f,0.20f, glm::vec3(0.61f,0.48f,0.31f));
    for (int i = 0; i < 3; ++i)
        drawShape(rectangleShape, -0.94f + i * 0.06f,-0.09f, 0.025f,0.48f, glm::vec3(0.56f,0.46f,0.34f));
    drawShape(rectangleShape, -0.88f,-0.36f, 0.28f,0.06f, glm::vec3(0.51f,0.39f,0.26f));

    // Place the palm at the sand line, just left of the right-side torch.
    drawPalmTree(0.80f, 0.045f);
    drawTorch(0.92f, 0.18f);
    drawTorch(0.92f, -0.73f);

    // Hovered lanes brighten; selected lanes keep a warm highlight and marker.
    for (int i = 0; i < 3; ++i) {
        glm::vec3 laneColor = glm::vec3(0.73f,0.57f,0.34f);
        if (selectedLanes[i]) laneColor = glm::vec3(0.82f,0.66f,0.42f);
        if (i == hoveredLane) laneColor = glm::vec3(0.90f,0.77f,0.52f);
        drawShape(rectangleShape, 0.10f, LANE_Y[i], 1.48f, LANE_HEIGHT, laneColor);
        drawShape(rectangleShape, 0.10f, LANE_Y[i] - 0.17f, 1.48f, 0.012f, glm::vec3(0.48f,0.37f,0.24f));
        if (selectedLanes[i])
            drawShape(triangleShape, -0.68f, LANE_Y[i], 0.045f, 0.07f, glm::vec3(0.20f,0.75f,0.28f));
    }

    // Draw every living player at the lane where it was placed.
    for (Player& player : players) {
        if (!player.alive || player.lane < 0 || player.lane >= LANE_COUNT) continue;
        player.pos.y = LANE_Y[player.lane];
        float jumpProgress = 1.0f - player.jumpTimer / JUMP_DURATION;
        float jumpOffset = player.jumpTimer > 0.0f
            ? 0.20f * std::sin(jumpProgress * 3.14159265f)
            : 0.0f;
        drawPlayerCharacter(player.pos.x, player.pos.y, player.type, player.health,
                            jumpOffset, player.ducking);
        drawHealthBar(player.pos.x, player.pos.y + 0.155f + jumpOffset,
                      player.health, maximumHealth(player.type),
                      glm::vec3(0.22f, 0.76f, 0.24f));
    }
    if (!gameOver) {
        for (const Enemy& enemy : enemies) {
            if (enemy.alive) {
                float enemyY = LANE_Y[enemy.lane];
                drawEnemyCharacter(enemy.pos.x, enemyY, enemy.type, enemy.health);
                drawHealthBar(enemy.pos.x, enemyY + 0.205f, enemy.health,
                              maximumHealth(enemy.type), glm::vec3(0.88f, 0.18f, 0.12f));
            }
        }
    }

    // A coin-like collectible. (Press C to collect the demo coin.)
    drawShape(circleShape, 0.32f, -0.43f, 0.035f, 0.035f, glm::vec3(1.0f,0.78f,0.10f));
    drawShape(circleShape, 0.32f, -0.43f, 0.020f, 0.020f, glm::vec3(0.90f,0.52f,0.05f));

    // Minimal UI bars (text rendering is intentionally left for a later step).
    drawShape(rectangleShape, 0.0f,0.93f, 1.90f,0.10f, glm::vec3(0.12f,0.13f,0.15f));
    drawShape(rectangleShape, -0.65f,0.93f, 0.22f,0.045f, glm::vec3(0.80f,0.20f,0.15f));
    drawShape(rectangleShape, -0.25f,0.93f, 0.22f,0.045f, glm::vec3(0.20f,0.72f,0.30f));
    drawShape(rectangleShape, 0.22f,0.93f, 0.22f,0.045f, glm::vec3(0.95f,0.74f,0.15f));
    drawShape(rectangleShape, 0.68f,0.93f, 0.22f,0.045f, glm::vec3(0.30f,0.62f,0.92f));

    if (paused || gameOver) {
        drawShape(rectangleShape, 0,0, 0.90f,0.44f, glm::vec3(0.10f,0.11f,0.13f));
        drawShape(rectangleShape, 0,0.08f, 0.54f,0.06f, gameOver ? glm::vec3(0.78f,0.18f,0.15f) : glm::vec3(0.95f,0.75f,0.18f));
        drawShape(rectangleShape, 0,-0.08f, 0.34f,0.04f, glm::vec3(0.85f,0.85f,0.85f));
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW.\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WINDOW_W, WINDOW_H, "Temple Defense - Starter", nullptr, nullptr);
    if (!window) {
        std::cerr << "Could not create OpenGL window.\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD.\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    glViewport(0, 0, WINDOW_W, WINDOW_H);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetKeyCallback(window, keyCallback);

    shaderProgram = createProgram();
    transformLocation = glGetUniformLocation(shaderProgram, "u_Transform");
    damageTintLocation = glGetUniformLocation(shaderProgram, "u_DamageTint");
    createGeometry();
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    spawnEnemy();
    std::cout << "Controls: click lanes to toggle selection; W/Up jumps and hold S/Down to duck; Esc pauses; R resets.\n";

    while (!glfwWindowShouldClose(window)) {
        float now = static_cast<float>(glfwGetTime());
        float deltaTime = std::min(now - lastFrameTime, 0.05f);
        lastFrameTime = now;
        if (!paused && !gameOver) {
            enemySpawnTimer += deltaTime;
            if (enemySpawnTimer >= ENEMY_SPAWN_INTERVAL) {
                spawnEnemy();
                enemySpawnTimer = 0.0f;
            }
            for (Player& player : players) {
                player.jumpTimer = std::max(0.0f, player.jumpTimer - deltaTime);
                player.ducking = player.alive && player.type != PlayerType::Defense &&
                    player.jumpTimer <= 0.0f && selectedLanes[player.lane] && duckKeyHeld;
            }
            for (Enemy& enemy : enemies) {
                if (!enemy.alive) continue;
                enemy.pos.x -= enemy.speed * deltaTime;
                if (enemy.pos.x < -0.78f) {
                    gameOver = true;
                    break;
                }
            }
            if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) coins = 1;
        }

        glClearColor(0.08f,0.09f,0.12f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        renderScene();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shaderProgram);
    glDeleteVertexArrays(1, &rectangleShape.vao); glDeleteBuffers(1, &rectangleShape.vbo);
    glDeleteVertexArrays(1, &triangleShape.vao); glDeleteBuffers(1, &triangleShape.vbo);
    glDeleteVertexArrays(1, &circleShape.vao); glDeleteBuffers(1, &circleShape.vbo);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

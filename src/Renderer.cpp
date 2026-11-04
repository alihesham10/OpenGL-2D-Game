#include "Renderer.h"

#include "GameConfig.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <cstddef>
#include <iostream>
#include <vector>

namespace {
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

float damageTintForHealth(int health, int maxHealth) {
    float healthRatio = static_cast<float>(health) / static_cast<float>(maxHealth);
    healthRatio = glm::clamp(healthRatio, 0.0f, 1.0f);
    return 0.90f * (1.0f - healthRatio);
}

}

bool Renderer::initialize() {
    shaderProgram_ = createProgram();
    transformLocation_ = glGetUniformLocation(shaderProgram_, "u_Transform");
    damageTintLocation_ = glGetUniformLocation(shaderProgram_, "u_DamageTint");
    createGeometry();
    return shaderProgram_ != 0;
}

void Renderer::shutdown() {
    glDeleteProgram(shaderProgram_);
    glDeleteVertexArrays(1, &rectangleShape_.vao);
    glDeleteBuffers(1, &rectangleShape_.vbo);
    glDeleteVertexArrays(1, &triangleShape_.vao);
    glDeleteBuffers(1, &triangleShape_.vbo);
    glDeleteVertexArrays(1, &circleShape_.vao);
    glDeleteBuffers(1, &circleShape_.vbo);
}

void Renderer::beginFrame() {
    glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::present(GLFWwindow* window) {
    glfwSwapBuffers(window);
}

GLuint Renderer::compileShader(GLenum type, const char* source) {
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

GLuint Renderer::createProgram() {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    GLint ok = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        std::cerr << "Shader link failed:\n" << log << '\n';
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

Renderer::Shape Renderer::makeShape(const std::vector<Vertex>& vertices, GLenum mode) {
    Shape shape;
    shape.count = static_cast<GLsizei>(vertices.size());
    shape.mode = mode;
    glGenVertexArrays(1, &shape.vao);
    glGenBuffers(1, &shape.vbo);
    glBindVertexArray(shape.vao);
    glBindBuffer(GL_ARRAY_BUFFER, shape.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, r));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    return shape;
}

void Renderer::addTriangle(std::vector<Vertex>& vertices,
                           float x1, float y1, float x2, float y2, float x3, float y3,
                           glm::vec3 color) {
    vertices.push_back({x1, y1, color.r, color.g, color.b});
    vertices.push_back({x2, y2, color.r, color.g, color.b});
    vertices.push_back({x3, y3, color.r, color.g, color.b});
}

void Renderer::createGeometry() {
    std::vector<Vertex> rectangle;
    const glm::vec3 white(1.0f);
    addTriangle(rectangle, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, white);
    addTriangle(rectangle, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, white);
    rectangleShape_ = makeShape(rectangle, GL_TRIANGLES);

    std::vector<Vertex> triangle;
    addTriangle(triangle, 0.0f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, white);
    triangleShape_ = makeShape(triangle, GL_TRIANGLES);

    std::vector<Vertex> circle;
    constexpr int segments = 36;
    for (int i = 0; i < segments; ++i) {
        float a1 = 2.0f * 3.14159265f * i / segments;
        float a2 = 2.0f * 3.14159265f * (i + 1) / segments;
        addTriangle(circle, 0.0f, 0.0f, std::cos(a1), std::sin(a1),
                    std::cos(a2), std::sin(a2), white);
    }
    circleShape_ = makeShape(circle, GL_TRIANGLES);
}

void Renderer::drawShape(const Shape& shape, float x, float y, float sx, float sy,
                         glm::vec3 color, float rotation) {
    if (grayscaleDrawing_) {
        float gray = glm::dot(color, glm::vec3(0.299f, 0.587f, 0.114f));
        color = glm::vec3(gray);
    }
    x = (x + sceneOffsetX_) * sceneScale_;
    y = (y + sceneOffsetY_) * sceneScale_;
    sx *= sceneScale_;
    sy *= sceneScale_;
    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(x, y, 0.0f));
    model = glm::rotate(model, rotation, glm::vec3(0, 0, 1));
    model = glm::scale(model, glm::vec3(sx, sy, 1.0f));
    glUniformMatrix4fv(transformLocation_, 1, GL_FALSE, glm::value_ptr(model));
    const GLint tint = glGetUniformLocation(shaderProgram_, "u_Tint");
    if (tint >= 0) glUniform3fv(tint, 1, &color[0]);
    if (damageTintLocation_ >= 0) glUniform1f(damageTintLocation_, currentDamageTint_);
    glBindVertexArray(shape.vao);
    glDrawArrays(shape.mode, 0, shape.count);
    glBindVertexArray(0);
}

void Renderer::drawRectangle(float x, float y, float width, float height, glm::vec3 color) {
    drawShape(rectangleShape_, x, y, width, height, color);
}

void Renderer::drawCircle(float x, float y, float width, float height, glm::vec3 color) {
    drawShape(circleShape_, x, y, width, height, color);
}

void Renderer::drawHealthBar(float x, float y, int health, int maxHealth, glm::vec3 fillColor) {
    constexpr float segmentWidth = 0.022f;
    constexpr float segmentGap = 0.004f;
    constexpr float barHeight = 0.026f;
    const float totalWidth = maxHealth * segmentWidth + (maxHealth - 1) * segmentGap;
    const float firstCenter = x - totalWidth * 0.5f + segmentWidth * 0.5f;
    for (int i = 0; i < maxHealth; ++i) {
        const float segmentX = firstCenter + i * (segmentWidth + segmentGap);
        drawShape(rectangleShape_, segmentX, y, segmentWidth, barHeight,
                  glm::vec3(0.16f, 0.12f, 0.10f));
        if (i < health) {
            drawShape(rectangleShape_, segmentX, y, segmentWidth * 0.72f,
                      barHeight * 0.62f, fillColor);
        }
    }
}

void Renderer::drawEnemyPart(const Shape& shape, float centerX, float centerY,
                            float offsetX, float offsetY, float width, float height,
                            glm::vec3 color, float rotation) {
    constexpr float laneCharacterScale = 0.68f;
    drawShape(shape, centerX + offsetX * laneCharacterScale,
              centerY + offsetY * laneCharacterScale,
              width * laneCharacterScale, height * laneCharacterScale,
              color, rotation);
}

void Renderer::drawTreasureMap(float x, float y) {
    drawShape(rectangleShape_, x, y, 0.092f, 0.066f, glm::vec3(0.32f, 0.20f, 0.08f), -0.08f);
    drawShape(rectangleShape_, x, y, 0.080f, 0.054f, glm::vec3(0.94f, 0.82f, 0.52f), -0.08f);
}

void Renderer::drawEnemyCharacter(float x, float y, const Enemy& enemy,
                                  float hardWeaponRotation) {
    currentDamageTint_ = damageTintForHealth(enemy.health, enemy.maximumHealth());
    const glm::vec3 skin(0.66f, 0.40f, 0.24f);
    const glm::vec3 darkCloth(0.20f, 0.12f, 0.11f);
    switch (enemy.type) {
    case EnemyType::EasyUpperRanged:
        // A hunched hooded scout wears a long tunic, unlike the upright player models.
        drawEnemyPart(rectangleShape_, x, y, 0.008f, -0.080f, 0.105f, 0.17f,
                      glm::vec3(0.35f,0.14f,0.12f), -0.12f);
        drawEnemyPart(triangleShape_, x, y, 0.012f, -0.15f, 0.13f, 0.14f,
                      glm::vec3(0.31f,0.12f,0.10f), 3.14159f);
        drawEnemyPart(circleShape_, x, y, -0.012f, 0.045f, 0.048f, 0.048f, skin);
        drawEnemyPart(triangleShape_, x, y, -0.012f, 0.078f, 0.13f, 0.11f, darkCloth);
        drawEnemyPart(rectangleShape_, x, y, -0.012f, 0.025f, 0.07f, 0.018f,
                      glm::vec3(0.14f,0.10f,0.09f));
        drawEnemyPart(rectangleShape_, x, y, -0.035f, -0.18f, 0.055f, 0.07f,
                      darkCloth, -0.12f);
        drawEnemyPart(rectangleShape_, x, y, 0.032f, -0.18f, 0.055f, 0.07f,
                      darkCloth, 0.10f);
        // Its visible reaching arm and short sling form a held weapon silhouette.
        drawEnemyPart(rectangleShape_, x, y, -0.050f, 0.002f, 0.075f, 0.022f,
                      glm::vec3(0.40f,0.22f,0.14f), 0.30f);
        drawEnemyPart(circleShape_, x, y, -0.090f, 0.025f, 0.020f, 0.020f, skin);
        drawEnemyPart(rectangleShape_, x, y, -0.115f, 0.075f, 0.085f, 0.014f,
                      glm::vec3(0.48f,0.30f,0.15f), 0.30f);
        drawEnemyPart(rectangleShape_, x, y, -0.155f, 0.055f, 0.035f, 0.045f,
                      glm::vec3(0.28f,0.20f,0.13f), 0.30f);
        break;
    case EnemyType::MediumLowerRanged:
        drawEnemyPart(rectangleShape_, x, y, 0, -0.085f, 0.13f, 0.12f, glm::vec3(0.34f,0.25f,0.18f));
        drawEnemyPart(circleShape_, x, y, 0, 0.025f, 0.045f, 0.045f, skin);
        drawEnemyPart(rectangleShape_, x, y, 0, 0.025f, 0.09f, 0.022f, darkCloth);
        drawEnemyPart(rectangleShape_, x, y, -0.04f, -0.16f, 0.05f, 0.07f, darkCloth, -0.3f);
        drawEnemyPart(rectangleShape_, x, y, 0.04f, -0.16f, 0.05f, 0.07f, darkCloth, 0.3f);
        drawEnemyPart(rectangleShape_, x, y, -0.09f, -0.10f, 0.12f, 0.025f,
                      glm::vec3(0.42f,0.27f,0.12f));
        drawEnemyPart(triangleShape_, x, y, -0.15f, -0.10f, 0.05f, 0.05f,
                      glm::vec3(0.58f,0.54f,0.45f), -1.57f);
        break;
    case EnemyType::HardMelee:
        drawEnemyPart(rectangleShape_, x, y, 0, -0.075f, 0.14f, 0.18f, glm::vec3(0.30f,0.22f,0.17f));
        drawEnemyPart(circleShape_, x, y, 0, 0.065f, 0.055f, 0.055f, skin);
        drawEnemyPart(rectangleShape_, x, y, 0, 0.105f, 0.13f, 0.035f, glm::vec3(0.24f,0.23f,0.20f));
        drawEnemyPart(triangleShape_, x, y, -0.075f, 0.11f, 0.055f, 0.085f,
                      glm::vec3(0.24f,0.23f,0.20f), -0.35f);
        drawEnemyPart(triangleShape_, x, y, 0.075f, 0.11f, 0.055f, 0.085f,
                      glm::vec3(0.24f,0.23f,0.20f), 0.35f);
        drawEnemyPart(rectangleShape_, x, y, -0.04f, -0.17f, 0.04f, 0.10f, darkCloth);
        drawEnemyPart(rectangleShape_, x, y, 0.04f, -0.17f, 0.04f, 0.10f, darkCloth);
        drawEnemyPart(rectangleShape_, x, y, -0.12f, -0.015f, 0.14f, 0.04f,
                      glm::vec3(0.62f,0.61f,0.56f), hardWeaponRotation);
        drawEnemyPart(rectangleShape_, x, y, -0.055f, -0.005f, 0.04f, 0.055f,
                      glm::vec3(0.34f,0.20f,0.11f), hardWeaponRotation);
        break;
    }
    currentDamageTint_ = 0.0f;
}

void Renderer::drawPlayerPart(const Shape& shape, float x, float y, float offsetX,
                              float offsetY, float sx, float sy, glm::vec3 color,
                              float rotation, float jumpOffset, bool ducking) {
    const float heightScale = ducking ? 0.65f : 1.0f;
    const float poseOffset = jumpOffset - (ducking ? 0.025f : 0.0f);
    drawShape(shape, x + offsetX * playerDrawingScale_,
              y + (poseOffset + offsetY * heightScale) * playerDrawingScale_,
              sx * playerDrawingScale_, sy * heightScale * playerDrawingScale_,
              color, rotation);
}

void Renderer::drawPlayerCharacter(float x, float y, PlayerType playerType, int health,
                                   float jumpOffset, bool ducking, bool attacking,
                                   float scale, bool grayscale, float shieldRaise) {
    const float previousScale = playerDrawingScale_;
    const bool previousGrayscale = grayscaleDrawing_;
    playerDrawingScale_ = scale;
    grayscaleDrawing_ = grayscale;
    currentDamageTint_ = damageTintForHealth(health, playerType == PlayerType::Defense ? 7 : 5);
    const glm::vec3 skin(0.82f,0.60f,0.40f);
    const glm::vec3 darkCloth(0.22f,0.18f,0.15f);
    const float swordRotation = attacking ? -0.95f : -0.35f;
    switch (playerType) {
    case PlayerType::Melee:
        drawPlayerPart(rectangleShape_, x,y,0,-0.075f,0.075f,0.15f,glm::vec3(0.78f,0.68f,0.48f),0,jumpOffset,ducking);
        drawPlayerPart(circleShape_, x,y,0,0.065f,0.05f,0.05f,skin,0,jumpOffset,ducking);
        drawPlayerPart(rectangleShape_, x,y,0,0.105f,0.075f,0.022f,glm::vec3(0.16f,0.10f,0.06f),0,jumpOffset,ducking);
        drawPlayerPart(rectangleShape_, x,y,-0.025f,-0.17f,0.025f,0.10f,darkCloth,0,jumpOffset,ducking);
        drawPlayerPart(rectangleShape_, x,y,0.025f,-0.17f,0.025f,0.10f,darkCloth,0,jumpOffset,ducking);
        drawPlayerPart(rectangleShape_, x,y,0.075f,-0.005f,0.105f,0.018f,glm::vec3(0.75f,0.76f,0.72f),swordRotation,jumpOffset,ducking);
        drawPlayerPart(rectangleShape_, x,y,0.035f,-0.015f,0.035f,0.025f,glm::vec3(0.42f,0.24f,0.10f),swordRotation,jumpOffset,ducking);
        break;
    case PlayerType::Defense:
        drawPlayerPart(rectangleShape_, x,y,0,-0.075f,0.14f,0.17f,glm::vec3(0.46f,0.50f,0.43f),0,jumpOffset,ducking);
        drawPlayerPart(circleShape_, x,y,0,0.065f,0.052f,0.052f,skin,0,jumpOffset,ducking);
        drawPlayerPart(rectangleShape_, x,y,0,0.105f,0.11f,0.035f,glm::vec3(0.53f,0.39f,0.18f),0,jumpOffset,ducking);
        drawPlayerPart(rectangleShape_, x,y,-0.035f,-0.17f,0.035f,0.10f,darkCloth,0,jumpOffset,ducking);
        drawPlayerPart(rectangleShape_, x,y,0.035f,-0.17f,0.035f,0.10f,darkCloth,0,jumpOffset,ducking);
        // A broad, tall shield reaches above the protected players' heads.
        drawPlayerPart(rectangleShape_, x,y,0.075f,-0.025f + shieldRaise,0.15f,0.30f,glm::vec3(0.40f,0.25f,0.12f),0,jumpOffset,ducking);
        drawPlayerPart(triangleShape_, x,y,0.075f,-0.19f + shieldRaise,0.15f,0.08f,glm::vec3(0.40f,0.25f,0.12f),0,jumpOffset,ducking);
        break;
    case PlayerType::LongRange:
        drawPlayerPart(rectangleShape_, x,y,0,-0.075f,0.065f,0.15f,glm::vec3(0.43f,0.47f,0.30f),0,jumpOffset,ducking);
        drawPlayerPart(circleShape_, x,y,0,0.065f,0.048f,0.05f,skin,0,jumpOffset,ducking);
        drawPlayerPart(triangleShape_, x,y,0,0.09f,0.11f,0.10f,glm::vec3(0.31f,0.34f,0.24f),0,jumpOffset,ducking);
        drawPlayerPart(rectangleShape_, x,y,-0.022f,-0.17f,0.024f,0.10f,darkCloth,0,jumpOffset,ducking);
        drawPlayerPart(rectangleShape_, x,y,0.022f,-0.17f,0.024f,0.10f,darkCloth,0,jumpOffset,ducking);
        drawPlayerPart(rectangleShape_, x,y,0.09f,-0.025f,0.025f,0.19f,glm::vec3(0.48f,0.28f,0.12f),attacking?0.15f:-0.15f,jumpOffset,ducking);
        drawPlayerPart(rectangleShape_, x,y,0.085f,-0.02f,0.105f,0.012f,glm::vec3(0.78f,0.70f,0.45f),0,jumpOffset,ducking);
        break;
    }
    currentDamageTint_ = 0.0f;
    playerDrawingScale_ = previousScale;
    grayscaleDrawing_ = previousGrayscale;
}

void Renderer::drawPalmTree(float x, float groundY) {
    const glm::vec3 leafColor(0.16f, 0.42f, 0.19f);
    const float trunkHeight = 0.25f;
    const float trunkCenterY = groundY + trunkHeight * 0.5f;
    drawShape(rectangleShape_, x, trunkCenterY, 0.034f, trunkHeight, glm::vec3(0.48f,0.31f,0.16f));
    const float crownY = groundY + trunkHeight;
    drawShape(triangleShape_, x - 0.065f, crownY + 0.015f, 0.13f, 0.065f, leafColor, -0.95f);
    drawShape(triangleShape_, x - 0.035f, crownY + 0.045f, 0.13f, 0.065f, leafColor, -0.45f);
    drawShape(triangleShape_, x + 0.035f, crownY + 0.045f, 0.13f, 0.065f, leafColor, 0.45f);
    drawShape(triangleShape_, x + 0.065f, crownY + 0.015f, 0.13f, 0.065f, leafColor, 0.95f);
}

void Renderer::drawTorch(float x, float y) {
    drawShape(rectangleShape_, x, y - 0.034f, 0.025f, 0.12f, glm::vec3(0.35f,0.20f,0.11f));
    drawShape(rectangleShape_, x, y + 0.025f, 0.040f, 0.018f, glm::vec3(0.70f,0.48f,0.22f));
    drawShape(triangleShape_, x, y + 0.067f, 0.040f, 0.065f, glm::vec3(1.0f,0.42f,0.08f));
    drawShape(triangleShape_, x, y + 0.060f, 0.018f, 0.040f, glm::vec3(1.0f,0.82f,0.20f));
}

void Renderer::drawEnvironment(const bool (&selectedLanes)[3], int hoveredLane) {
    drawShape(rectangleShape_, 0,0.55f,2.0f,0.90f,glm::vec3(0.42f,0.70f,0.83f));
    drawShape(rectangleShape_, 0,-0.48f,2.0f,1.05f,glm::vec3(0.78f,0.64f,0.40f));
    // Sandy banks and river sit in the open background band above the top lane.
    drawShape(rectangleShape_, -0.56f,0.22f,0.88f,0.34f,glm::vec3(0.78f,0.64f,0.40f));
    drawShape(rectangleShape_, 0.56f,0.22f,0.88f,0.34f,glm::vec3(0.78f,0.64f,0.40f));
    // Keep the three pyramids visible on the sandy bank to the river's right.
    drawShape(triangleShape_, 0.43f,0.23f,0.52f,0.30f,glm::vec3(0.62f,0.46f,0.30f));
    drawShape(triangleShape_, 0.66f,0.27f,0.62f,0.38f,glm::vec3(0.70f,0.52f,0.33f));
    drawShape(triangleShape_, 0.88f,0.23f,0.48f,0.30f,glm::vec3(0.58f,0.42f,0.27f));
    drawShape(rectangleShape_, 0.0f,0.22f,0.24f,0.34f,glm::vec3(0.17f,0.52f,0.65f));
    drawShape(rectangleShape_, -0.055f,0.22f,0.018f,0.34f,glm::vec3(0.42f,0.73f,0.76f));
    // Temple columns and walls span the complete height of the playable lanes.
    drawShape(rectangleShape_, -0.88f,-0.485f,0.20f,0.97f,glm::vec3(0.77f,0.69f,0.54f));
    drawShape(triangleShape_, -0.88f,0.055f,0.28f,0.16f,glm::vec3(0.61f,0.48f,0.31f));
    for (int i = 0; i < 3; ++i)
        drawShape(rectangleShape_, -0.94f + i * 0.06f,-0.49f,0.025f,0.88f,glm::vec3(0.56f,0.46f,0.34f));
    drawShape(rectangleShape_, -0.88f,-0.94f,0.28f,0.06f,glm::vec3(0.51f,0.39f,0.26f));
    // Three palms stand along the river's left sandy bank.
    drawPalmTree(-0.78f, 0.055f);
    drawPalmTree(-0.57f, 0.055f);
    drawPalmTree(-0.46f, 0.055f);
    // Statue on the right bank; obelisk on the left bank with a pointed cap.
    drawShape(rectangleShape_, 0.29f,-0.79f,0.13f,0.28f,glm::vec3(0.66f,0.56f,0.39f));
    drawShape(circleShape_, 0.29f,-0.59f,0.10f,0.10f,glm::vec3(0.71f,0.61f,0.43f));
    drawShape(rectangleShape_, 0.29f,-0.94f,0.20f,0.045f,glm::vec3(0.49f,0.39f,0.27f));
    drawShape(rectangleShape_, -0.23f,0.27f,0.09f,0.42f,glm::vec3(0.43f,0.34f,0.25f));
    drawShape(triangleShape_, -0.23f,0.525f,0.10f,0.10f,glm::vec3(0.51f,0.40f,0.28f));
    drawShape(rectangleShape_, -0.23f,0.07f,0.15f,0.04f,glm::vec3(0.34f,0.27f,0.20f));
    drawTorch(0.92f, 0.13f);
    drawTorch(0.92f, GameConfig::LANE_Y[2]);
    for (int i = 0; i < 3; ++i) {
        glm::vec3 laneColor(0.73f,0.57f,0.34f);
        if (selectedLanes[i]) laneColor = glm::vec3(0.82f,0.66f,0.42f);
        if (i == hoveredLane) laneColor = glm::vec3(0.90f,0.77f,0.52f);
        drawShape(rectangleShape_,0.10f,GameConfig::LANE_Y[i],1.48f,GameConfig::LANE_HEIGHT,laneColor);
        drawShape(rectangleShape_,0.10f,GameConfig::LANE_Y[i]-0.17f,1.48f,0.012f,glm::vec3(0.48f,0.37f,0.24f));
        if (selectedLanes[i])
            drawShape(triangleShape_,-0.68f,GameConfig::LANE_Y[i],0.045f,0.07f,glm::vec3(0.20f,0.75f,0.28f));
    }
}

void Renderer::drawWorld(std::vector<Player>& players, const std::vector<Enemy>& enemies,
                         const std::vector<Projectile>& enemyProjectiles,
                         const std::vector<Projectile>& playerProjectiles,
                         const std::vector<Collectible>& collectibles,
                         const bool (&selectedLanes)[3], int hoveredLane, bool gameOver) {
    glUseProgram(shaderProgram_);
    drawEnvironment(selectedLanes, hoveredLane);
    for (Player& player : players) {
        if (!player.alive || player.lane < 0 || player.lane >= GameConfig::LANE_COUNT) continue;
        player.pos.y = GameConfig::LANE_Y[player.lane];
        const float jumpProgress = 1.0f - player.jumpTimer / GameConfig::JUMP_DURATION;
        const float jumpOffset = player.jumpTimer > 0.0f
            ? 0.20f * std::sin(jumpProgress * 3.14159265f) : 0.0f;
        drawPlayerCharacter(player.pos.x, player.pos.y, player.type, player.health,
                            jumpOffset, player.ducking, player.attackAnimationTimer > 0.0f,
                            0.68f);
        drawHealthBar(player.pos.x, player.pos.y + 0.135f + jumpOffset,
                      player.health, player.maximumHealth(), glm::vec3(0.22f,0.76f,0.24f));
    }
    if (!gameOver) {
        for (const Enemy& enemy : enemies) {
            if (!enemy.alive) continue;
            const float enemyY = GameConfig::LANE_Y[enemy.lane];
            drawEnemyCharacter(enemy.pos.x, enemyY, enemy);
            drawHealthBar(enemy.pos.x, enemyY + 0.135f, enemy.health,
                          enemy.maximumHealth(), glm::vec3(0.88f,0.18f,0.12f));
        }
        for (const Projectile& projectile : enemyProjectiles) {
            const glm::vec3 color = projectile.height == ProjectileHeight::Upper
                ? glm::vec3(0.95f,0.35f,0.12f) : glm::vec3(0.95f,0.78f,0.20f);
            drawCircle(projectile.pos.x, projectile.pos.y, 0.035f, 0.035f, color);
        }
        for (const Projectile& projectile : playerProjectiles)
            drawShape(triangleShape_, projectile.pos.x, projectile.pos.y, 0.045f,0.025f,glm::vec3(0.95f,0.88f,0.44f));
    }
    for (const Collectible& collectible : collectibles) {
        if (!collectible.alive) continue;
        drawCircle(collectible.pos.x, GameConfig::LANE_Y[collectible.lane], 0.035f,0.035f,glm::vec3(1.0f,0.78f,0.10f));
        drawCircle(collectible.pos.x, GameConfig::LANE_Y[collectible.lane], 0.020f,0.020f,glm::vec3(0.90f,0.52f,0.05f));
    }
}

void Renderer::drawCutscene(const std::vector<Player>& heroes,
                            const std::vector<Enemy>& enemies, float elapsed,
                            bool heshamAttacking, bool fahmyAiming,
                            float shikoShieldRaise, float bossWeaponRotation) {
    const bool noSelectedLanes[GameConfig::LANE_COUNT] = {false, false, false};
    sceneScale_ = 1.0f + 0.018f * glm::clamp(elapsed / 10.0f, 0.0f, 1.0f);
    sceneOffsetX_ = -0.025f * glm::clamp(elapsed / 10.0f, 0.0f, 1.0f);
    sceneOffsetY_ = 0.0f;
    glUseProgram(shaderProgram_);
    drawEnvironment(noSelectedLanes, -1);

    for (const Player& hero : heroes) {
        const bool attacking = hero.type == PlayerType::Melee ? heshamAttacking
            : (hero.type == PlayerType::LongRange && fahmyAiming);
        const float shieldRaise = hero.type == PlayerType::Defense ? shikoShieldRaise : 0.0f;
        drawPlayerCharacter(hero.pos.x, hero.pos.y, hero.type, hero.health,
                            0.0f, false, attacking, 0.68f, false, shieldRaise);
    }

    for (std::size_t i = 0; i < enemies.size(); ++i) {
        if (!enemies[i].alive) continue;
        const float weaponRotation = i == 0 ? bossWeaponRotation : -0.25f;
        drawEnemyCharacter(enemies[i].pos.x, enemies[i].pos.y, enemies[i], weaponRotation);
    }

    for (const Player& hero : heroes) {
        if (hero.type == PlayerType::Melee) {
            drawTreasureMap(hero.pos.x + 0.12f, hero.pos.y + 0.055f);
            break;
        }
    }

    sceneScale_ = 1.0f;
    sceneOffsetX_ = 0.0f;
    sceneOffsetY_ = 0.0f;
}

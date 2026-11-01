// Temple Defense - OpenGL 3.3 starter project
// Built from the concepts in Graphics Labs 1-3: VAO/VBO, shaders, transforms,
// primitive shapes, GLFW input, and mouse-coordinate conversion.
// This is a foundation, not the complete assignment.

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

constexpr int WINDOW_W = 1100;
constexpr int WINDOW_H = 700;
constexpr int LANE_COUNT = 3;
constexpr float LANE_HEIGHT = 0.32f;
constexpr float LANE_Y[LANE_COUNT] = {-0.125f, -0.48f, -0.835f};

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
void main() { FragColor = vec4(vColor * u_Tint, 1.0); }
)";

struct Vertex { float x, y, r, g, b; };
struct Shape { GLuint vao = 0, vbo = 0; GLsizei count = 0; GLenum mode = GL_TRIANGLES; };
struct Enemy { glm::vec2 pos; int lane; float speed; int hp; bool alive; };

GLuint shaderProgram = 0;
GLint transformLocation = -1;
Shape rectangleShape, triangleShape, circleShape;
int selectedLane = 1;
int hoveredLane = -1;
float playerX = -0.62f;
float playerY = 0.0f;
float enemyX = 0.72f;
float enemySpeed = 0.10f;
int coins = 0;
bool paused = false;
bool gameOver = false;
float lastFrameTime = 0.0f;
std::vector<Enemy> enemies;

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

void mouseButtonCallback(GLFWwindow* window, int button, int action, int) {
    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) return;
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    glm::vec2 p = cursorToNdc(x, y);
    int lane = laneFromY(p.y);
    if (lane >= 0) selectedLane = lane;
}

void keyCallback(GLFWwindow* window, int key, int, int action, int) {
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_ESCAPE) paused = !paused;
    if (key == GLFW_KEY_R) {
        playerX = -0.62f; enemyX = 0.72f; coins = 0; gameOver = false; paused = false;
    }
    if (key == GLFW_KEY_UP || key == GLFW_KEY_W) selectedLane = std::max(0, selectedLane - 1);
    if (key == GLFW_KEY_DOWN || key == GLFW_KEY_S) selectedLane = std::min(LANE_COUNT - 1, selectedLane + 1);
    if (key == GLFW_KEY_LEFT || key == GLFW_KEY_A) playerX = std::max(-0.80f, playerX - 0.06f);
    if (key == GLFW_KEY_RIGHT || key == GLFW_KEY_D) playerX = std::min(-0.38f, playerX + 0.06f);
    if (key == GLFW_KEY_SPACE && gameOver) {
        playerX = -0.62f; enemyX = 0.72f; coins = 0; gameOver = false;
    }
    (void)window;
}

void drawCharacter(float x, float y, bool enemy) {
    // Simple placeholder characters built from basic primitives.
    glm::vec3 skin = enemy ? glm::vec3(0.66f,0.40f,0.24f) : glm::vec3(0.82f,0.60f,0.40f);
    glm::vec3 clothes = enemy ? glm::vec3(0.35f,0.15f,0.12f) : glm::vec3(0.78f,0.68f,0.48f);
    drawShape(rectangleShape, x, y - 0.08f, enemy ? 0.09f : 0.08f, 0.18f, clothes);
    drawShape(circleShape, x, y + 0.055f, 0.055f, 0.055f, skin);
    drawShape(rectangleShape, x, y - 0.19f, 0.035f, 0.10f, glm::vec3(0.22f,0.18f,0.15f));
    drawShape(rectangleShape, x + (enemy ? -0.075f : 0.075f), y - 0.02f, 0.07f, 0.025f,
              enemy ? glm::vec3(0.25f,0.25f,0.25f) : glm::vec3(0.45f,0.25f,0.10f), enemy ? 0.0f : -0.3f);
    if (enemy) {
        drawShape(rectangleShape, x, y + 0.085f, 0.11f, 0.025f, glm::vec3(0.12f,0.10f,0.09f));
    } else {
        // Tiny dark hair shape.
        drawShape(rectangleShape, x, y + 0.095f, 0.08f, 0.025f, glm::vec3(0.16f,0.10f,0.06f));
    }
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

    // Three lanes and separators. Hovered lane is lighter; selected lane has a green marker.
    for (int i = 0; i < 3; ++i) {
        glm::vec3 laneColor = (i == hoveredLane) ? glm::vec3(0.90f,0.77f,0.52f) : glm::vec3(0.73f,0.57f,0.34f);
        drawShape(rectangleShape, 0.10f, LANE_Y[i], 1.48f, LANE_HEIGHT, laneColor);
        drawShape(rectangleShape, 0.10f, LANE_Y[i] - 0.17f, 1.48f, 0.012f, glm::vec3(0.48f,0.37f,0.24f));
        if (i == selectedLane)
            drawShape(triangleShape, -0.68f, LANE_Y[i], 0.045f, 0.07f, glm::vec3(0.20f,0.75f,0.28f));
    }

    // Placeholder player and enemy on the selected lane.
    playerY = LANE_Y[selectedLane];
    drawCharacter(playerX, playerY, false);
    if (!gameOver) drawCharacter(enemyX, LANE_Y[1], true);

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
    createGeometry();
    std::cout << "Controls: click a lane or use W/S or Up/Down to select it; A/D moves the player; Esc pauses; R resets.\n";

    while (!glfwWindowShouldClose(window)) {
        float now = static_cast<float>(glfwGetTime());
        float deltaTime = std::min(now - lastFrameTime, 0.05f);
        lastFrameTime = now;
        if (!paused && !gameOver) {
            enemyX -= enemySpeed * deltaTime;
            if (enemyX < -0.78f) gameOver = true;
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

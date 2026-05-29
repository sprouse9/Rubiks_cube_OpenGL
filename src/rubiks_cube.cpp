#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <ctime>
#include <vector>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"

#include <iostream>

enum class Axis
{
    X,
    Y,
    Z
};

struct Move
{
    Axis axis;

    int layer;      // -1, 0, 1

    int direction;  // +1 or -1

    float currentAngle;
};

enum Face
{
    BACK = 0,
    FRONT,
    LEFT,
    RIGHT,
    BOTTOM,
    TOP
};

struct Cubelet
{
    glm::ivec3 gridPos;
    glm::mat4 orientation;
    glm::vec3 faceColors[6];
};


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

bool isInLayer(const Cubelet& c, const Move& move);
glm::vec3 getAxisVector(Axis axis);
void commitMove(std::vector<Cubelet>& cubelets, const Move& move);

Move randomMove();

GLFWwindow* createOpenGLWindow(int width, int height, const char* title);

// settings
const unsigned int SCR_WIDTH = 640;
const unsigned int SCR_HEIGHT = 480;

int main()
{
    GLFWwindow* window = createOpenGLWindow(
        640,
        480,
        "Rubik's Cube OpenGL Demo"
    );

    if (!window)
        return 1;

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("shaders/rubiks_cube.vert", "shaders/rubiks_cube.frag");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {

        // =========================
        // BACK FACE -Z (Blue)
        // =========================
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        // =========================
        // FRONT FACE +Z (Green)
        // =========================
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        // =========================
        // LEFT FACE -X (Orange)
        // =========================
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

        // =========================
        // RIGHT FACE +X (Red)
        // =========================
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,

     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        // =========================
        // BOTTOM FACE -Y (Yellow)
        // =========================
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,

     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        // =========================
        // TOP FACE +Y (White)
        // =========================
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f
    };
    
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // color attribute
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    //glEnableVertexAttribArray(1);

    // tell opengl for each sampler to which texture unit it belongs to
    ourShader.use();


    const glm::vec3 BLACK  = glm::vec3(0.03f, 0.03f, 0.03f);
    const glm::vec3 BLUE   = glm::vec3(0.0f, 0.0f, 1.0f);
    const glm::vec3 GREEN  = glm::vec3(0.0f, 1.0f, 0.0f);
    const glm::vec3 ORANGE = glm::vec3(1.0f, 0.5f, 0.0f);
    const glm::vec3 RED    = glm::vec3(1.0f, 0.0f, 0.0f);
    const glm::vec3 YELLOW = glm::vec3(1.0f, 1.0f, 0.0f);
    const glm::vec3 WHITE  = glm::vec3(1.0f, 1.0f, 1.0f);


    std::vector<Cubelet> cubelets;

    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            for (int z = -1; z <= 1; z++)
            {
                Cubelet c;

                c.gridPos = glm::ivec3(x, y, z);
                c.orientation = glm::mat4(1.0f);

                // start all faces black
                for (int i = 0; i < 6; i++)
                    c.faceColors[i] = BLACK;

                // expose only outer faces
                if (x ==  1) c.faceColors[RIGHT]  = RED;
                if (x == -1) c.faceColors[LEFT]   = ORANGE;

                if (y ==  1) c.faceColors[TOP]    = WHITE;
                if (y == -1) c.faceColors[BOTTOM] = YELLOW;

                if (z ==  1) c.faceColors[FRONT]  = GREEN;
                if (z == -1) c.faceColors[BACK]   = BLUE;

                cubelets.push_back(c);
            }
        }
    }

    float cubeScale = 0.4f;
    float gap = 0.01f;
    float spacing = cubeScale + gap;

    srand((unsigned int)time(nullptr));

    bool moveActive = true;
    Move activeMove = randomMove();

    float rotationSpeed = 360.0f;
    float lastFrame = 0.0f;
    float lastRotationEndedTime = 0.0f;
    float pauseDuration = 0.50f;

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        float currentFrame = (float)glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (moveActive)
        {
            activeMove.currentAngle += rotationSpeed * deltaTime;

            // Here we commit and finalize the current rotation
            if (activeMove.currentAngle >= 90.0f)
            {
                activeMove.currentAngle = 90.0f;

                commitMove(cubelets, activeMove);

                activeMove = randomMove();

                moveActive = false;
                lastRotationEndedTime = currentFrame;
            }
        }
        else
        {
            float pauseElapsed = currentFrame - lastRotationEndedTime;

            if (pauseElapsed > pauseDuration)
            {
                moveActive = true;
            }
        }

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();

        glm::vec3 lightPos(2.0f, 3.0f, 4.0f);
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

        // Your view matrix later translates the world by -3 on Z,
        // so the camera is effectively at world position (0, 0, 3).
        glm::vec3 cameraPos(0.0f, 0.0f, 3.0f);

        ourShader.setVec3("lightPos", lightPos);
        ourShader.setVec3("lightColor", lightColor);
        ourShader.setVec3("viewPos", cameraPos);




        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

        int framebufferWidth = 0;
        int framebufferHeight = 0;

        glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);

        if (framebufferHeight == 0)
            framebufferHeight = 1;

        float aspect = static_cast<float>(framebufferWidth) /
                    static_cast<float>(framebufferHeight);

        projection = glm::perspective(
            glm::radians(45.0f),
            aspect,
            0.1f,
            100.0f
        );



        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);





        glBindVertexArray(VAO);

        glm::mat4 cubeRotation = glm::mat4(1.0f);

        cubeRotation = glm::rotate(
            cubeRotation,
            glm::radians(25.0f),
            glm::vec3(1.0f, 0.0f, 0.0f)
        );

        cubeRotation = glm::rotate(
            cubeRotation,
            glm::radians(135.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );



        for (Cubelet& cubelet : cubelets)
        {
            glm::mat4 model = cubeRotation;

            if (moveActive && isInLayer(cubelet, activeMove))
            {
                model = glm::rotate(
                    model,
                    glm::radians(activeMove.currentAngle * activeMove.direction),
                    getAxisVector(activeMove.axis)
                );
            }

            model = glm::translate(
                model,
                glm::vec3(cubelet.gridPos) * spacing
            );

            model = model * cubelet.orientation;

            model = glm::scale(
                model,
                glm::vec3(cubeScale)
            );

            ourShader.setMat4("model", model);




            // glDrawArrays(GL_TRIANGLES, 0, 36);
            // we will now draw each cubelet separately

            ourShader.setVec3("faceColor", cubelet.faceColors[BACK]);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            ourShader.setVec3("faceColor", cubelet.faceColors[FRONT]);
            glDrawArrays(GL_TRIANGLES, 6, 6);

            ourShader.setVec3("faceColor", cubelet.faceColors[LEFT]);
            glDrawArrays(GL_TRIANGLES, 12, 6);

            ourShader.setVec3("faceColor", cubelet.faceColors[RIGHT]);
            glDrawArrays(GL_TRIANGLES, 18, 6);

            ourShader.setVec3("faceColor", cubelet.faceColors[BOTTOM]);
            glDrawArrays(GL_TRIANGLES, 24, 6);

            ourShader.setVec3("faceColor", cubelet.faceColors[TOP]);
            glDrawArrays(GL_TRIANGLES, 30, 6);



        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }



        // optional: de-allocate all resources once they've outlived their purpose:
        // ------------------------------------------------------------------------
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);

        // glfw: terminate, clearing all previously allocated GLFW resources.
        // ------------------------------------------------------------------
        glfwTerminate();
        return 0;
    }

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    if (height == 0)
        height = 1;

    glViewport(0, 0, width, height);
}


bool isInLayer(const Cubelet& c, const Move& move)
{
    switch (move.axis)
    {
        case Axis::X:
            return c.gridPos.x == move.layer;

        case Axis::Y:
            return c.gridPos.y == move.layer;

        case Axis::Z:
            return c.gridPos.z == move.layer;
    }

    return false;
}

glm::vec3 getAxisVector(Axis axis)
{
    switch (axis)
    {
        case Axis::X:
            return glm::vec3(1,0,0);

        case Axis::Y:
            return glm::vec3(0,1,0);

        case Axis::Z:
            return glm::vec3(0,0,1);
    }

    return glm::vec3(0,1,0);
}

void commitMove(std::vector<Cubelet>& cubelets, const Move& move)
{
    glm::mat4 finalRotation = glm::rotate(
        glm::mat4(1.0f),
        glm::radians(90.0f * move.direction),
        getAxisVector(move.axis)
    );

    for (Cubelet& c : cubelets)
    {
        if (!isInLayer(c, move))
            continue;

        int oldX = c.gridPos.x;
        int oldY = c.gridPos.y;
        int oldZ = c.gridPos.z;

        if (move.axis == Axis::X)
        {
            if (move.direction == 1)
            {
                c.gridPos.y = -oldZ;
                c.gridPos.z = oldY;
            }
            else
            {
                c.gridPos.y = oldZ;
                c.gridPos.z = -oldY;
            }
        }
        else if (move.axis == Axis::Y)
        {
            if (move.direction == 1)
            {
                c.gridPos.x = oldZ;
                c.gridPos.z = -oldX;
            }
            else
            {
                c.gridPos.x = -oldZ;
                c.gridPos.z = oldX;
            }
        }
        else if (move.axis == Axis::Z)
        {
            if (move.direction == 1)
            {
                c.gridPos.x = -oldY;
                c.gridPos.y = oldX;
            }
            else
            {
                c.gridPos.x = oldY;
                c.gridPos.y = -oldX;
            }
        }

        c.orientation = finalRotation * c.orientation;
    }
}

Move randomMove()
{
    Move move;

    move.axis = static_cast<Axis>(rand() % 3); // X, Y, Z
    move.layer = (rand() % 3) - 1;             // -1, 0, 1
    move.direction = (rand() % 2 == 0) ? 1 : -1;
    move.currentAngle = 0.0f;

    return move;
}


GLFWwindow* createOpenGLWindow(int width, int height, const char* title)
{
    // glfw: initialize and configure
    // ------------------------------
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return nullptr;
    }

    // macOS supports OpenGL 4.1 Core Profile, not modern 4.5/4.6.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef __APPLE__
        // Required on macOS for OpenGL 3.2+ core contexts.
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        std::cout << "Apple computer detected\n";
    #endif

    // Anti-aliasing
    glfwWindowHint(GLFW_SAMPLES, 4);

   // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return nullptr;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    int framebuffer_width = 0;
    int framebuffer_height = 0;
    glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
    glViewport(0, 0, framebuffer_width, framebuffer_height);

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "Width x Height: (" 
              << framebuffer_width << ", " 
              << framebuffer_height << ")\n";

    return window;
}
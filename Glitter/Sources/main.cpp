// Local Headers
#include "glitter.hpp"
#include "shader.h"
#include "camera.h"
#include "model.h"
#include "TextureBuffer.h"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION

// Standard Headers
#include <cstdio>
#include <cstdlib>

struct Hue {
    glm::vec3 cool;
    glm::vec3 warm;
    float alpha;
    float beta;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// set default hue colors and weights
struct Hue hue = {
    glm::vec3(0.0f, 0.0f, 0.4f), // cool
    glm::vec3(0.4f, 0.4f, 0.0f), // warm
    0.2f, // alpha
    0.6f // beta
};

// camera
Camera camera(glm::vec3(-10.0f, 10.0f, 20.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

Shader genShader(string fname, string glitterDir) {
    string vertexShader = glitterDir + "\\Shaders\\" + fname + ".vs";
    string fragShader = glitterDir + "\\Shaders\\" + fname + ".fs";
    return Shader(vertexShader.c_str(), fragShader.c_str());
}

int main(int argc, char * argv[]) {

    // Load GLFW and Create a Window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    auto mWindow = glfwCreateWindow(mWidth, mHeight, "OpenGL", nullptr, nullptr);
    // Check for Valid Context
    if (mWindow == nullptr) {
        fprintf(stderr, "Failed to Create OpenGL Context");
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(mWindow);
    glfwSetFramebufferSizeCallback(mWindow, framebuffer_size_callback);
    glfwSetCursorPosCallback(mWindow, mouse_callback);
    glfwSetScrollCallback(mWindow, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize GLAD");
        return -1;
    }

    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // add extra depth/normal framebuffers
    // used for silhouetteing
    // -----------------------------
    TextureBuffer normalBuff = TextureBuffer(SCR_WIDTH, SCR_HEIGHT, true);
    TextureBuffer depthBuff = TextureBuffer(SCR_WIDTH, SCR_HEIGHT, false);
    TextureBuffer normalEdgeBuff = TextureBuffer(SCR_WIDTH, SCR_HEIGHT, true);
    TextureBuffer depthEdgeBuff = TextureBuffer(SCR_WIDTH, SCR_HEIGHT, false);

    // set glitter dir and shader locations
    // ------------------------------------
    string glitterDir = "C:\\Users\\gusca\\Desktop\\graph final\\Glitter\\Glitter";

    // build and compile our shader programs
    // ------------------------------------
    Shader ourShader = genShader("model", glitterDir);
    Shader silNormalShader = genShader("silNormal", glitterDir);
    Shader silDepthShader = genShader("silDepth", glitterDir);
    Shader normalShader = genShader("normal", glitterDir);
    Shader depthShader = genShader("depth", glitterDir);
    Shader comboShader = genShader("combo", glitterDir);
    Shader quadShader = genShader("quad", glitterDir);

    // load models
    // -----------
    string modelObj = "\\resources\\A-Wing Starfighter.obj";
    //string modelObj = "\\resources\\teapot\\teapot_n_glass.obj";
    Model ourModel((glitterDir + modelObj).c_str());

    // Create Context and Load OpenGL Functions
    glfwMakeContextCurrent(mWindow);
    gladLoadGL();
    fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));

    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates. NOTE that this plane is now much smaller and at the top of the screen
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    // screen quad VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));


    // render loop
    // -----------
    while (!glfwWindowShouldClose(mWindow))
    {
        glEnable(GL_DEPTH_TEST);
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(mWindow);
        
        // set up MVP matrices
        // model matrix
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); 
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // render depth and normal textures
        // -----
        glBindFramebuffer(GL_FRAMEBUFFER, normalBuff.FBO);
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        silNormalShader.use();
        silNormalShader.setMat4("projection", projection);
        silNormalShader.setMat4("view", view);
        silNormalShader.setMat4("model", model);

        ourModel.DrawToBuffer(silNormalShader);

        glBindFramebuffer(GL_FRAMEBUFFER, depthBuff.FBO);
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        silDepthShader.use();
        silDepthShader.setMat4("projection", projection);
        silDepthShader.setMat4("view", view);
        silDepthShader.setMat4("model", model);

        ourModel.DrawToBuffer(silDepthShader);

        // process depth and normal for outlines
        // -----
        glBindFramebuffer(GL_FRAMEBUFFER, normalEdgeBuff.FBO);
        glDisable(GL_DEPTH_TEST);
        normalShader.use();
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, normalBuff.tex);	
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindFramebuffer(GL_FRAMEBUFFER, depthEdgeBuff.FBO);
        glDisable(GL_DEPTH_TEST);
        depthShader.use();
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, depthBuff.tex);	
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // render to main frame
        // ------
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // don't forget to enable shader before setting uniforms
        ourShader.use();

        // update light direction for hue
        ourShader.setVec3("aLightDir", camera.Right);

        // hue colors and weights
        ourShader.setVec3("hue.cool", hue.cool);
        ourShader.setVec3("hue.warm", hue.warm);
        ourShader.setFloat("hue.alpha", hue.alpha);
        ourShader.setFloat("hue.beta", hue.beta);

        // view/projection transformations
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        ourShader.setMat4("model", model);

        ourModel.Draw(ourShader);

        glDisable(GL_DEPTH_TEST);
        quadShader.use();

        glBindTexture(GL_TEXTURE_2D, normalEdgeBuff.tex);

        glBindVertexArray(quadVAO);
        //glDrawArrays(GL_TRIANGLES, 0, 6);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return EXIT_SUCCESS;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.sprint();
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
        camera.slow();
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);

    // control Hue alpha
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        hue.alpha = min(hue.alpha + 0.001f, 1.0f);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        hue.alpha = max(hue.alpha - 0.001f, 0.0f);

    // control Hue beta
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        hue.beta = min(hue.beta + 0.001f, 1.0f);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        hue.beta = max(hue.beta - 0.001f, 0.0f);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

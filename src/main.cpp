#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <iostream>

// structures
enum SELECTED_OBJECT { TEAPOT, NANOSUIT};

// constants
const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;
const GLfloat near_plane = -10.0f, far_plane = 10.0f;

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
GLfloat lightRotationY;
GLfloat lightRotationZ;
glm::vec3 lightDir;
glm::vec3 lightColor;
GLfloat lightColorCoeff;

// shadow parameters
GLuint shadowMapFBO;
GLuint depthMapTexture;
glm::mat4 lightView;
glm::mat4 lightProjection;
glm::mat4 lightSpaceTrMatrix;

// fog parameters
GLboolean fogEnabled; // I ran out of buttons so I won't bother with making the density and gradient uniforms

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;
GLint lightSpaceTrMatrixLoc;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 1.0f, 4.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];

// models
gps::Model3D teapot;
gps::Model3D ground;
gps::Model3D nanosuit;

// transformation variables
GLfloat teapotAngleX;
GLfloat teapotAngleY;
GLfloat teapotAngleZ;
GLfloat teapotPositionX;
GLfloat teapotPositionY;
GLfloat teapotPositionZ;

GLfloat nanosuitAngleX;
GLfloat nanosuitAngleY = 270.0f;
GLfloat nanosuitAngleZ;
GLfloat nanosuitPositionX = 5.0f;
GLfloat nanosuitPositionY = 0.25f;
GLfloat nanosuitPositionZ = -3.0f;

GLfloat cameraYaw;
GLfloat cameraPitch;

// shaders
gps::Shader myBasicShader;
gps::Shader myShadowShader;
gps::Shader mySkyBoxShader;

// animations
bool teapotAnimation = true;
bool teapotAnimationDirection = false;
bool nanosuitAnimation = true;
bool nanosuitAnimationDirection = false;
bool nanosuitAnimationTurning = false;
bool lightAnimation = true;

gps::SkyBox mySkyBox;
SELECTED_OBJECT active_object = TEAPOT;

bool wireframeEnable = false;

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);

    glViewport(0, 0, width, height);

    projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 20.0f);
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    WindowDimensions newDimensions;
    newDimensions.width = width;
    newDimensions.height = height;
    myWindow.setWindowDimensions(newDimensions);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    //TODO
    //not doing it
}

void processMovement() {
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	}

    if (pressedKeys[GLFW_KEY_R]) {
        myCamera.move(gps::MOVE_UP, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    }

    if (pressedKeys[GLFW_KEY_F]) {
        myCamera.move(gps::MOVE_DOWN, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    }

    if (pressedKeys[GLFW_KEY_Q]) {
        switch (active_object) {
        case TEAPOT:
            teapotAnimation = false;
            teapotAngleY -= 1.0f;
            if (teapotAngleY < 0.0f) {
                teapotAngleY += 360.0f;
            }
            teapotAngleZ = 0.0f;
            break;
        case NANOSUIT:
            nanosuitAnimation = false;
            nanosuitAnimationDirection = false;
            nanosuitAnimationTurning = false;
            nanosuitAngleY -= 1.0f;
            if (nanosuitAngleY < 0.0f) {
                nanosuitAngleY += 360.0f;
            }
            nanosuitAngleZ = 0.0f;
            break;
        }
    }

    if (pressedKeys[GLFW_KEY_E]) {
        switch (active_object) {
        case TEAPOT:
            teapotAnimation = false;
            teapotAngleY += 1.0f;
            if (teapotAngleY >= 360.0f) {
                teapotAngleY -= 360.0f;
            }
            teapotAngleZ = 0.0f;
            break;
        case NANOSUIT:
            nanosuitAnimation = false;
            nanosuitAnimationDirection = false;
            nanosuitAnimationTurning = false;
            nanosuitAngleY += 1.0f;
            if (nanosuitAngleY >= 360.0f) {
                nanosuitAngleY -= 360.0f;
            }
            nanosuitAngleZ = 0.0f;
            break;
        }
    }

    if (pressedKeys[GLFW_KEY_I]) {
        switch (active_object) {
        case TEAPOT:
            teapotAnimation = false;
            teapotAngleX -= 1.0f;
            if (teapotAngleX < 0.0f) {
                teapotAngleX += 360.0f;
            }
            teapotAngleZ = 0.0f;
            break;
        case NANOSUIT:
            nanosuitAnimation = false;
            nanosuitAnimationDirection = false;
            nanosuitAnimationTurning = false;
            nanosuitAngleX -= 1.0f;
            if (nanosuitAngleX < 0.0f) {
                nanosuitAngleX += 360.0f;
            }
            nanosuitAngleZ = 0.0f;
            break;
            break;
        }
    }

    if (pressedKeys[GLFW_KEY_K]) {
        switch (active_object) {
        case TEAPOT:
            teapotAnimation = false;
            teapotAngleX += 1.0f;
            if (teapotAngleX >= 360.0f) {
                teapotAngleX -= 360.0f;
            }
            teapotAngleZ = 0.0f;
            break;
        case NANOSUIT:
            nanosuitAnimation = false;
            nanosuitAnimationDirection = false;
            nanosuitAnimationTurning = false;
            nanosuitAngleX += 1.0f;
            if (nanosuitAngleX >= 360.0f) {
                nanosuitAngleX -= 360.0f;
            }
            nanosuitAngleZ = 0.0f;
            break;
            break;
        }

    }

    //reset objects
    if (pressedKeys[GLFW_KEY_Z]) {
        teapotAnimation = false;
        teapotAngleX = 0.0f;
        teapotAngleY = 0.0f;
        teapotAngleZ = 0.0f;
        teapotPositionX = 0.0f;
        teapotPositionY = 0.0f;
        teapotPositionZ = 0.0f;
        
        nanosuitAngleX = 0.0f;
        nanosuitAngleY = 0.0f;
        nanosuitAngleZ = 0.0f;
        nanosuitPositionX = 0.0f;
        nanosuitPositionY = 0.25f;
        nanosuitPositionZ = -3.0f;
        
        myBasicShader.useShaderProgram();
        lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
        lightColorCoeff = 1.0f;
        glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "lightColorCoeff"), lightColorCoeff);
        mySkyBoxShader.useShaderProgram();
        glUniform1f(glGetUniformLocation(mySkyBoxShader.shaderProgram, "lightColorCoeff"), lightColorCoeff);

        lightRotationY = 0.0f;
        lightRotationZ = 0.0f;
    }

    // reset camera
    if (pressedKeys[GLFW_KEY_X]) {
        cameraPitch = 0.0f;
        cameraYaw = 0.0f;
        myCamera = gps::Camera(glm::vec3(0.0f, 0.0f, 3.0f),
            glm::vec3(0.0f, 0.0f, -10.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    }

    if (pressedKeys[GLFW_KEY_UP]) {
        cameraYaw += 1.0f;
        myCamera.rotate(cameraPitch, cameraYaw);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    }

    if (pressedKeys[GLFW_KEY_DOWN]) {
        cameraYaw -= 1.0f;
        myCamera.rotate(cameraPitch, cameraYaw);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    }

    if (pressedKeys[GLFW_KEY_LEFT]) {
        cameraPitch += 1.0f;
        myCamera.rotate(cameraPitch, cameraYaw);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    }

    if (pressedKeys[GLFW_KEY_RIGHT]) {
        cameraPitch -= 1.0f;
        myCamera.rotate(cameraPitch, cameraYaw);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    }

    // begin animations
    if (pressedKeys[GLFW_KEY_V]) {
        if (!teapotAnimation) {
            teapotAngleX = 0.0f;
            teapotAngleY = 0.0f;
            teapotAngleZ = 0.0f;
            teapotAnimation = true;
            teapotAnimationDirection = false;
        }
        if (!lightAnimation) {
            lightRotationY = 0.0f;
            lightRotationZ = 0.0f;
            lightAnimation = true;

            myBasicShader.useShaderProgram();
            glm::mat4 transform = glm::rotate(glm::mat4(1.0f), glm::radians(lightRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
            lightDir = glm::normalize(glm::vec3(transform * glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)));
            glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
            // based on sun's position
            lightColorCoeff = lightRotationZ > 180.0f ? -1.0f + (lightRotationZ / 180.0f) : 1.0f - (lightRotationZ / 180.0f);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "lightColorCoeff"), lightColorCoeff);
            mySkyBoxShader.useShaderProgram();
            glUniform1f(glGetUniformLocation(mySkyBoxShader.shaderProgram, "lightColorCoeff"), lightColorCoeff);
        }
        if (!nanosuitAnimation) {
            nanosuitAngleX = 0.0f;
            nanosuitAngleY = 270.0f;
            nanosuitAngleZ = 0.0f;

            nanosuitPositionX = 5.0f;
            nanosuitPositionY = 0.25f;
            nanosuitPositionZ = -3.0f;

            nanosuitAnimation = true;
            nanosuitAnimationDirection = false;
            nanosuitAnimationTurning = false;
        }

    }

    if (pressedKeys[GLFW_KEY_B]) {
        if (teapotAnimation) {
            teapotAngleX = 0.0f;
            teapotAngleY = 0.0f;
            teapotAngleZ = 0.0f;
            teapotAnimation = false;
            teapotAnimationDirection = false;
        }
        if (lightAnimation) {
            lightRotationY = 0.0f;
            lightRotationZ = 0.0f;
            lightAnimation = false;

            myBasicShader.useShaderProgram();
            glm::mat4 transform = glm::rotate(glm::mat4(1.0f), glm::radians(lightRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
            lightDir = glm::normalize(glm::vec3(transform * glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)));
            glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
            // based on sun's position
            lightColorCoeff = lightRotationZ > 180.0f ? -1.0f + (lightRotationZ / 180.0f) : 1.0f - (lightRotationZ / 180.0f);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "lightColorCoeff"), lightColorCoeff);
            mySkyBoxShader.useShaderProgram();
            glUniform1f(glGetUniformLocation(mySkyBoxShader.shaderProgram, "lightColorCoeff"), lightColorCoeff);
        }
        if (nanosuitAnimation) {
            nanosuitAngleX = 0.0f;
            nanosuitAngleY = 0.0f;
            nanosuitAngleZ = 0.0f;

            nanosuitPositionX = 0.0f;
            nanosuitPositionY = 0.25f;
            nanosuitPositionZ = -3.0f;

            nanosuitAnimation = false;
            nanosuitAnimationDirection = false;
            nanosuitAnimationTurning = false;
        }
    }

    if (pressedKeys[GLFW_KEY_M]) {
        switch (active_object) {
        case TEAPOT:
            teapotPositionX += 0.1f;
            break;
        case NANOSUIT:
            nanosuitPositionX += 0.1f;
            break;
        }
    }

    if (pressedKeys[GLFW_KEY_N]) {
        switch (active_object) {
        case TEAPOT:
            teapotPositionX -= 0.1f;
            break;
        case NANOSUIT:
            nanosuitPositionX -= 0.1f;
            break;
        }
    }

    if (pressedKeys[GLFW_KEY_H]) {
        switch (active_object) {
        case TEAPOT:
            teapotPositionZ += 0.1f;
            break;
        case NANOSUIT:
            nanosuitPositionZ += 0.1f;
            break;
        }
    }

    if (pressedKeys[GLFW_KEY_Y]) {
        switch (active_object) {
        case TEAPOT:
            teapotPositionZ -= 0.1f;
            break;
        case NANOSUIT:
            nanosuitPositionZ -= 0.1f;
            break;
        }
    }

    if (pressedKeys[GLFW_KEY_U]) {
        switch (active_object) {
        case TEAPOT:
            teapotPositionY += 0.1f;
            break;
        case NANOSUIT:
            nanosuitPositionY += 0.1f;
            break;
        }
    }

    if (pressedKeys[GLFW_KEY_J]) {
        switch (active_object) {
        case TEAPOT:
            teapotPositionY -= 0.1f;
            break;
        case NANOSUIT:
            nanosuitPositionY -= 0.1f;
            break;
        }
    }

    if (pressedKeys[GLFW_KEY_P]) {
        lightAnimation = false;

        glm::mat4 transform;
        switch (active_object) {
        case TEAPOT:
            lightRotationZ = 0.0f;
            lightRotationY += 1.0f;
            if (lightRotationY >= 360.0f)
                lightRotationY -= 360.0f;
            transform = glm::rotate(glm::mat4(1.0f), glm::radians(lightRotationY), glm::vec3(0.0f, 1.0f, 0.0f));
            lightDir = glm::normalize(glm::vec3(transform * glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)));
            myBasicShader.useShaderProgram();
            glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
            lightColorCoeff = 1.0f;
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "lightColorCoeff"), lightColorCoeff);
            mySkyBoxShader.useShaderProgram();
            glUniform1f(glGetUniformLocation(mySkyBoxShader.shaderProgram, "lightColorCoeff"), lightColorCoeff);
            break;
        case NANOSUIT:
            lightRotationY = 0.0f;
            lightRotationZ += 1.0f;
            if (lightRotationZ >= 360.0f)
                lightRotationZ -= 360.0f;
            myBasicShader.useShaderProgram();
            transform = glm::rotate(glm::mat4(1.0f), glm::radians(lightRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
            lightDir = glm::normalize(glm::vec3(transform * glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)));
            glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
            // change color if sun is under the ground
            lightColorCoeff = lightRotationZ > 180.0f ? -1.0f + (lightRotationZ / 180.0f) : 1.0f - (lightRotationZ / 180.0f);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "lightColorCoeff"), lightColorCoeff);
            mySkyBoxShader.useShaderProgram();
            glUniform1f(glGetUniformLocation(mySkyBoxShader.shaderProgram, "lightColorCoeff"), lightColorCoeff);
            break;
        }
    }

    if (pressedKeys[GLFW_KEY_1]) {
        fprintf(stdout, "Active object: teapot\n");
        active_object = TEAPOT;
    }

    if (pressedKeys[GLFW_KEY_2]) {
        fprintf(stdout, "Active object: nanosuit\n");
        active_object = NANOSUIT;
    }

    if (pressedKeys[GLFW_KEY_C]) {
        fogEnabled = !fogEnabled;
        myBasicShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "fogEnabled"), fogEnabled);
        mySkyBoxShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(mySkyBoxShader.shaderProgram, "fogEnabled"), fogEnabled);
    }

    if (pressedKeys[GLFW_KEY_O]) {
        if (!wireframeEnable) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        wireframeEnable = !wireframeEnable;
    }
}

void initOpenGLWindow() {
    myWindow.Create(1024, 768, "OpenGL Project Core");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    teapot.LoadModel("models/teapot/teapot20segUT.obj");
    ground.LoadModel("models/ground/ground.obj");
    nanosuit.LoadModel("models/nanosuit/nanosuit.obj");

    std::vector<const GLchar*> faces;
    faces.push_back("models/skybox/right.tga");
    faces.push_back("models/skybox/left.tga");
    faces.push_back("models/skybox/top.tga");
    faces.push_back("models/skybox/bottom.tga");
    faces.push_back("models/skybox/back.tga");
    faces.push_back("models/skybox/front.tga");
    mySkyBox.Load(faces);

}

void initShaders() {
	myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
    myShadowShader.loadShader(
        "shaders/shadow.vert",
        "shaders/shadow.frag"
    );
    mySkyBoxShader.loadShader(
        "shaders/skyboxShader.vert",
        "shaders/skyboxShader.frag"
    );
}

void initUniforms() {
	myBasicShader.useShaderProgram();

    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(teapotAngleX), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(teapotAngleY), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(teapotAngleZ), glm::vec3(0.0f, 0.0f, 1.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
	projection = glm::perspective(glm::radians(45.0f),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               0.1f, 20.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    // set shadow stuff
    lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    lightProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, near_plane, far_plane);
    lightSpaceTrMatrix = lightProjection * lightView;
    lightSpaceTrMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix");
    // send lightSpaceTrMatrix
    glUniformMatrix4fv(lightSpaceTrMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightSpaceTrMatrix));

    myShadowShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(myShadowShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceTrMatrix));

    // fog and daylight intensity
    lightColorCoeff = 1.0f;
    fogEnabled = false;
    myBasicShader.useShaderProgram();
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "fogEnabled"), fogEnabled);
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "lightColorCoeff"), lightColorCoeff);
    mySkyBoxShader.useShaderProgram();
    glUniform1i(glGetUniformLocation(mySkyBoxShader.shaderProgram, "fogEnabled"), fogEnabled);
    glUniform1f(glGetUniformLocation(mySkyBoxShader.shaderProgram, "lightColorCoeff"), lightColorCoeff);
}

void initFBO() {
    // Create the FBO, the depth texture and attach the depth texture to the FBO

    //generate FBO ID
    glGenFramebuffers(1, &shadowMapFBO);

    // create depth texture for FBO
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT,
        0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    //attach texture to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void playAnimations() {
    if (teapotAnimation) {
        if (teapotAngleZ >= 45.0f || teapotAngleZ <= -45.0f)
            teapotAnimationDirection = !teapotAnimationDirection;
        if (!teapotAnimationDirection)
            teapotAngleZ += 1.0f;
        else
            teapotAngleZ -= 1.0f;
    }
    if (lightAnimation) {
        lightRotationZ += 0.1f;
        if (lightRotationZ >= 360.0f)
            lightRotationZ -= 360.0f;
        myBasicShader.useShaderProgram();
        glm::mat4 transform = glm::rotate(glm::mat4(1.0f), glm::radians(lightRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
        lightDir = glm::normalize(glm::vec3(transform * glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)));
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
        // based on sun's position
        lightColorCoeff = lightRotationZ > 180.0f ? -1.0f + (lightRotationZ / 180.0f) : 1.0f - (lightRotationZ / 180.0f);
        glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "lightColorCoeff"), lightColorCoeff);
        mySkyBoxShader.useShaderProgram();
        glUniform1f(glGetUniformLocation(mySkyBoxShader.shaderProgram, "lightColorCoeff"), lightColorCoeff);
    }
    if (nanosuitAnimation) {
        if (!nanosuitAnimationTurning) {
            if (!nanosuitAnimationDirection) {
                nanosuitPositionX -= 0.01f;
                if (nanosuitPositionX <= -5.0f) {
                    nanosuitAnimationTurning = true;
                }
            }
            else {
                nanosuitPositionX += 0.02f;
                if (nanosuitPositionX >= 5.0f) {
                    nanosuitAnimationTurning = true;
                }
            }
        }
        else {
            nanosuitAngleY += 1.0f;
            if (nanosuitAngleY >= 360.0f) {
                nanosuitAngleY -= 360.0f;
            }
            if (nanosuitAngleY == 90.0f || nanosuitAngleY == 270.0f) {
                nanosuitAnimationTurning = false;
                nanosuitAnimationDirection = !nanosuitAnimationDirection;
            }
        }
    }
}

void renderTeapot(gps::Shader shader, bool depthPass) {
    // select active shader program
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(teapotPositionX, teapotPositionY, teapotPositionZ));
    model = glm::rotate(model, glm::radians(teapotAngleX), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(teapotAngleY), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(teapotAngleZ), glm::vec3(0.0f, 0.0f, 1.0f));

    //send teapot model matrix data to shader
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"),
        1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"),
            1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    // draw teapot
    teapot.Draw(shader);
}

void renderNanosuit(gps::Shader shader, bool depthPass) {
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(nanosuitPositionX, nanosuitPositionY, nanosuitPositionZ));
    model = glm::rotate(model, glm::radians(nanosuitAngleX), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(nanosuitAngleY), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(nanosuitAngleZ), glm::vec3(0.0f, 0.0f, 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"),
        1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"),
            1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    // draw teapot
    nanosuit.Draw(shader);
}

void renderGround(gps::Shader shader, bool depthPass) {
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.75f, 0.0f));
    model = glm::scale(model, glm::vec3(1.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"),
        1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"),
            1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    ground.Draw(shader);
}

void renderSkyBox(gps::Shader shader) {
    shader.useShaderProgram();

    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "view"), 1, GL_FALSE,
        glm::value_ptr(view));

    projection = glm::perspective(glm::radians(45.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 1000.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "projection"), 1, GL_FALSE,
        glm::value_ptr(projection));

    mySkyBox.Draw(shader, view, projection);
}

void renderObjects(gps::Shader shader, bool depthPass) {
    // render the teapot
    renderTeapot(shader, depthPass);

    // render the nanosuit
    renderNanosuit(shader, depthPass);

   // render the ground
    renderGround(shader, depthPass);

    // render skybox
    renderSkyBox(mySkyBoxShader);
}

glm::mat4 computeLightSpaceTrMatrix() {
    lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    lightSpaceTrMatrix = lightProjection * lightView;
    return lightSpaceTrMatrix;
}

void renderScene() {

    playAnimations();

    // Prepare the shadows
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    myShadowShader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(myShadowShader.shaderProgram, "lightSpaceTrMatrix"),
        1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    renderObjects(myShadowShader, true);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Draw with shadows
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    myBasicShader.useShaderProgram();

    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
        1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
   
    //bind the shadow map
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

    renderObjects(myBasicShader, false);

}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
    initFBO();
    setWindowCallbacks();

	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
	    renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}

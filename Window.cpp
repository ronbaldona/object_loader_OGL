#include "Window.h"

#include <iostream>

#include "PrintDebug.h"

#include "Object.h"
#include "Model.h"
#include "Camera.h"
#include "Skybox.h"
#include "Shader.h"
#include "OBJObject.h"
#include "Ground.h"
#include "DirLight.h"
#include "SPointLight.h"

namespace {
	// TODO: CLEAN UP BY ADDING SIMPLE FILE LOADING SYSTEM
	std::string objPath = "Models/source/robot.obj";
	// test objects
	Object* testObj;
	Object* testQuad;
	Skybox* skybox;
	Ground* ground;
	DirLight* testDLight;
	SPointLight* testPLight;
	Light* currLight;

	Shader* skyboxShader;
	Shader* testShader;
	Shader* depthShader;
	Shader* screenShader;

	// Camera/Window variables
	int wWidth, wHeight;
	Camera mainCam;
	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 lightSpaceTransfMat;

	// Trackball mode variables
	bool lmbPressed = false;
	double oldPos[2];
	glm::vec3 oldPosVec;

	// FPS camera mode variables
	glm::vec2 lastCursorPos;

	// Modes
	enum CAMERA_MODES{ OBJECT_MODE, FPS_MODE };
	int CURR_CAM_MODE = OBJECT_MODE;
	int NUM_CAM_MODES = 2;

	// Frames per second tracking
	double deltaTime = 0.0f;

	// Framebuffer
}

/// <summary>
/// Changes xpos and ypos in window space where origin is at the upper left
/// corner to the center of the screen. Normalizes coordinates based on
/// smallest dimension of window
/// </summary>
/// <param name="xpos"> x position of cursor. A return parameter </param>
/// <param name="ypos"> y position of cursor. A return parameter </param>
inline void cursorPosChangeBasis(double& xpos, double& ypos) {
	double smallDim = (wWidth > wHeight) ? wHeight : wWidth;
	xpos = (xpos - (double)(0.5 * wWidth)) / (double)(0.5 * smallDim);
	ypos = ((double)(0.5 * wHeight) -  ypos) / (double)(0.5 * smallDim);
}

/// <summary>
/// Projects cursor onto unit sphere superimposed onto window
/// </summary>
/// <param name="xpos"> x position of cursor before basis change </param>
/// <param name="ypos"> y position of cursor before basis change </param>
/// <returns></returns>
inline glm::vec3 projectCursorOntoSphere(double xpos, double ypos) {
	cursorPosChangeBasis(xpos, ypos);

	// clip 2d vector at edge of unit circle on xy plane
	double d = sqrt(xpos * xpos + ypos * ypos);
	if (d > 1) {
		xpos /= d;
		ypos /= d;
		d = 1;
	}
	
	// Now clip 3d vector at edge of unit hemisphere 
	return glm::vec3(xpos, ypos, sqrt(1 - d));
}

Window::Window() {
	wWidth = 800;
	wHeight = 600;

	std::cout << "Initializing window of size " << wWidth << " x ";
	std::cout << wHeight << std::endl;

	windowptr = initGLFWWindowSettings(wWidth, wHeight);
	initGLFWcallbacks();
	view = mainCam.getViewMat();
	projection = mainCam.getProjMat(wWidth, wHeight);

}

Window::~Window() {
	if (windowptr != nullptr) {
		glfwTerminate();
		windowptr = nullptr;
	}
}

Window::Window(int _width, int _height) {
	wWidth = _width;
	wHeight = _height;

	std::cout << "Initializing window of size " << wWidth << " x ";
	std::cout << wHeight << std::endl;

	windowptr = initGLFWWindowSettings(wWidth, wHeight);
	initGLFWcallbacks();
	view = mainCam.getViewMat();
	projection = mainCam.getProjMat(wWidth, wHeight);
}

GLFWwindow* Window::initGLFWWindowSettings(int width, int height) {
	// Initializes GLFW library
	glfwInit();

	// Establishes the version type of OpenGL (3.3 core)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(width, height, "Test Window", NULL, NULL);
	if (window == nullptr) {
		std::cout << "Failed to create GLFW window\n";
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	std::cout << glfwGetVersionString() << std::endl;

	// More settings
	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	else
		std::cout << "Raw mouse motion not supported\n";

	lastCursorPos = glm::vec2(0.5f * width, 0.5f * height);

	return window;
}

GLFWwindow* Window::getWindowptr() const {
	return windowptr;
}

void Window::setObjToView(const std::string& path) {
	objPath = path;
}

void Window::initializeScene() {
	// Initialize objects
	testObj = new Model(objPath.c_str());
	skybox = new Skybox();
	ground = new Ground();
	testQuad = new Model("Models/quad.obj");

    testDLight = new DirLight();
	testPLight = new SPointLight();

	// Initialize shaders
	testShader = new Shader("Shaders/test.vert", "Shaders/test.frag");
	skyboxShader = new Shader("Shaders/Skybox.vert", "Shaders/Skybox.frag");
	depthShader = new Shader("Shaders/DepthShader.vert", 
		                     "Shaders/DepthShader.frag");
	screenShader = new Shader("Shaders/ScreenQuad.vert",
		                      "Shaders/ScreenQuad.frag");
	
	// Basic light space tester
	glm::mat4 lightProjMat = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 
		                                0.5f, 50.0f);
	glm::mat4 lightView = glm::lookAt(glm::vec3(5, 5, 0), 
		                              glm::vec3(0),
		                              glm::vec3(0, 1, 0));
	lightSpaceTransfMat = lightProjMat * lightView;
}

void Window::cleanUpScene() {
	std::cout << "Deleting scene objects\n";
	// Clean up models
	delete testObj;
	delete skybox;
	delete ground;
	delete testQuad;
	delete testDLight;
	delete testPLight;

	// Clean up shaders
	testShader->deleteShader();
	delete testShader;
	skyboxShader->deleteShader();
	delete skyboxShader;
	depthShader->deleteShader();
	delete depthShader;
	screenShader->deleteShader();
	delete screenShader;

}

void Window::framebuffer_size_callback(GLFWwindow* window, int width, 
	int height) {
	wWidth = width;
	wHeight = height;
	lastCursorPos = glm::vec2(0.5f * width, 0.5f * height);
	projection = mainCam.getProjMat(width, height);
	glViewport(0, 0, width, height);
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, 
	int action, int mods) {
	// TODO: Add caps lock check
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			std::cout << "Closing window\n";
			glfwSetWindowShouldClose(window, true);
			return;
		case GLFW_KEY_R:
			testObj->reset();
			break;
		case GLFW_KEY_UP:
			testObj->translate(0, 1.0f, 0);
			break;
		case GLFW_KEY_DOWN:
			testObj->translate(0, -1.0f, 0);
			break;
		case GLFW_KEY_LEFT:
			testObj->translate(-1.0f, 0, 0);
			break;
		case GLFW_KEY_RIGHT:
			testObj->translate(1.0f, 0, 0);
			break;
		case GLFW_KEY_C:
			CURR_CAM_MODE = ++CURR_CAM_MODE % NUM_CAM_MODES;
			std::cout << "CAM MODE: " << CURR_CAM_MODE << std::endl;
			if (CURR_CAM_MODE == FPS_MODE)
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			else
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		
			break;
		case GLFW_KEY_P:
			// just some debug info
			std::cout << "FPS: " << 1.0f / deltaTime << std::endl;
			errorHandler::printGlError();
			std::cout << std::endl;
			break;
		
		// Camera light options
		case GLFW_KEY_F1:
			// DIRECTIONAL LIGHT
			currLight = testDLight;
			break;
		case GLFW_KEY_F2:
			// POINT LIGHT
			currLight = testPLight;
			break;
		}

		// lower case
		if (mods != GLFW_MOD_SHIFT) {
			switch (key) {
			case GLFW_KEY_S:
				if (CURR_CAM_MODE != FPS_MODE)
				    testObj->scale(glm::vec3(0.9f));
				break;
			default:
				break;
			}
		}
		// upper case
		else {
			switch (key) {
			case GLFW_KEY_S:
				if (CURR_CAM_MODE != FPS_MODE)
				    testObj->scale(glm::vec3(1.1f));
				break;
			default:
				break;
			}
		}
	}

}

void Window::cursor_position_callback(GLFWwindow* window, double xpos, 
	double ypos) {
	// Get new pos in new basis
	if (lmbPressed && CURR_CAM_MODE == OBJECT_MODE) {
		// Project both new and old positions to unit sphere
		glm::vec3 newPosVec = projectCursorOntoSphere(xpos, ypos);

		// Retrieve angle and axis then rotate
		glm::vec3 axis = glm::cross(oldPosVec, newPosVec);
		//std::cout << "Axis of rotation: " << axis.x << " " << axis.y << " " << axis.z << std::endl;
		if (length(axis) > 1e-4f) {
			float dot = glm::dot(oldPosVec, newPosVec) / 
				(glm::length(oldPosVec) * glm::length(newPosVec));
			float angle = acosf(dot) / 15.0f;

			testObj->rotate(angle, axis);
		}
	}

	if (CURR_CAM_MODE == FPS_MODE) {
		mainCam.rotateCamFromMouseMove((float)(lastCursorPos.x - xpos),
			                           (float)(lastCursorPos.y - ypos));
		view = mainCam.getViewMat();
		lastCursorPos = glm::vec2((float)xpos, (float)ypos);
	}

}

void Window::mouse_button_callback(GLFWwindow* window, int button, int action,
	int mods) {
	//TODO
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (CURR_CAM_MODE == OBJECT_MODE) {
			if (action == GLFW_PRESS) {
				// Set cursor and motion modes
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

				// Get cursor position and unit sphere projection on button press
				lmbPressed = true;
				glfwGetCursorPos(window, oldPos, &oldPos[1]);
				oldPosVec = projectCursorOntoSphere(oldPos[0], oldPos[1]);

			}
			else if (action == GLFW_RELEASE) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				lmbPressed = false;
			}
		}
	}

}

void Window::scroll_callback(GLFWwindow* window, double xoffset,
	double yoffset) {
	testObj->translate(glm::vec3(0, 0, -yoffset));
}

void Window::processKeyInput() const {
	if (CURR_CAM_MODE == FPS_MODE) {
		float camSpeed = (float)deltaTime * 10;
		if (glfwGetKey(windowptr, GLFW_KEY_W) == GLFW_PRESS) {
			glm::vec3 dir = glm::normalize(mainCam.getCenter() - mainCam.getEye());
			mainCam.moveCam(dir * camSpeed);
			view = mainCam.getViewMat();
		}
		if (glfwGetKey(windowptr, GLFW_KEY_A) == GLFW_PRESS) {
			glm::vec3 dir = glm::normalize(mainCam.getCenter() - mainCam.getEye());
			mainCam.moveCam(-glm::normalize(glm::cross(dir, mainCam.getUp())) * camSpeed);
			view = mainCam.getViewMat();
		}
		if (glfwGetKey(windowptr, GLFW_KEY_S) == GLFW_PRESS) {
			glm::vec3 dir = glm::normalize(mainCam.getCenter() - mainCam.getEye());
			mainCam.moveCam(-dir * camSpeed);
			view = mainCam.getViewMat();
		}
		if (glfwGetKey(windowptr, GLFW_KEY_D) == GLFW_PRESS) {
			glm::vec3 dir = glm::normalize(mainCam.getCenter() - mainCam.getEye());
			mainCam.moveCam(glm::normalize(glm::cross(dir, mainCam.getUp())) * camSpeed);
			view = mainCam.getViewMat();
		}
	}

}

void Window::initGLFWcallbacks() {
	//glfwSetErrorCallback(error_callback);
	glfwSetKeyCallback(windowptr, key_callback);
	glfwSetFramebufferSizeCallback(windowptr, framebuffer_size_callback);
	glfwSetCursorPosCallback(windowptr, cursor_position_callback);
	glfwSetMouseButtonCallback(windowptr, mouse_button_callback);
	glfwSetScrollCallback(windowptr, scroll_callback);
}

void Window::render() {
	double currTime = glfwGetTime();

	// Clear color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Render scene to depth buffer first
	testDLight->startRenderToDepthMap();
	depthShader->use();
	depthShader->setMat4("lightTransform", lightSpaceTransfMat);
	glCullFace(GL_FRONT);
	testObj->draw(*depthShader, view, projection);
	glCullFace(GL_BACK);
	ground->draw(*depthShader, view, projection);
	testDLight->endRenderToDepthMap(wWidth, wHeight);

	// Set up lights here for now...HACKY
	testShader->use();
	testDLight->dataToShader(*testShader);
	testShader->setInt("numDirLights", 1);

	// For shadow mapping
	testShader->setMat4("lightTransform", lightSpaceTransfMat);
	glActiveTexture(GL_TEXTURE0);
	testDLight->bindDepthMapTexture();
	testShader->setInt("depthMap", 0);
	//testPLight->dataToShader(*testShader);
	//testShader->setInt("numSPointLights", 1);

	// Render the texture to screen?
	/*
	glActiveTexture(GL_TEXTURE0);
	testDLight->bindDepthMapTexture();
	testQuad->draw(*screenShader, view, projection);
	glBindTexture(GL_TEXTURE_2D, 0);
	*/

	testObj->draw(*testShader, view, projection);
	ground->draw(*testShader, view, projection);

	// Skybox gets used last
	skybox->draw(*skyboxShader, view, projection);
	// Check for events and swap buffers
	glfwSwapBuffers(windowptr);
	glfwPollEvents(); // LOOK THIS UP LATER

	deltaTime = glfwGetTime() - currTime;
}

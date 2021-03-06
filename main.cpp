#include "main.h"

#define DEBUG_MODE
#ifdef DEBUG_MODE
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#endif

inline std::string printUsageStatement() {
	std::string usage = "===========\nUSAGE\n===========\n";
	usage += "\t [-h] [-w width height] obj\n";
	return usage;

}

// Initializes GLAD for OpenGL function uses
int initOpenGLGlad() {
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD\n";
		return EXIT_FAILURE;
	}

	std::cout << "OpenGL version is " << glGetString(GL_VERSION) << std::endl;

	return EXIT_SUCCESS;
}

// Initializes various OpenGL settings
void initOpenGLSettings() {
	// Enable z-buffer based depth testing
	glEnable(GL_DEPTH_TEST);
	// Enable face culling
	glEnable(GL_CULL_FACE);
	glViewport(0, 0, 800, 600);
	glClearColor(0, 0, 0, 0);
}

int main(int argc, char* argv[]) {
#ifdef DEBUG_MODE
	std::cout << "MEMORY DEBUG MODE IS ON\n\n";
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::string objToLoad = TEST_OBJ;
	int width = STND_WIDTH;
	int height = STND_HEIGHT;

	if (argc > MAX_NUM_USAGE) {
		std::cout << "Error: Number of given arguments not supported!\n";
		std::cout << printUsageStatement << std::endl;
		exit(EXIT_FAILURE);
	}

	// Parse user usage arguments
	// For now only deals with reading one scene object file
	for (int i = 1; i < argc; ++i) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
			case 'h':
				settings |= PRINT_HELP_BIT;
				break;
			case 'w':
				width = atoi(argv[i + 1]);
				height = atoi(argv[i + 2]);
				// move to next option
				i += 2;
				break;
			}

		}
		// Assuming object name is a file name
		else {
			objToLoad = std::string(argv[i]);
		}
	}

	// Deal with settings
	if (settings & PRINT_HELP_BIT)
		std::cout << printUsageStatement();

	std::cout << "Object to view: " << objToLoad << std::endl;

	// Make sure to make the window BEFORE initializing GLAD
	Window mainWindow = Window(width, height);

	// Initialize GLAD
	if (initOpenGLGlad() == EXIT_FAILURE)
		return EXIT_FAILURE;

	// Other OpenGL settings
	initOpenGLSettings();

	// Initialize object in scene
	Window::setObjToView(objToLoad);
	Window::initializeScene();

	// Main render loop
	while (!glfwWindowShouldClose(mainWindow.getWindowptr())) {
		glEnable(GL_PROGRAM_POINT_SIZE);
		mainWindow.render();
		mainWindow.processKeyInput();
	}

	Window::cleanUpScene();

	return EXIT_SUCCESS;
}
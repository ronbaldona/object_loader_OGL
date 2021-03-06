/*  Implementation of a basic static skybox
    - RAB
 */

#pragma once

#include <glad/glad.h>

#include "Object.h"
#include "Shader.h"

class Skybox : public Object
{
	// Vertices define a length 1 cube centered at the world origin
	static const glm::vec3 vertices[8];
	// Indices are defined so that the inside of the cube is forward (CW)
	static const glm::ivec3 indices[12];

	// Buffer/VAO objects
	GLuint VAO, VBO, EBO, texId;

	/// <summary>
	/// Loads texture from a given directory path.
	/// Below is the expected structure for each skybox
	///   * Each texture file will have the last 3 letters of its name
	///     represent which face the texture represents (_lf = left)
	///     with the first char _ being the delimiter
	///   * There will be 6 faces
	/// </summary>
	/// <param name="path"> File path name of object </param>
	/// <returns> True if successful, Otherwise false </returns>
	bool load(const char* path);

	/// <summary>
    /// Initializes buffer objects for rendering
    /// </summary>
	void init();

public:

	enum CUBE_FACES : int {
		POSITIVE_X, NEGATIVE_X,
		POSITIVE_Y, NEGATIVE_Y,
		POSITIVE_Z, NEGATIVE_Z
	};

	/// <summary>
	/// Initializes standard skybox from Skybox/Default directory
	/// </summary>
	Skybox();
	
	// Deletes skybox members
	~Skybox();

	/// <summary>
	/// Initializes standard skybox from a given directory path
	/// </summary>
	/// <param name="path"> path name of directory containing skybox textures
	/// </param>
	Skybox(const char* path);

	/// <summary>
	/// ONLY HERE TO CONFORM TO OBJECT CLASS
	/// </summary>
	/// <param name="program"></param>
	void sendMatToShader(const Shader& program) const {}

	/// <summary>
    /// Draw skybox to screen.
    /// </summary>
    /// <param name="program"> Shader program </param>
    /// <param name="view"> view matrix </param>
    /// <param name="projection"> projection matrix </param>
	void draw(const Shader& program, glm::mat4 view, glm::mat4 projection);

};


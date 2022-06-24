#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "scene.h"

char* readShaderFile(char* path);

unsigned int compileShader(unsigned int shader);

unsigned int getSceneVertexShader(char* path, Scene scene);

unsigned int getShader(char* path, unsigned int type);

void linkShaders(unsigned int shaderProgram, unsigned int vertexShader, unsigned int fragmentShader);
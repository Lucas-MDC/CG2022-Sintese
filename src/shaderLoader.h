#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

char* readShaderFile(char* path);

unsigned int compileShader(unsigned int shader);

unsigned int getShader(char* path, unsigned int type);

void linkShaders(unsigned int shaderProgram, unsigned int vertexShader, unsigned int fragmentShader);
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shaderLoader.h"
#include "algebra.h"
#include "scene.h"

#define true  1
#define false 0

typedef struct openGlEnv
{
    GLFWwindow* window;
    unsigned int width;
    unsigned int height;
} openGlEnv;

int mainRender(Scene scene);

void algebraTest();
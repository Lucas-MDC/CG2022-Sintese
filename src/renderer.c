#include "renderer.h"



void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

openGlEnv envInitialize(unsigned int width, unsigned int height)
{
    // Inicializando glfw 
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criando uma janela
    GLFWwindow* window = glfwCreateWindow(width, height, "Sintese", NULL, NULL);
    assert(window != NULL);
    glfwMakeContextCurrent(window);

    // Inicializando GLAD
    assert(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress));
    glViewport(0, 0, width, height);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    return (openGlEnv){window, width, height};
}

void inputHandler(GLFWwindow* window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

/*
void render(openGlEnv* env, unsigned int shaderProgram, unsigned int* VAO)
{

}
*/

unsigned int getShaderProgram(Scene scene)
{
    unsigned int vertexShader   = getSceneVertexShader("./vertex.glsl", scene);
    unsigned int fragmentShader = getShader("./fragment.glsl", GL_FRAGMENT_SHADER);
    unsigned int shaderProgram  = glCreateProgram();
    linkShaders(shaderProgram, vertexShader, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int mainRender(Scene scene)
{
    int width  = 1920;
    int height = 1080;

    openGlEnv env              = envInitialize(scene.observer.width, scene.observer.height);
    unsigned int shaderProgram = getShaderProgram(scene);

    float* dislocations           = getPixelDislocations(scene.observer.width, scene.observer.height);
    float* pixelVertexCoordinates = getPixelVertexCoordinates(pixelVertexCoordinates, env.width, env.height);

    unsigned int vertexDataSize = sizeof(float)*env.width*env.height*5;
    float*       vertexData     = malloc(vertexDataSize);

    // Writting pixel coordinates into the buffer
    int pos = 0;
    for(int i = 0; i < env.height*env.width*3; i+=3)
    {
        vertexData[pos + 0] = pixelVertexCoordinates[i  ];
        vertexData[pos + 1] = pixelVertexCoordinates[i+1];
        vertexData[pos + 2] = pixelVertexCoordinates[i+2];
        pos += 5;
    }

    // Writting pixel horizontal and vertical versors dislocations into the buffer
    pos = 3;
    for(int i = 0; i < env.width*env.height*2; i+=2)
    {
        vertexData[pos    ] = dislocations[i  ];
        vertexData[pos + 1] = dislocations[i+1];
        pos += 5;
    }

    unsigned int VBO;
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    float angle = 0.0;
    while(!glfwWindowShouldClose(env.window))
    {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertexDataSize, vertexData, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3* sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        inputHandler(env.window);
        glUseProgram(shaderProgram);

        int loc = 0;

        // New
        loc = glGetUniformLocation(shaderProgram, "observer");
        glUniform1fv(loc, sizeof(scene.observer), (float*)&scene.observer);

        loc = glGetUniformLocation(shaderProgram, "ambientLight");
        glUniform3fv(loc, 1, (float*)&scene.ambientLight);

        loc = glGetUniformLocation(shaderProgram, "lightSources");
        glUniform1fv(loc, sizeof(LightSource)*scene.lightSourceNumber, scene.lightSources);

        loc = glGetUniformLocation(shaderProgram, "shapeLocations");
        glUniform1iv(loc, scene.geometryObjectsNumber, scene.geometryObjectsShapeLocations);

        loc = glGetUniformLocation(shaderProgram, "shapeTypes");
        glUniform1iv(loc, scene.geometryObjectsNumber, scene.geometryObjectsTypes);

        loc = glGetUniformLocation(shaderProgram, "shapeColors");
        glUniform1fv(loc, sizeof(GeometryColor)*scene.geometryObjectsNumber, scene.geometryObjectsColor);

        loc = glGetUniformLocation(shaderProgram, "shapes");
        glUniform1fv(loc, scene.geometryObjectsTotalSize, scene.geometryObjectsShapes);

        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, env.height*env.width);
        glDrawArrays(GL_POINTS, 0, 1);
        glfwSwapBuffers(env.window);
        glfwPollEvents();
    }

    free((void*)dislocations);
    free((void*)pixelVertexCoordinates);
    free((void*)vertexData);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();

    return 0;
}
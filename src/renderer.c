#include "renderer.h"
#define true  1
#define false 0

void window_size_callback(GLFWwindow* window, int width, int height)
{
    glfwSetWindowSize(window, width, height);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

GLFWwindow* envInitialize(unsigned int width, unsigned int height)
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

    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    return window;
}

void moveObserverPosition(Scene* scene, float dc, float dh, float dv)
{
    scene->observer.xOrigin += scene->observer.xCenterVersor*dc;
    scene->observer.yOrigin += scene->observer.yCenterVersor*dc;
    scene->observer.zOrigin += scene->observer.zCenterVersor*dc;

    scene->observer.xOrigin += scene->observer.xHorizontalVersor*dh;
    scene->observer.yOrigin += scene->observer.yHorizontalVersor*dh;
    scene->observer.zOrigin += scene->observer.zHorizontalVersor*dh;

    scene->observer.xOrigin += scene->observer.xVerticalVersor*dv;
    scene->observer.yOrigin += scene->observer.yVerticalVersor*dv;
    scene->observer.zOrigin += scene->observer.zVerticalVersor*dv;

    scene->observer.xObserved = scene->observer.xOrigin + scene->observer.xCenterVersor*1;
    scene->observer.yObserved = scene->observer.yOrigin + scene->observer.yCenterVersor*1;
    scene->observer.zObserved = scene->observer.zOrigin + scene->observer.zCenterVersor*1;

    /*
    scene->observer.xOrigin += scene->observer.xHorizontalVersor*dh;
    scene->observer.yOrigin += scene->observer.yHorizontalVersor*dh;
    scene->observer.zOrigin += scene->observer.zHorizontalVersor*dh;

    scene->observer.xOrigin += scene->observer.xVerticalVersor*dv;
    scene->observer.yOrigin += scene->observer.yVerticalVersor*dv;
    scene->observer.zOrigin += scene->observer.zVerticalVersor*dv;
    */

    vec3 centerVersor     = getScreenCenterVersor((vec3){scene->observer.xOrigin, scene->observer.yOrigin, scene->observer.zOrigin}, (vec3){scene->observer.xObserved, scene->observer.yObserved, scene->observer.zObserved});
    vec3 horizontalVersor = getScreenHorizontalVersor(centerVersor);
    vec3 verticalVersor   = getScreenVerticalVersor(centerVersor, horizontalVersor);

    scene->observer.xCenterVersor = centerVersor.x;
    scene->observer.yCenterVersor = centerVersor.y;
    scene->observer.zCenterVersor = centerVersor.z;
    scene->observer.xHorizontalVersor = horizontalVersor.x;
    scene->observer.yHorizontalVersor = horizontalVersor.y;
    scene->observer.zHorizontalVersor = horizontalVersor.z;
    scene->observer.xVerticalVersor   = verticalVersor.x;
    scene->observer.yVerticalVersor   = verticalVersor.y;
    scene->observer.zVerticalVersor   = verticalVersor.z;
}

void movementHandler(GLFWwindow* window, Scene* scene)
{
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        moveObserverPosition(scene, 0.1, 0.0, 0.0);   
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)    
        moveObserverPosition(scene, -0.1, 0.0, 0.0);  
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)    
        moveObserverPosition(scene, 0.0, -0.1, 0.0);  
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)    
        moveObserverPosition(scene, 0.0, 0.1, 0.0);  
}

void inputHandler(GLFWwindow* window, Scene* scene)
{
    movementHandler(window, scene);

    int width;
    int height;

    glfwGetWindowSize(window, &width, &height);
    scene->observer.width  = width;
    scene->observer.height = height;
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

void printFPSEverySecond(clock_t tic, clock_t toc, double* acc, int* iteration, int* next)
{
    acc[0] += 10000*((double)(tic - toc)/CLOCKS_PER_SEC);

    if(fmod(acc[0], (double)((int)acc[0])) < 0.05 && (int)acc[0] == next[0])
    {
        printf("FPS: %i, Time: %f\n", iteration[0], acc[0]);
        next[0] +=1;
        iteration[0]    = 0;
    }
}

float* getVertaxBufferWithPixelData(Scene scene, unsigned int vertexDataSize)
{
    float* vertexData             = malloc(vertexDataSize);
    float* pixelVertexCoordinates = getPixelVertexCoordinates(pixelVertexCoordinates, scene.observer.width, scene.observer.height);
    float* dislocations           = getPixelDislocations(scene.observer.width, scene.observer.height);

    // Writting pixel coordinates into the buffer
    int pos = 0;
    for(int i = 0; i < scene.observer.height*scene.observer.width*3; i+=3)
    {
        vertexData[pos + 0] = pixelVertexCoordinates[i  ];
        vertexData[pos + 1] = pixelVertexCoordinates[i+1];
        vertexData[pos + 2] = pixelVertexCoordinates[i+2];
        pos += 5;
    }

    // Writting pixel horizontal and vertical versors dislocations into the buffer
    pos = 3;
    for(int i = 0; i < scene.observer.width*scene.observer.height*2; i+=2)
    {
        vertexData[pos    ] = dislocations[i  ];
        vertexData[pos + 1] = dislocations[i+1];
        pos += 5;
    }

    free((void*)dislocations);
    free((void*)pixelVertexCoordinates);
    return vertexData;
}

int mainRender(Scene scene)
{
    GLFWwindow* window         = envInitialize(scene.observer.width, scene.observer.height);
    unsigned int shaderProgram = getShaderProgram(scene);
    float  vertexDataSize = sizeof(float)*scene.observer.width*scene.observer.height*5;
    float* vertexData     = getVertaxBufferWithPixelData(scene, vertexDataSize);

    unsigned int VBO;
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    int i = 0;
    clock_t tic  = clock();
    clock_t toc  = clock();
    double  acc  = 0;
    int     next = 1;

    while(!glfwWindowShouldClose(window))
    {
        tic = clock();
        i++;

        printFPSEverySecond(tic, toc, &acc, &i, &next);

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

        inputHandler(window, &scene);
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
        glDrawArrays(GL_POINTS, 0, scene.observer.height*scene.observer.width);
        glDrawArrays(GL_POINTS, 0, 1);
        glfwSwapBuffers(window);
        glfwPollEvents();

        toc = clock();
    }

    free((void*)vertexData);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();

    return 0;
}
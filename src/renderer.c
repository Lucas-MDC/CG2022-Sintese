#include "renderer.h"

float normalize(float x, float size)
{
    return x/size;
}

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

void render(openGlEnv* env, unsigned int shaderProgram, unsigned int* VAO)
{

}

float* loadVertexBuffer(unsigned int width, unsigned int height)
{
    float* vertices = (float*)malloc(sizeof(float)*height*width*6);

    int pos = 0;
    for (int h = (int)-(height)/2; h < (int)(height)/2; h++)
    {
        for(int w = (int)-(width)/2; w < (int)(width)/2; w++)
        {
            vertices[pos    ] = normalize(w, width/2);
            vertices[pos + 1] = normalize(h, height/2);
            vertices[pos + 2] = 0;
            vertices[pos + 3] = normalize(w, width/2);
            vertices[pos + 4] = normalize(h, height/2);
            vertices[pos + 5] = ((float)(rand() % 1000))/1000;
            pos += 6;
        }
    }

    return vertices;
};

float* getPixelVertexCoordinates(float* vertices, unsigned int width, unsigned int height)
{
    vertices = (float*)malloc(sizeof(float)*height*width*3);
    int pos = 0;
    for (int h = (int)-(height)/2; h < (int)(height)/2; h++)
    {
        for(int w = (int)-(width)/2; w < (int)(width)/2; w++)
        {
            vertices[pos    ] = normalize(w, width/2);
            vertices[pos + 1] = normalize(h, height/2);
            vertices[pos + 2] = 0;
            pos += 3;
        }
    }

    return vertices;
};

unsigned int getShaderProgram()
{
    unsigned int vertexShader   = getShader("./vertex.glsl", GL_VERTEX_SHADER);
    unsigned int fragmentShader = getShader("./fragment.glsl", GL_FRAGMENT_SHADER);
    unsigned int shaderProgram  = glCreateProgram();
    linkShaders(shaderProgram, vertexShader, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int mainRender(Scene scene)
{
    //int width  = 1920;
    //int height = 1080;

    int width  = 800;
    int height = 800;

    openGlEnv env = envInitialize(width, height);
    unsigned int shaderProgram = getShaderProgram();

    // Geometry data
    float dh   = 0.001;
    float dv   = 0.001;
    float distance = 1;

    vec3   origin    = (vec3){-5, 0, 0};
    vec3   direction = (vec3){ 5, 0, 0};

    float* pixelVertexCoordinates = getPixelVertexCoordinates(pixelVertexCoordinates, env.width, env.height);

    vec3* rayDirections = malloc(sizeof(vec3)*env.width*env.height);

    unsigned int vertexDataSize = sizeof(float)*env.width*env.height*6;
    float*       vertexData     = malloc(vertexDataSize);

    // Escrevendo info da posicao de cada pixel no buffer
    int pos = 0;
    for(int i = 0; i < env.height*env.width*3; i+=3)
    {
        vertexData[pos + i    ] = pixelVertexCoordinates[i];
        vertexData[pos + i + 1] = pixelVertexCoordinates[i+1];
        vertexData[pos + i + 2] = pixelVertexCoordinates[i+2];
        pos += 3;
    }

    float geometry[] = 
    {
        0, 0, 0.0, 1.0,
        0.2, 0.8, 0.8, 0.45,
        0.5, 0.8, 0.8, 0.45,
        10
    };

    float ambientLight[4] = {1, 1, 1, 0.1};
    float light[7]        = {0.0, 5, 5, 1, 1, 1, 1};

    unsigned int VBO;
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    float angle = 0.0;
    while(!glfwWindowShouldClose(env.window))
    {
        /*
        angle += 0.1;

        if(angle >= 3.1415*2.0)
            angle = 0.0;

        origin    = (vec3){5*cos(angle), 5*sin(angle), 0};
        direction = (vec3){geometry[1] - origin.x, geometry[2] - origin.y, geometry[3] - origin.z};
        */

        // Needs urgents optimization
        // May be moved to the shader in a pixel by pixel basis
        // Send P, H, V, dh, dv and let each pixel get is ray direction
        getRayDirections(rayDirections, env.width, env.height, origin, direction, dh, dv, distance);

        // Escrevendo info de cada raio de cada pixel no buffer
        pos = 3;
        for(int i = 0; i < env.width*env.height; i++)
        {
            vertexData[pos    ] = rayDirections[i].x;
            vertexData[pos + 1] = rayDirections[i].y;
            vertexData[pos + 2] = rayDirections[i].z;
            pos +=6;
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertexDataSize, vertexData, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        inputHandler(env.window);
        glUseProgram(shaderProgram);
        
        // Old
        int loc = glGetUniformLocation(shaderProgram, "rayOrigin");
        glUniform3fv(loc, 1, (float*)&origin);

        loc = glGetUniformLocation(shaderProgram, "geometry");
        glUniform1fv(loc, 13, geometry);

        loc = glGetUniformLocation(shaderProgram, "light");
        glUniform1fv(loc, 7, light);

        // New
        loc = glGetUniformLocation(shaderProgram, "observer");
        glUniform1fv(loc, sizeof(scene.observer), (float*)&scene.observer);

        loc = glGetUniformLocation(shaderProgram, "ambientLight");
        glUniform4fv(loc, 1, (float*)&scene.ambientLight);

        loc = glGetUniformLocation(shaderProgram, "shapeLocations");
        glUniform1iv(loc, scene.geometryObjectsNumber, scene.geometryObjectsShapeLocations);

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

    free((void*)rayDirections);
    free((void*)pixelVertexCoordinates);
    free((void*)vertexData);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();

    return 0;
}

void algebraTest()
{
    int width  = 3;
    int height = 5;
    float dh   = 1;
    float dv   = 1;
    float distance = 10;

    vec3 origin    = (vec3){-5, 0, 0};
    vec3 direction = (vec3){ 5, 0, 0};
    printf("%f, %f, %f\n", ((float*)&origin)[0], ((float*)&origin)[1], ((float*)&origin)[2]);

    vec3 a = (vec3){1.0, 2.0, 3.0};
    vec3 b = (vec3){2.0, 3.0, 4.0};
    vec3 c = (vec3){3.0, 4.0, 0};

    // Operacoes basicas
    printf("%s\n", "addCoonstVec3(1, a):");
    printVec3(addConstVec3(1, a));

    printf("%s\n", "mulCoonstVec3(1, a):");
    printVec3(mulConstVec3(2, a));

    printf("%s\n", "addVec3(a, b):");
    printVec3(addVec3(a, b));

    printf("%s\n", "subVec3(a, b):");
    printVec3(subVec3(a, b));

    printf("%s\n", "euclidianVec3(a, b):");
    printf("%f\n", euclidianVec3(a, b));

    printf("%s\n", "normVec3(c):");
    printf("%f\n", normVec3(c));

    printf("%s\n", "normalizeVec3(c):");
    printVec3(normalizeVec3(c));

    printf("%s\n", "crossVec3(a, b):");
    printVec3(crossVec3(a, b));

    vec3 P = getScreenCenterVersor(origin, direction);
    printf("%s\n", "getScreenCenterVersor(origin, direction):");
    printVec3(P);

    vec3 H = getScreenHorizontalVersor(P);
    printf("%s\n", "getScreenHorizontalVersor(P):");
    printVec3(H);

    vec3 V = getScreenVerticalVersor(P, H);
    printf("%s\n", "getScreenVerticalVersor(P, H):");
    printVec3(V);

    printf("%s\n", "getScreenPoint(origin, P, H, V, dh, dv, distance):");
    printVec3(getScreenPoint(origin, P, H, V, dh, dv, distance));

    vec3* pixels = malloc(sizeof(vec3)*width*height);

    printf("%s\n", "buildPixelDirections(width, height, origin, P, H, V, dh, dv, distance):");
    //buildRayDirections(pixels, width, height, origin, P, H, V, dh, dv, distance);
    getRayDirections(pixels, width, height, origin, direction, dh, dv, distance);
    for(int i = 0; i < width*height; i++)
    {
        printVec3(pixels[i]);
    }

    free(pixels);

    return;
}
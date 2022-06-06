#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define WIDTH  800
#define HEIGHT 600
#define true  1
#define false 0

typedef struct openGlEnv
{
    GLFWwindow* window;
    unsigned int width;
    unsigned int height;
} openGlEnv;

char* readShaderFile(char* path)
{
    FILE* file = fopen(path, "r");

    fseek(file, 0, SEEK_END);
    int size = ftell(file);

    char* shaderCode = (char*) malloc(sizeof(char)*size);

    fseek(file, 0, SEEK_SET);
    fread(shaderCode, sizeof(char), size, file);
    fclose(file);
    shaderCode[size] = '\0';

    return shaderCode;
}

unsigned int compileShader(unsigned int shader)
{
    int success;
    char log[1024];

    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, log);
        printf("Erro de compilação de shaders: \n%s\n", log);
        return 1;
    }
    
    return 0;
}

unsigned int getShader(char* path, unsigned int type)
{
    char*        shaderCode = readShaderFile(path);
    unsigned int shader     = glCreateShader(type);

    glShaderSource(shader, 1, (const GLchar * const*)&shaderCode, NULL);
    free((void*) shaderCode);
    
    assert(compileShader(shader) == 0);
    return shader;
}

void linkShaders(unsigned int shaderProgram, unsigned int vertexShader, unsigned int fragmentShader)
{
    int  success;
    char log[1024];

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if(!success) 
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, log);
        printf("Erro de linking de shaders: \n%s\n", log);
    }

    assert(success);
}

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
    inputHandler(env->window);
    glUseProgram(shaderProgram);
    
    float arr[5] = {0, 0.1, 0, 0.1, 0.0};
    int loc = glGetUniformLocation(shaderProgram, "arr");
    glUniform1fv(loc, 5, arr);

    float* test = (float*) malloc(sizeof(float)*480000);
    loc = glGetUniformLocation(shaderProgram, "test");
    glUniform1fv(loc, 480000, test);

    glBindVertexArray(*VAO);
    glDrawArrays(GL_POINTS, 0, env->height*env->width);
    glDrawArrays(GL_POINTS, 0, 1);
    glfwSwapBuffers(env->window);
    glfwPollEvents();
}

float* buildPixelVertices(unsigned int width, unsigned int height)
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

int main(void)
{
    openGlEnv env = envInitialize(WIDTH, HEIGHT);

    unsigned int vertexShader   = getShader("./vertex.glsl", GL_VERTEX_SHADER);
    unsigned int fragmentShader = getShader("./fragment.glsl", GL_FRAGMENT_SHADER);;
    unsigned int shaderProgram  = glCreateProgram();
    linkShaders(shaderProgram, vertexShader, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    unsigned int VBO;
    unsigned int VAO;
    unsigned int verticesSize = sizeof(float)*env.width*env.height*6;
    float*       vertices     = buildPixelVertices(env.width, env.height);

    while(!glfwWindowShouldClose(env.window))
    {
        // Vertex Input
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        render(&env, shaderProgram, &VAO);
    }

    free((void*)vertices);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}
#include "shaderLoader.h"

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

    //printf("%s", shaderCode);

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
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

unsigned int getShaderInternal(char* shaderCode, unsigned int type)
{
    unsigned int shader  = glCreateShader(type);

    glShaderSource(shader, 1, (const GLchar * const*)&shaderCode, NULL);
    free((void*) shaderCode);
    
    assert(compileShader(shader) == 0);
    return shader;
}

int intDigits(unsigned int i)
{
    int n = 0;
    while(i > 0)
    {
        n++;
        i = i/10;
    }

    return n;
}

void replaceShaderNumber(char* str, unsigned int pos, unsigned int number)
{
    char buffer[] = "        ";
    snprintf(&buffer[8-intDigits(number)], intDigits(number)+1, "%d", number);
    memcpy(&str[pos], buffer, 8);
}

unsigned int getSceneVertexShader(char* path, Scene scene)
{
    char* shaderCode     = readShaderFile(path);
    char* numberLocation = NULL;

    numberLocation = strstr(shaderCode, "LIGHT_SOURCES_SIZE 00000000");
    assert(numberLocation != NULL);
    replaceShaderNumber(numberLocation, 19, scene.lightSourceNumber*sizeof(LightSource)/sizeof(float));

    numberLocation = strstr(shaderCode, "SHAPE_LOCATIONS_SIZE 00000000");
    assert(numberLocation != NULL);
    replaceShaderNumber(numberLocation, 21, scene.geometryObjectsNumber);

    numberLocation = strstr(shaderCode, "SHAPE_TYPES_SIZE 00000000");
    assert(numberLocation != NULL);
    replaceShaderNumber(numberLocation, 17, scene.geometryObjectsNumber);        

    numberLocation = strstr(shaderCode, "SHAPE_COLORS_SIZE 00000000");
    assert(numberLocation != NULL);
    replaceShaderNumber(numberLocation, 18, scene.geometryObjectsNumber*sizeof(GeometryColor)/sizeof(float));

    numberLocation = strstr(shaderCode, "SHAPES_SIZE 00000000");
    assert(numberLocation != NULL);
    replaceShaderNumber(numberLocation, 12, scene.geometryObjectsTotalSize);

    return getShaderInternal(shaderCode, GL_VERTEX_SHADER);
}

unsigned int getShader(char* path, unsigned int type)
{
    char*  shaderCode = readShaderFile(path);
    return getShaderInternal(shaderCode, type);
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
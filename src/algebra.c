#include "algebra.h"

void printVec3(vec3 a)
{
    printf("%f, %f, %f\n", a.x, a.y, a.z);
}

vec3 addConstVec3(float constant, vec3 a)
{
    return (vec3){constant + a.x, constant + a.y, constant + a.z};
}

vec3 mulConstVec3(float constant, vec3 a)
{
    return (vec3){constant * a.x, constant * a.y, constant * a.z};
}

vec3 addVec3(vec3 a, vec3 b)
{
    return (vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

vec3 subVec3(vec3 a, vec3 b)
{
    return (vec3){b.x - a.x, b.y - a.y, b.z - a.z};
}

float euclidianVec3(vec3 a, vec3 b)
{
    float dx = (b.x - a.x);
    float dy = (b.y - a.y);
    float dz = (b.z - a.z);

    return sqrt(dx*dx + dy*dy + dz*dz);
}

float normVec3(vec3 a)
{
    return euclidianVec3(a, (vec3){0, 0, 0});
}

vec3 normalizeVec3(vec3 a)
{
    return mulConstVec3(1/normVec3(a), a);
}

vec3 crossVec3(vec3 a, vec3 b)
{
    return (vec3){a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
}

Matrix newMatrix(int rowNum, int colNum)
{
    float** data = (float**)calloc(sizeof(float*), rowNum);
    
    for (int i = 0; i < rowNum; i++)
        data[i] = (float*)calloc(sizeof(float), colNum);

    return (Matrix){data, rowNum, colNum};
}

void freeMatrix(Matrix a)
{
    for (int i = 0; i < a.rowNum; i++)
        free(a.data[i]);

    free(a.data);
}

int matricesAreMutipliable(Matrix a, Matrix b)
{
    return a.colNum == b.rowNum;
}

Matrix mulMatrix(Matrix a, Matrix b)
{
    assert(matricesAreMutipliable(a, b));

    Matrix ab = newMatrix(a.rowNum, b.colNum);

    for (int i = 0; i < ab.rowNum; i++)
    {
        for(int j = 0; j < ab.colNum; j++)
        {
            for(int k = 0; k < a.colNum; k++)
            {
                ab.data[i][j] += a.data[i][k]*b.data[k][j];
            }
        }
    }

    return ab;
}

Matrix matrixFromVec3(vec3 v)
{
    Matrix a = newMatrix(3, 1);
    a.data[0][0] = v.x;
    a.data[1][0] = v.y;
    a.data[2][0] = v.z;
    return a;
}

vec3 vec3FromMatrix(Matrix a)
{
    return (vec3){a.data[0][0], a.data[1][0], a.data[2][0]};
}

Matrix yawRotation(Matrix a, float angle)
{
    Matrix r = newMatrix(3, 3);

    float cosa = cos(angle);
    float sina = sin(angle);

    r.data[0][0] = cosa; r.data[0][1] = -sina; r.data[0][2] = 0; 
    r.data[1][0] = sina; r.data[1][1] =  cosa; r.data[1][2] = 0; 
    r.data[2][0] = 0     ; r.data[2][1] = 0      ; r.data[2][2] = 1;

    Matrix rotated = mulMatrix(r, a);
    freeMatrix(r);
    return rotated;
}

Matrix pitchRotation(Matrix a, float angle)
{
    Matrix r = newMatrix(3, 3);

    float cosa = cos(angle);
    float sina = sin(angle);

    r.data[0][0] = cosa ; r.data[0][1] = 0; r.data[0][2] = sina; 
    r.data[1][0] = 0      ; r.data[1][1] = 1; r.data[1][2] = 0     ; 
    r.data[2][0] = -sina; r.data[2][1] = 0; r.data[2][2] = cosa;

    Matrix rotated = mulMatrix(r, a);
    freeMatrix(r);
    return rotated;
}

Matrix rollRotation(Matrix a, float angle)
{
    Matrix r = newMatrix(3, 3);
    float cosa = cos(angle);
    float sina = sin(angle);

    r.data[0][0] = 1; r.data[0][1] = 0     ; r.data[0][2] = 0      ; 
    r.data[1][0] = 0; r.data[1][1] = cosa; r.data[1][2] = -sina; 
    r.data[2][0] = 0; r.data[2][1] = sina; r.data[2][2] =  cosa;

    Matrix rotated = mulMatrix(r, a);
    freeMatrix(r);
    return rotated;
}

Matrix arbitraryAxisRotation(Matrix a, int ax, int ay, int az, float angle)
{
    Matrix r   = newMatrix(3, 3);
    float cosa = cos(angle);
    float sina = sin(angle);

    r.data[0][0] = cosa+ax*ax*(1-cosa); r.data[0][1] = ax*ay*(1-cosa)-az*sina; r.data[0][2] = ax*ay*(1-cosa)+ay*sina; 
    r.data[1][0] = ay*ax*(1-cosa)+ax*sina; r.data[1][1] = cosa+ay*ay*(1-cosa); r.data[1][2] = ay*az*(1-cosa)-ax*sina; 
    r.data[2][0] = az*ax*(1-cosa)-ay*sina; r.data[2][1] = az*ay*(1-cosa)+az*sina; r.data[2][2] = cosa+az*az*(1-cosa);

    Matrix rotated = mulMatrix(r, a);
    freeMatrix(r);
    return rotated;
}

vec3 getScreenCenterVersor(vec3 origin, vec3 direction)
{
    return normalizeVec3(subVec3(origin, direction));
}

vec3 getScreenHorizontalVersor(vec3 centerVer)
{
    return mulConstVec3(-1, crossVec3(centerVer, (vec3){0, 0, 1}));   
}

vec3 getScreenVerticalVersor(vec3 centerVer, vec3 horizontalVer)
{
    return crossVec3(centerVer, horizontalVer);   
}

vec3 getScreenPoint(vec3 origin, vec3 center, vec3 horizontal, vec3 vertical, float dh, float dv, float distance)
{
    return addVec3(mulConstVec3(1/distance, center), addVec3(mulConstVec3(dh, horizontal), mulConstVec3(dv, vertical)));
}

float normalize(float x, float size)
{
    return x/size;
}

float* getPixelVertexCoordinates(float* vertices, unsigned int width, unsigned int height)
{
    vertices = (float*)malloc(sizeof(float)*height*width*3);
    int pos = 0;
    for (int h = (int)-(height)/2; h < (int)(height)/2; h++)
    {
        for(int w = (int)-(width)/2; w < (int)(width)/2; w++)
        {
            vertices[pos    ] = ((float)w)/(width/2);
            vertices[pos + 1] = ((float)h)/(height/2);
            vertices[pos + 2] = 0;
            pos += 3;
        }
    }

    return vertices;
};

float* getPixelDislocations(unsigned int width, unsigned int height)
{
    float* dislocations = (float*)malloc(sizeof(float)*2*width*height);

    for(int i = 0; i < height; i++)
    {
        int lineJump = i*width;
        for(int j = 0; j < width; j++)
        {
            dislocations[(j + lineJump)*2    ] = (float) i; 
            dislocations[(j + lineJump)*2 + 1] = (float) j;
        }
    }

    return dislocations;
}
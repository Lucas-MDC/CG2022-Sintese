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
    return addVec3(mulConstVec3(distance, center), addVec3(mulConstVec3(dh, horizontal), mulConstVec3(dv, vertical)));
}

void buildRayDirections(vec3* pixels, unsigned int width, unsigned int height, vec3 origin, vec3 center, vec3 horizontal, vec3 vertical, float dh, float dv, float distance)
{
    vec3 pixelStart = getScreenPoint(origin, center, horizontal, vertical, 0.5*(1 - (float)width)*dh, 0.5*(1 - (float)height)*dv, distance);
    
    int lineJump = 0;
    for(int i = 0; i < height; i++)
    {
        lineJump = i*width;
        for(int j = 0; j < width; j++)
        {
            pixels[j + lineJump] = normalizeVec3(addVec3(mulConstVec3(i*dv, vertical) , addVec3(mulConstVec3(j*dh, horizontal), pixelStart)));
        }
    }

    return;
}

void getRayDirections(vec3* pixels, unsigned int width, unsigned int height, vec3 origin, vec3 direction, float dh, float dv, float distance)
{
    vec3 P = getScreenCenterVersor(origin, direction);
    vec3 H = getScreenHorizontalVersor(P);
    vec3 V = getScreenVerticalVersor(P, H);
    buildRayDirections(pixels, width, height, origin, P, H, V, dh, dv, distance);
    return;
}
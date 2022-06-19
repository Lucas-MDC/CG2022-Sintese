#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct vec3
{
    float x;
    float y;
    float z;
} vec3;

void printVec3(vec3 a);

vec3 addConstVec3(float constant, vec3 a);

vec3 mulConstVec3(float constant, vec3 a);

vec3 addVec3(vec3 a, vec3 b);

vec3 subVec3(vec3 a, vec3 b);

float euclidianVec3(vec3 a, vec3 b);

float normVec3(vec3 a);

vec3 normalizeVec3(vec3 a);

vec3 crossVec3(vec3 a, vec3 b);

vec3 getScreenCenterVersor(vec3 origin, vec3 direction);

vec3 getScreenHorizontalVersor(vec3 centerVer);

vec3 getScreenVerticalVersor(vec3 centerVer, vec3 horizontalVer);

vec3 getScreenPoint(vec3 origin, vec3 center, vec3 horizontal, vec3 vertical, float dh, float dv, float distance);

void buildRayDirections(vec3* pixels, unsigned int width, unsigned int height, vec3 origin, vec3 center, vec3 horizontal, vec3 vertical, float dh, float dv, float distance);

void getRayDirections(vec3* pixels, unsigned int width, unsigned int height, vec3 origin, vec3 center, float dh, float dv, float distance);

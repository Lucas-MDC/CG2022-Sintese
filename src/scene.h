#pragma once
#include <stdio.h>
#include <string.h>

#define LIGHT_TYPE_POINT 1.0
#define GEOMETRY_TYPE_SHERE 1.0

typedef struct Observer
{
    float xOrigin;
    float yOrigin;
    float zOrigin;
    float xObserved;
    float yObserved;
    float zObserved;
    float width;
    float height;
    float dh;
    float dv;
    float distance;
} Observer;

typedef struct ObserverProcessed
{
    float xOrigin;
    float yOrigin;
    float zOrigin;
    float xCenterVersor;
    float yCenterVersor;
    float zCenterVersor;
    float xHorizontalVersor;
    float yHorizontalVersor;
    float zHorizontalVersor;
    float xVerticalVersor;
    float yVerticalVersor;
    float zVerticalVersor;
    float width;
    float height;
    float dh;
    float dv;
    float distance;
} ObserverProcessed;

typedef struct RayInfo
{
    float horizontalSteps;
    float verticalSteps;
} RayInfo;

typedef struct AmbientLight
{
    float r;
    float g;
    float b;
} AmbientLight;

typedef struct LightSource
{
    float type;
    float x;
    float y;
    float z;
    float r;
    float g;
    float b;
    float attenuation;
} LightSource;

// Depreciado
typedef struct GeometryHeader
{
    float type;
    float number;
} GeometryHeader;

typedef struct GeometryColor
{
    float ambientConstant;
    float rDiffuse;
    float gDiffuse;
    float bDiffuse;
    float diffuseConstant;
    float rSpecular;
    float gSpecular;
    float bSpecular;
    float specularConstant;
    float transparency;
    float shininess;
} GeometryColor;

typedef struct GeometrySphere
{
    float xCenter;
    float yCenter;
    float zCenter;
    float radius;
} GeometrySphere;

typedef struct Scene
{
    ObserverProcessed observer;
    AmbientLight      ambientLight;
    int               lightSourceNumber;
    float*            lightSources;
    int               geometryObjectsNumber;
    int*              geometryObjectsShapeLocations;
    float*            geometryObjectsTypes;
    float*            geometryObjectsColor;
    float*            geometryObjectsShapes;
    int               geometryObjectsTotalSize;
} Scene;

unsigned int loadBuffer(float* buffer, void* object, unsigned int size);

ObserverProcessed getObserverProcessed(Observer obs);

Scene getTestScene();
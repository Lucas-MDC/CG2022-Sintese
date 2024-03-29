#pragma once
#include <stdio.h>
#include <string.h>

#define LIGHT_TYPE_POINT 1.0
#define GEOMETRY_TYPE_SPHERE 1
#define GEOMETRY_TYPE_TRIANGLE 2
#define GEOMETRY_HAS_NO_VOLUME 0.0
#define GEOMETRY_HAS_VOLUME 1.0

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
    float xObserved;
    float yObserved;
    float zObserved;
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
    float refractionConstant;
    float hasVolume;
} GeometryColor;

typedef struct GeometrySphere
{
    float xCenter;
    float yCenter;
    float zCenter;
    float radius;
} GeometrySphere;

typedef struct GeometryTriangle
{
    float xVertexA;
    float yVertexA;
    float zVertexA;
    float xVertexB;
    float yVertexB;
    float zVertexB;
    float xVertexC;
    float yVertexC;
    float zVertexC;
    float xNormal;
    float yNormal;
    float zNormal;
} GeometryTriangle;

typedef struct Scene
{
    ObserverProcessed observer;
    AmbientLight      ambientLight;
    int               lightSourceNumber;
    float*            lightSources;
    int               geometryObjectsNumber;
    int*              geometryObjectsShapeLocations;
    int*              geometryObjectsTypes;
    float*            geometryObjectsColor;
    float*            geometryObjectsShapes;
    int               geometryObjectsTotalSize;
} Scene;

unsigned int loadBuffer(float* buffer, void* object, unsigned int size);

ObserverProcessed getObserverProcessed(Observer obs);

void updateSceneVersors(Scene* scene);

Scene getTestScene();
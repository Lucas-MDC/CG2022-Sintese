#include "scene.h"
#include "algebra.h"

unsigned int loadBuffer(float* buffer, void* object, unsigned int size)
{
    memcpy(buffer, (float*) object, size);

    return size/sizeof(float);
}

ObserverProcessed getObserverProcessed(Observer obs)
{
    vec3 centerVersor     = getScreenCenterVersor((vec3){obs.xOrigin, obs.yOrigin, obs.zOrigin}, (vec3){obs.xObserved, obs.yObserved, obs.zObserved});
    vec3 horizontalVersor = getScreenHorizontalVersor(centerVersor);
    vec3 verticalVersor   = getScreenVerticalVersor(centerVersor, horizontalVersor);

    return (ObserverProcessed)
    {
        obs.xOrigin,
        obs.yOrigin,
        obs.zOrigin,
        centerVersor.x,
        centerVersor.y,
        centerVersor.z,
        horizontalVersor.x,
        horizontalVersor.y,
        horizontalVersor.z,
        verticalVersor.x,
        verticalVersor.y,
        verticalVersor.z,
        obs.width,
        obs.height,
        obs.dh,
        obs.dv,
        obs.distance,
        obs.xObserved,
        obs.yObserved,
        obs.zObserved
    };
}

void updateSceneVersors(Scene* scene)
{
    vec3 centerVersor     = getScreenCenterVersor((vec3){scene->observer.xOrigin, scene->observer.yOrigin, scene->observer.zOrigin}, (vec3){scene->observer.xObserved, scene->observer.yObserved, scene->observer.zObserved});
    vec3 horizontalVersor = getScreenHorizontalVersor(centerVersor);
    vec3 verticalVersor   = getScreenVerticalVersor(centerVersor, horizontalVersor);

    scene->observer.xCenterVersor     = centerVersor.x;
    scene->observer.yCenterVersor     = centerVersor.y;
    scene->observer.zCenterVersor     = centerVersor.z;
    scene->observer.xHorizontalVersor = horizontalVersor.x;
    scene->observer.yHorizontalVersor = horizontalVersor.y;
    scene->observer.zHorizontalVersor = horizontalVersor.z;
    scene->observer.xVerticalVersor   = verticalVersor.x;
    scene->observer.yVerticalVersor   = verticalVersor.y;
    scene->observer.zVerticalVersor   = verticalVersor.z;
}

GeometryTriangle makeGeometryTriangle(vec3 a, vec3 b, vec3 c)
{
    vec3 ab     = subVec3(b, a);
    vec3 ac     = subVec3(c, a);
    vec3 normal = normalizeVec3(crossVec3(ab, ac));

    return (GeometryTriangle){a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z, normal.x, normal.y, normal.z};
}

Scene getTestScene()
{
    Observer obs = 
    {
        -5.0, 0.0, 0.0, // x, y, z origin   coordinates
        1.0 , 0.0, 0.0, // x, y, z observed coordinates 
        1920.0, 1080.0,      // resolution 
        0.001, 0.001, 1  // delta horizontal, delta vertical, distance
    };
    ObserverProcessed obsp = getObserverProcessed(obs);

    AmbientLight ambientLight = {0.5, 0.5, 0.5};

    LightSource lightA = 
    {
        LIGHT_TYPE_POINT, 
        5.0, 0.0, 10.0, // x, y, z cooridantes
        1.0, 1.0, 1.0 , // r, g, b colors
        1               // attenuation factor
    };

    LightSource lightB = 
    {
        LIGHT_TYPE_POINT,
        5.0, 20.0, 0.0  , // x, y, z cooridantes
        1.0, 1.0 , 1.0 , // r, g, b colors
        1                // attenuation factor
    };

    GeometryColor sphereColorA =
    {
        0.2,                 // ambient constant
        1.0, 0.0, 0.0, 0.35, // r, g, b diffuse colors, diffuse constant
        1.0, 0.8, 0.8, 0.45, // r, g, b specullar colors, specular constant
        0.0, 10.0, 0.0,      // transparency, shininess and refraction constant
        GEOMETRY_HAS_VOLUME
    };

    GeometryColor sphereColorB =
    {
        0.2,                 // ambient constant
        0.0, 1.0, 0.0, 0.35, // r, g, b diffuse colors, diffuse constant
        0.8, 1.0, 0.8, 0.45, // r, g, b specullar colors, specular constant
        0.0, 15.0, 0.0,      // transparency, shininess and refraction constant
        GEOMETRY_HAS_VOLUME
    };

    GeometryColor sphereColorC =
    {
        0.2,                 // ambient constant
        1.0, 1.0, 1.0, 0.35, // r, g, b diffuse colors, diffuse constant
        1.0, 1.0, 1.0, 0.45, // r, g, b specullar colors, specular constant
        0.8, 100.0, 1.333,   // transparency, shininess and refraction constant
        GEOMETRY_HAS_VOLUME
    };

    GeometryColor triangleColorA =
    {
        0.1,                 // ambient constant
        1.0, 1.0, 1.0, 0.00, // r, g, b diffuse colors, diffuse constant
        1.0, 1.0, 1.0, 0.90, // r, g, b specullar colors, specular constant
        0.0, 100.0, 1.333,   // transparency, shininess and refraction constant
        GEOMETRY_HAS_NO_VOLUME
    };

    GeometrySphere   sphereA   = { 5.0,  0.0, 0.0, 1.0};
    GeometrySphere   sphereB   = { 0.0, -4.0, 0.0, 0.5};
    GeometrySphere   sphereC   = { 5.0, -4.0, 0.0, 2.0};
    GeometryTriangle triangleA = makeGeometryTriangle((vec3){10.0, -6.0, -1.0}, (vec3){10.0, -4.0, 4.0}, (vec3){10.0, -2.0, 1.0});

    int    lightSourceNumber       = 2;
    float* lightSourceBuffer       = (float*)calloc(1, sizeof(LightSource)*lightSourceNumber);
    int    geometryShapesNumber    = 4;
    int*   geometryShapesLocations = (int*  )calloc(1, sizeof(unsigned int)*geometryShapesNumber);
    int*   geometryTypesBuffer     = (int*  )calloc(1, sizeof(float)*geometryShapesNumber);
    float* geometryColorsBuffer    = (float*)calloc(1, sizeof(GeometryColor)*geometryShapesNumber);
    float* geometryShapesBuffer    = (float*)calloc(1, sizeof(GeometrySphere)*3 + sizeof(GeometryTriangle));

    Scene scene = 
    {
        obsp,
        ambientLight,
        lightSourceNumber,
        lightSourceBuffer,
        geometryShapesNumber,
        geometryShapesLocations,
        geometryTypesBuffer,
        geometryColorsBuffer,
        geometryShapesBuffer,
        geometryShapesNumber
    };

    unsigned int loc;

    // Loading lights
    loc = 0;
    loc += loadBuffer(&lightSourceBuffer[loc], (float*)&lightA, sizeof(LightSource));
    loc += loadBuffer(&lightSourceBuffer[loc], (float*)&lightB, sizeof(LightSource));

    geometryTypesBuffer[0] = GEOMETRY_TYPE_SPHERE;
    geometryTypesBuffer[1] = GEOMETRY_TYPE_SPHERE;
    geometryTypesBuffer[2] = GEOMETRY_TYPE_SPHERE;
    geometryTypesBuffer[3] = GEOMETRY_TYPE_TRIANGLE;

    // Loading Geometry Colors
    loc = 0;
    loc += loadBuffer(&geometryColorsBuffer[loc], (float*)&sphereColorA  , sizeof(GeometryColor));
    loc += loadBuffer(&geometryColorsBuffer[loc], (float*)&sphereColorB  , sizeof(GeometryColor));
    loc += loadBuffer(&geometryColorsBuffer[loc], (float*)&sphereColorC  , sizeof(GeometryColor));
    loc += loadBuffer(&geometryColorsBuffer[loc], (float*)&triangleColorA, sizeof(GeometryColor));

    // Loading Geometry Shapes
    loc = 0;
    geometryShapesLocations[0] = loc;
    loc += loadBuffer(&geometryShapesBuffer[loc], (float*)&sphereA, sizeof(GeometrySphere));
    geometryShapesLocations[1] = loc;
    loc += loadBuffer(&geometryShapesBuffer[loc], (float*)&sphereB, sizeof(GeometrySphere));
    geometryShapesLocations[2] = loc;
    loc += loadBuffer(&geometryShapesBuffer[loc], (float*)&sphereC, sizeof(GeometrySphere));
    geometryShapesLocations[3] = loc;
    loc += loadBuffer(&geometryShapesBuffer[loc], (float*)&triangleA, sizeof(GeometryTriangle));
    scene.geometryObjectsTotalSize = loc;
    
    return scene;
}
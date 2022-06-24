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
        obs.distance
    };
}

Scene getTestScene()
{
    Observer obs = 
    {
        0.0  , 0.0  , 0.0, // x, y, z origin   coordinates
        1.0  , 0.0  , 0.0, // x, y, z observed coordinates 
        800.0, 800.0,      // resolution 
        0.01 , 0.01 , 1.0  // delta horizontal, delta vertical, ditance
    };
    ObserverProcessed obsp = getObserverProcessed(obs);

    AmbientLight ambientLight = {0.0, 0.05, 0.1};

    LightSource lightA = 
    {
        LIGHT_TYPE_POINT, 
        5.0, 0.0, 5.0, // x, y, z cooridantes
        1.0, 1.0, 1.0, // r, g, b colors
        1              // attenuation factor
    };

    LightSource lightB = 
    {
        LIGHT_TYPE_POINT,
        5.0, 10.0, 5.0  , // x, y, z cooridantes
        1.0, 0.0 , 0.0  , // r, g, b colors
        0.5               // attenuation factor
    };

    GeometryHeader geoHeader = 
    {
        GEOMETRY_TYPE_SHERE, 
        3.0
    };

    GeometryColor sphereColorA =
    {
        0.1,                 // ambient constant
        0.8, 0.0, 0.0, 0.45, // r, g, b diffuse colors, diffuse constant
        0.8, 0.6, 0.6, 0.45, // r, g, b specullar colors, specular constant
        0.0, 10.0            // transparency and shininess
    };

    GeometryColor sphereColorB =
    {
        0.1,                 // ambient constant
        0.0, 0.8, 0.0, 0.45, // r, g, b diffuse colors, diffuse constant
        0.6, 0.8, 0.6, 0.45, // r, g, b specullar colors, specular constant
        0.0, 15.0            // transparency and shininess
    };

    GeometryColor sphereColorC =
    {
        0.1,                 // ambient constant
        0.6, 0.0, 0.8, 0.45, // r, g, b diffuse colors, diffuse constant
        0.6, 0.0, 0.8, 0.45, // r, g, b specullar colors, specular constant
        0.0, 20.0            // transparency and shininess
    };

    GeometrySphere sphereA = {5.0,  0.0, 0.0, 1.0};
    GeometrySphere sphereB = {5.0,  3.0, 0.0, 1.0};
    GeometrySphere sphereC = {5.0, -3.0, 0.0, 2.0};

    int           lightSourceNumber       = 2;
    float*        lightSourceBuffer       = (float*)calloc(1, sizeof(LightSource)*lightSourceNumber);
    int           geometryShapesNumber    = 3;
    int*          geometryShapesLocations = (int*)calloc(1, sizeof(unsigned int)*geometryShapesNumber);
    float*        geometryTypesBuffer     = (float*)calloc(1, sizeof(float)*geometryShapesNumber);
    float*        geometryColorsBuffer    = (float*)calloc(1, sizeof(GeometryColor)*geometryShapesNumber);
    float*        geometryShapesBuffer    = (float*)calloc(1, sizeof(GeometrySphere)*geometryShapesNumber);

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

    geometryTypesBuffer[0] = GEOMETRY_TYPE_SHERE;
    geometryTypesBuffer[1] = GEOMETRY_TYPE_SHERE;
    geometryTypesBuffer[2] = GEOMETRY_TYPE_SHERE;

    // Loading Geometry Colors
    loc = 0;
    loc += loadBuffer(&geometryColorsBuffer[loc], (float*)&sphereColorA, sizeof(GeometryColor));
    loc += loadBuffer(&geometryColorsBuffer[loc], (float*)&sphereColorB, sizeof(GeometryColor));
    loc += loadBuffer(&geometryColorsBuffer[loc], (float*)&sphereColorC, sizeof(GeometryColor));

    // Loading Geometry Shapes
    loc = 0;
    geometryShapesLocations[0] = loc;
    loc += loadBuffer(&geometryShapesBuffer[loc], (float*)&sphereA, sizeof(GeometrySphere));
    geometryShapesLocations[1] = loc;
    loc += loadBuffer(&geometryShapesBuffer[loc], (float*)&sphereB, sizeof(GeometrySphere));
    geometryShapesLocations[2] = loc;
    loc += loadBuffer(&geometryShapesBuffer[loc], (float*)&sphereC, sizeof(GeometrySphere));
    scene.geometryObjectsTotalSize = loc;

    printf("%f\n", scene.observer.xVerticalVersor);
    printf("%f, %f\n", ((LightSource*)scene.lightSources)->attenuation, ((LightSource*)&scene.lightSources[sizeof(LightSource)/sizeof(float)])->attenuation);
    printf("%f, %f, %f\n", 
        ((GeometryColor*)&scene.geometryObjectsColor[0*sizeof(GeometryColor)/sizeof(float)])->shininess,
        ((GeometryColor*)&scene.geometryObjectsColor[1*sizeof(GeometryColor)/sizeof(float)])->shininess,
        ((GeometryColor*)&scene.geometryObjectsColor[2*sizeof(GeometryColor)/sizeof(float)])->shininess
    );
    printf("%f, %f, %f\n", 
        ((GeometrySphere*)&scene.geometryObjectsShapes[scene.geometryObjectsShapeLocations[0]])->radius,
        ((GeometrySphere*)&scene.geometryObjectsShapes[scene.geometryObjectsShapeLocations[1]])->radius,
        ((GeometrySphere*)&scene.geometryObjectsShapes[scene.geometryObjectsShapeLocations[2]])->radius
    );
    printf("%i \n", scene.geometryObjectsTotalSize);

    return scene;
}
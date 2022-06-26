#version 330 core
#extension GL_NV_shader_buffer_load : enable
out vec4 pixelNewColor;
layout (location = 0) in vec3 pixelPos;
layout (location = 1) in vec2 pixelDislocations;

#define OBSERVER_STRUCT_SIZE 17
#define LIGHT_SOURCE_STRUCT_SIZE 8
#define GEOMETRY_COLOR_STRUCT_SIZE 11

// DONT TOUCH THESE, THE PRECISE LINES ARE USED TO DYNAMICALLY SET THE SIZES IN THE CPU CODE
#define LIGHT_SOURCES_SIZE 00000000
#define SHAPE_LOCATIONS_SIZE 00000000
#define SHAPE_TYPES_SIZE 00000000
#define SHAPE_COLORS_SIZE 00000000
#define SHAPES_SIZE 00000000

#define LIGHT_SOUCES_NUMBER LIGHT_SOURCES_SIZE/LIGHT_SOURCE_STRUCT_SIZE
#define SHAPES_NUMBER SHAPE_TYPES_SIZE

#define GEOMETRY_TYPE_SPHERE 1
#define GEOMETRY_TYPE_TRIANGLE 2

#define INTERSECTED     1.0
#define NOT_INTERSECTED 0.0

#define IS_SHADOW_RAY 1
#define IS_NOT_SHADOW_RAY 0

struct ObserverProcessed
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
};

struct LightSource
{
    float type;
    float x;
    float y;
    float z;
    float r;
    float g;
    float b;
    float attenuation;
};

struct GeometryColor
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
};

struct GeometrySphere
{
    float xCenter;
    float yCenter;
    float zCenter;
    float radius;
};

struct GeometryIntersection
{
    float intersected;
    vec3  point;
    vec3  normal;
};


uniform float observer[OBSERVER_STRUCT_SIZE];
uniform float lightSources[LIGHT_SOURCES_SIZE];
uniform int   shapeLocations[SHAPE_LOCATIONS_SIZE];
uniform int   shapeTypes[SHAPE_TYPES_SIZE];
uniform float shapeColors[SHAPE_COLORS_SIZE];
uniform float shapes[SHAPES_SIZE];
uniform vec3  ambientLight;

/* * * * * * * * * * * * * * * * * * * * * * * * * * Geometry and Light Structs Functions * * * * * * * * * * * * * * * * * * * * * * * * * */

////// General functions
/*
    shapeIndex = which shape is it
*/
int getShapeAbsolutePosition(int shapeIndex)
{
    return shapeLocations[shapeIndex];
}

////// Light functions
struct LightSource makeLightSource(int lightIndex)
{
    int pos = lightIndex*LIGHT_SOURCE_STRUCT_SIZE;
    return LightSource(
                        lightSources[pos + 0], 
                        lightSources[pos + 1], lightSources[pos + 2], lightSources[pos + 3],
                        lightSources[pos + 4], lightSources[pos + 5], lightSources[pos + 6], 
                        lightSources[pos + 7]
                      );
}

vec3 getLightSourcePosition(int lightIndex)
{
    int pos = lightIndex*LIGHT_SOURCE_STRUCT_SIZE;
    return vec3(lightSources[pos + 1], lightSources[pos + 2], lightSources[pos + 3]);
}

vec3 getLightSourceColor(int lightIndex)
{
    int pos = lightIndex*LIGHT_SOURCE_STRUCT_SIZE;
    return vec3(lightSources[pos + 4], lightSources[pos + 5], lightSources[pos + 6]);
}

float getLightSourceAttenuation(int lightIndex)
{
    int pos = lightIndex*LIGHT_SOURCE_STRUCT_SIZE;
    return lightSources[pos + 7];
}

////// Geometry (Shape) functions
/*
    absposition = absolute position inside the shapes array
*/
struct GeometrySphere makeGeometrySphere(int shapeIndex)
{
    int pos = getShapeAbsolutePosition(shapeIndex);
    return GeometrySphere(shapes[pos + 0], shapes[pos + 1], shapes[pos + 2], shapes[pos + 3]);
}

/*
    shapeIndex = which shape is it
*/
struct GeometryColor makeGeometryColor(int shapeIndex)
{
    int pos = shapeIndex*GEOMETRY_COLOR_STRUCT_SIZE;
    return GeometryColor(
                            shapeColors[pos + 0], 
                            shapeColors[pos + 1], shapeColors[pos + 2], shapeColors[pos + 3], shapeColors[pos + 4],
                            shapeColors[pos + 5], shapeColors[pos + 6], shapeColors[pos + 7], shapeColors[pos + 8], 
                            shapeColors[pos + 9], shapeColors[pos + 10]
                        );
}

float getGeometryAmbientConstant(int shapeIndex)
{
    return shapeColors[shapeIndex*GEOMETRY_COLOR_STRUCT_SIZE + 0];
}

vec4 getGeometryDiffuseColor(int shapeIndex)
{
    int pos = shapeIndex*GEOMETRY_COLOR_STRUCT_SIZE; 
    return vec4(shapeColors[pos + 1], shapeColors[pos + 2], shapeColors[pos + 3], shapeColors[pos + 4]);
}

vec4 getGeometrySpecularColor(int shapeIndex)
{
    int pos = shapeIndex*GEOMETRY_COLOR_STRUCT_SIZE;
    return vec4(shapeColors[pos + 5], shapeColors[pos + 6], shapeColors[pos + 7], shapeColors[pos + 8]);
}

float getGeometryTransparency(int shapeIndex)
{
    return shapeColors[shapeIndex*GEOMETRY_COLOR_STRUCT_SIZE + 9];
}

float getGeometryShininess(int shapeIndex)
{
    return shapeColors[shapeIndex*GEOMETRY_COLOR_STRUCT_SIZE + 10];
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * Observer Struct Functions * * * * * * * * * * * * * * * * * * * * * * * * * */
vec3 getOrigin()
{
    return vec3(observer[0], observer[1], observer[2]);
}

vec3 getCenterVersor()
{
    return vec3(observer[3], observer[4], observer[5]);
}

vec3 getHorizontalVersor()
{
    return vec3(observer[6], observer[7], observer[8]);
}

vec3 getVerticalVersor()
{
    return vec3(observer[9], observer[10], observer[11]);
}

vec2 getScreenDimensions()
{
    return vec2(observer[12], observer[13]);
}

vec2 getScreenDifferentials()
{
    return vec2(observer[14], observer[15]);
}

float getScreenDistance()
{
    return observer[16];
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * Geometry Functions * * * * * * * * * * * * * * * * * * * * * * * * * */
////// General functions
float angleBetweenVec3(vec3 a, vec3 b)
{
    return acos(dot(a, b)/(length(a)*length(b)));
}

vec3 getDirectionVersor(vec3 centerVersor, vec3 horizontalVersor, vec3 verticalVersor, float dh, float dv, float dist)
{
    return (1/dist)*centerVersor + dh*horizontalVersor + dv*verticalVersor;
}

vec3 getLightDirection(vec3 intersectionPoint, vec3 lightPosition)
{
    return normalize(lightPosition - intersectionPoint);
}

vec3 getIntersectDirection(vec3 observer, vec3 intersectionPoint)
{
    return normalize(observer - intersectionPoint);
}

vec3 getRayDirection()
{
    float width      = getScreenDimensions().x;
    float height     = getScreenDimensions().y;
    vec3  origin     = getOrigin();
    vec3  center     = getCenterVersor();
    vec3  horizontal = getHorizontalVersor();
    vec3  vertical   = getVerticalVersor();
    float dh         = getScreenDifferentials()[0];
    float dv         = getScreenDifferentials()[1];
    float dist       = getScreenDistance();

    //vec3 pixelStart = getDirectionVersor(origin, center, horizontal, vertical, 0.5*(1 - (float)width)*dh, 0.5*(1 - (float)height)*dv, dist);
    //return normalize(pixelDislocations[0]*dv*vertical + pixelDislocations[1]*dh*horizontal + pixelStart);

    vec3 pixelStart = getDirectionVersor(center, horizontal, vertical, 0.5*(1.0 - (float)width)*dh, 0.5*(1 - (float)height)*dv, dist);
    return normalize(pixelDislocations[0]*dv*vertical + pixelDislocations[1]*dh*horizontal + pixelStart);
}

////// Triangle functions

////// Sphere functions
float raySphereIntersectionB(vec3 rO, vec3 rD, GeometrySphere s)
{
    return 2.0 * (rD.x * (rO.x - s.xCenter) + rD.y * (rO.y - s.yCenter) + rD.z * (rO.z - s.zCenter));
}

float raySphereIntersectionC(vec3 rO, vec3 rD, GeometrySphere s)
{
    return pow(rO.x - s.xCenter, 2) + pow(rO.y - s.yCenter, 2) + pow(rO.z - s.zCenter, 2) - pow(s.radius, 2);
}

float raySphereIntersectionDiscriminant(float B, float C)
{
    return pow(B, 2) - 4*C;
}

vec4 raySphereIntersection(vec3 origin, vec3 direction, GeometrySphere sphere)
{
    float B = raySphereIntersectionB(origin, direction, sphere);
    float C = raySphereIntersectionC(origin, direction, sphere);

    float discriminant = raySphereIntersectionDiscriminant(B, C);

    if (discriminant < 0)
        return vec4(0, 0, 0, NOT_INTERSECTED);

    discriminant = sqrt(discriminant);

    float t  = 0;
    float t0 = (-discriminant - B)/2;
    float t1 = (discriminant  - B)/2;

    if(t0 <= t1)
        t = t0;
    else
        t = t1;

    if(t < 0)
        return vec4(0, 0, 0, NOT_INTERSECTED);

    return vec4(origin.x + direction.x*t, origin.y + direction.y*t, origin.z + direction.z*t, INTERSECTED);   
}

vec3 sphereNormal(vec3 point, struct GeometrySphere sphere)
{
    return normalize(vec3((point.x - sphere.xCenter)/sphere.radius, (point.y - sphere.yCenter)/sphere.radius, (point.z - sphere.zCenter)/sphere.radius));
}

/*
    r0 = ray origin    (x, y, z)
    rD = ray direction (x, y, z)
    s  = sphere (x center, y center, z center, radius)
*/
vec4 raySphereIntersectionOld(vec3 rO, vec3 rD, vec4 s)
{
    float B = 2.0 * (rD.x * (rO.x - s.x) + rD.y * (rO.y - s.y) + rD.z * (rO.z - s.z));
    float C = pow(rO.x - s.x, 2) + pow(rO.y - s.y, 2) + pow(rO.z - s.z, 2) - pow(s.w, 2); 

    float discriminant = pow(B, 2) - 4*C;

    if (discriminant < 0)
        return vec4(0, 0, 0, NOT_INTERSECTED);

    discriminant = sqrt(discriminant);

    float t  = 0;
    float t0 = (-discriminant - B)/2;
    float t1 = (discriminant  - B)/2;

    if(t0 <= t1)
        t = t0;
    else
        t = t1;

    if(t < 0)
        return vec4(0, 0, 0, NOT_INTERSECTED);

    return vec4(rO.x + rD.x*t, rO.y + rD.y*t, rO.z + rD.z*t, INTERSECTED);   
}

/*
    p = point in the spehre (x, y, z)
    s = sphere (x center, y center, z center, radius)
*/
vec3 sphereNormalOld(vec3 p, vec4 s)
{
    return normalize(vec3((p.x - s.x)/s.w, (p.y - s.y)/s.w, (p.z - s.z)/s.w));
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * Illumination Models Functions * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
    amb[0] = ambient light intensity
    amb[1] = ambient light coefficient
    light[0] = light intensity
    light[1] = light attenuation coefficient
    difC[0] = diffuse color
    difC[1] = diffuse coefficient
    speC[0] = specular color
    speC[1] = specular coefficient
    normA = reflection vector and normal vector angle
    obsA  = relfection vector and obeserver angle
    specPer = specularity perfection
*/
float phongIlluminationChannelOld(vec2 amb, vec2 light, vec2 difC, vec2 speC, float normA, float obsA, float specPer)
{
    return amb[0]*amb[1]*difC[0] + light[1]*light[0]*(difC[1]*difC[0]*cos(abs(normA)) + speC[1]*speC[0]*pow(abs(cos(obsA)), specPer));
}

vec4 phongIlluminationOld(vec4 ambL, vec4 light, vec4 difC, vec4 speC, float normA, float obsA, float specPer)
{
    float r = phongIlluminationChannelOld(vec2(ambL.x, ambL.w), vec2(light.x, light.w), vec2(difC.x, difC.w), vec2(speC.x, speC.w), normA, obsA, specPer);
    float g = phongIlluminationChannelOld(vec2(ambL.y, ambL.w), vec2(light.y, light.w), vec2(difC.y, difC.w), vec2(speC.y, speC.w), normA, obsA, specPer);
    float b = phongIlluminationChannelOld(vec2(ambL.z, ambL.w), vec2(light.z, light.w), vec2(difC.z, difC.w), vec2(speC.z, speC.w), normA, obsA, specPer);

    return vec4(r, g, b, 1);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * Raytracing Algorithm Functions * * * * * * * * * * * * * * * * * * * * * * * * * */
GeometryIntersection rayIntersection(vec3 origin, vec3 direction, int objectID)
{
    GeometryIntersection intersection;
    vec4                 intersect;
    vec3                 normal;

    if (shapeTypes[objectID] == GEOMETRY_TYPE_SPHERE)
    {
        GeometrySphere sphere = makeGeometrySphere(objectID);
        intersect             = raySphereIntersection(origin, direction, sphere);
        normal                = sphereNormal(vec3(intersect.x, intersect.y, intersect.z), sphere);
        return GeometryIntersection(intersect.w, vec3(intersect.x, intersect.y, intersect.z), normal); 
    }
    else if (shapeTypes[objectID] == GEOMETRY_TYPE_TRIANGLE)
    {
        // TODO: implementar triagulos
        return GeometryIntersection(NOT_INTERSECTED, vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0));   
    }
    else
    {
        return GeometryIntersection(NOT_INTERSECTED, vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0));
    }
}

bool hasIntersected(GeometryIntersection intersectionPoint)
{
    return intersectionPoint.intersected == INTERSECTED;
}

bool hasNotIntersected(GeometryIntersection intersectionPoint)
{
    return intersectionPoint.intersected != INTERSECTED;
}

struct RayCasted
{
    int objectID;
    float intersectionDistance;
    GeometryIntersection intersection;
};

RayCasted rayCast(vec3 origin, vec3 direction, int isShadowRay)
{
    int   minObjectID = 0;
    float minDistance = 999999999.9;
    GeometryIntersection intersection;
    RayCasted raycasted = RayCasted(0, 0, GeometryIntersection(NOT_INTERSECTED, vec3(0,0,0), vec3(0,0,0)));

    for(int objectID = 0; objectID < SHAPES_NUMBER; objectID++)
    {
        intersection = rayIntersection(origin, direction, objectID);

        if(hasNotIntersected(intersection))
            continue;

        float dist = distance(origin, intersection.point);

        if(dist < minDistance)
        {
            minObjectID = objectID;
            minDistance = dist;
            raycasted = RayCasted(minObjectID, minDistance, intersection);
        }
        
        // A single intersection guarantees that the point will be in darkness from the light source in this specific direction
        if(isShadowRay == IS_SHADOW_RAY)
            break;
    }

    return raycasted;
}

float phongIlluminationChannel(vec2 light, vec2 difC, vec2 speC, float normA, float obsA, float specPer)
{
    return light[1]*light[0]*(difC[1]*difC[0]*cos(abs(normA)) + speC[1]*speC[0]*pow(abs(cos(obsA)), specPer));
}

vec4 phongIllumination(vec3 lightC, float lightAtt, vec4 difC, vec4 speC, float normA, float obsA, float specPer)
{
    float r = phongIlluminationChannel(vec2(lightC.x, lightAtt), vec2(difC.x, difC.w), vec2(speC.x, speC.w), normA, obsA, specPer);
    float g = phongIlluminationChannel(vec2(lightC.y, lightAtt), vec2(difC.y, difC.w), vec2(speC.y, speC.w), normA, obsA, specPer);
    float b = phongIlluminationChannel(vec2(lightC.z, lightAtt), vec2(difC.z, difC.w), vec2(speC.z, speC.w), normA, obsA, specPer);

    return vec4(r, g, b, 1);
}

vec4 phongIlluminationAmbientLight(vec3 ambientColor, float ambientConstant, vec4 difC)
{
    return vec4(ambientColor.r*difC.r*ambientConstant, ambientColor.g*difC.g*ambientConstant, ambientColor.b*difC.b*ambientConstant, 1);
}

vec4 rayTrace(vec3 origin, vec3 direction)
{
    RayCasted raycasted = rayCast(origin, direction, IS_NOT_SHADOW_RAY);
    
    if(hasNotIntersected(raycasted.intersection))
        return vec4(0.0, 0.0, 0.0, 0.0);

    vec4  objectDiffuseColor  = getGeometryDiffuseColor(raycasted.objectID);
    vec4  objectSpecularColor = getGeometrySpecularColor(raycasted.objectID);
    float objectTransparency  = getGeometryTransparency(raycasted.objectID);
    float objectShininess     = getGeometryShininess(raycasted.objectID);

    vec4 rgb = vec4(0, 0, 0, 1);

    float delta = 0.001;
    vec3  intersectionDirection = getIntersectDirection(origin, raycasted.intersection.point);
    vec3  intersectionPointMod  = raycasted.intersection.point + delta*intersectionDirection;

    for(int lightID = 0; lightID < LIGHT_SOUCES_NUMBER; lightID++)
    {
        vec3  lightPosition    = getLightSourcePosition(lightID);
        vec3  lightColor       = getLightSourceColor(lightID);
        float lightAttenuation = getLightSourceAttenuation(lightID);
        vec3  lightDirection   = getLightDirection(raycasted.intersection.point, lightPosition);

        RayCasted shadowRaycasted = rayCast(intersectionPointMod, lightDirection, IS_SHADOW_RAY);

        /*
        vec3 normal       = sphereNormalOld(intersect, sphereGeometry);
        vec3 observerRay  = normalize(rO - intersect);
        vec3 shadowRay    = normalize(lightCoordiantes - intersect);
        float delta = 0.0001;
        vec4 shadowRayIntersect = raySphereIntersectionOld(intersect + delta*observerRay, shadowRay, sphereGeometry);
        */

        // Continue, since the light source is not ocluded
        if(hasIntersected(shadowRaycasted.intersection))
        {
            //rgb += vec4(1.0, 1.0, 1.0, 1.0); //DEBUG
            continue;
        }

        vec3  reflectedRay  = reflect(lightDirection, raycasted.intersection.normal);
        float normalAngle   = angleBetweenVec3(raycasted.intersection.normal, lightDirection);
        float observerAngle = angleBetweenVec3(reflectedRay, intersectionDirection);

        rgb += phongIllumination
              ( 
                lightColor,
                lightAttenuation, 
                objectDiffuseColor,
                objectSpecularColor,
                normalAngle, 
                observerAngle,
                objectShininess
              );
    }

    float objectAmbConstant = getGeometryAmbientConstant(raycasted.objectID);
    rgb += phongIlluminationAmbientLight(ambientLight, objectAmbConstant, objectDiffuseColor)*2;

    return min(rgb, vec4(1.0, 1.0, 1.0, 1.0));
}

vec4 rayTest(vec3 rO, vec3 rD, vec4 ambientLight, float light[7], float sphere[13])
{
    vec4  sphereGeometry   = vec4(sphere[0], sphere[1], sphere[2] , sphere[3] );
    vec4  colorDiffuse     = vec4(sphere[4], sphere[5], sphere[6] , sphere[7] );
    vec4  colorSpecular    = vec4(sphere[8], sphere[9], sphere[10], sphere[11]);
    float reflectionPerf   = sphere[12];
    vec3  lightCoordiantes = vec3(light[0], light[1], light[2]);
    vec4  lightProperties  = vec4(light[3] , light[4] , light[5] , light[6]);

    vec4  rayIntersect = raySphereIntersectionOld(rO, rD, sphereGeometry);
    
    float intersected  = rayIntersect.w;
    vec3  intersect    = vec3(rayIntersect.x, rayIntersect.y, rayIntersect.z);

    if(intersected == NOT_INTERSECTED)
        return vec4(0, 0, 0, 0);

    vec3 normal       = sphereNormalOld(intersect, sphereGeometry);
    vec3 observerRay  = normalize(rO - intersect);
    vec3 shadowRay    = normalize(lightCoordiantes - intersect);

    /*
    observerRay  = normalize(intersect - rO);
    shadowRay    = normalize(lightCoordiantes - intersect);

    observerRay  = normalize(rO - intersect);
    shadowRay   = normalize(lightCoordiantes - intersect);

    observerRay  = normalize(intersect - rO);
    shadowRay    = normalize(intersect - lightCoordiantes);
    */

    float delta = 0.0001;
    vec4 shadowRayIntersect = raySphereIntersectionOld(intersect + delta*observerRay, shadowRay, sphereGeometry);
    if (shadowRayIntersect.w == INTERSECTED)
    {
        return phongIlluminationOld(ambientLight, vec4(0, 0, 0, 0), colorDiffuse, colorSpecular, 0, 0, 0);
    }
    
    vec3  reflectedRay  = reflect(shadowRay, normal);
    float normalAngle   = angleBetweenVec3(normal, shadowRay);
    float observerAngle = angleBetweenVec3(reflectedRay, observerRay);

    return phongIlluminationOld(ambientLight, lightProperties, colorDiffuse, colorSpecular, normalAngle, observerAngle, reflectionPerf);
}

void main()
{
    int obj = 0;
    GeometryColor  gc = makeGeometryColor(obj);
    GeometrySphere gs = makeGeometrySphere(obj);
    LightSource    ls = makeLightSource(0);

    vec4 ambientTest = vec4(ambientLight.r, ambientLight.g, ambientLight.b, 1.0);

    float sphereTest[13] = float[13]
    (
        gs.xCenter, gs.yCenter, gs.zCenter, gs.radius, gc.rDiffuse, gc.gDiffuse, gc.bDiffuse, 
        gc.diffuseConstant, gc.rSpecular, gc.gSpecular, gc.bSpecular, gc.specularConstant, gc.shininess
    );

    float lightTest[7] = float[7]
    (
        ls.x, ls.y, ls.z, ls.r, ls.g, ls.b, ls.attenuation
    );

    vec3 originTest = getOrigin();
    vec3 direction  = getRayDirection();

    gl_Position = vec4(pixelPos.x, pixelPos.y, 0, 1.0);
    //pixelNewColor = rayTest(getOrigin(), getRayDirection(), ambientTest, lightTest, sphereTest);

    pixelNewColor = rayTrace(getOrigin(), getRayDirection());

    //pixelNewColor = vec4(originTest.x, originTest.y, originTest.z, 1.0);
}

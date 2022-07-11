#version 330 core
#extension GL_NV_shader_buffer_load : enable
out vec4 pixelNewColor;
layout (location = 0) in vec3 pixelPos;
layout (location = 1) in vec2 pixelDislocations;

#define OBSERVER_STRUCT_SIZE 17
#define LIGHT_SOURCE_STRUCT_SIZE 8
#define GEOMETRY_COLOR_STRUCT_SIZE 13

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
#define GEOMETRY_HAS_NO_VOLUME 0
#define GEOMETRY_HAS_VOLUME 1

#define INTERSECTED     1.0
#define NOT_INTERSECTED 0.0

#define IS_SHADOW_RAY 1
#define IS_NOT_SHADOW_RAY 0

#define REFLECTION_ERR -0.0001
#define REFRACTION_ERR 0.0001

#define CHILD_SIGN_ROOT  0
#define CHILD_SIGN_REFLECTION 1
#define CHILD_SIGN_REFRACTION 2

#define UNRESOLVED_COLOR vec3(0.0, 0.0, 0.0)

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
    float refractionConstant;
    float hasVolume;
};

struct GeometrySphere
{
    float xCenter;
    float yCenter;
    float zCenter;
    float radius;
};

struct GeometryTriangle
{
    vec3 vertexA;
    vec3 vertexB;
    vec3 vertexC;
    vec3 normal;
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
GeometrySphere makeGeometrySphere(int shapeIndex)
{
    int pos = getShapeAbsolutePosition(shapeIndex);
    return GeometrySphere(shapes[pos + 0], shapes[pos + 1], shapes[pos + 2], shapes[pos + 3]);
}

GeometryTriangle makeGeometryTriangle(int shapeIndex)
{
    int pos = getShapeAbsolutePosition(shapeIndex);
    return GeometryTriangle(vec3(shapes[pos + 0], shapes[pos + 1], shapes[pos + 2]), vec3(shapes[pos + 3], shapes[pos + 4], shapes[pos + 5]), vec3(shapes[pos + 6], shapes[pos + 7], shapes[pos + 8]), vec3( shapes[pos + 9], shapes[pos + 10], shapes[pos + 11]));
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

float getGeometryRefractionConstant(int shapeIndex)
{
    return shapeColors[shapeIndex*GEOMETRY_COLOR_STRUCT_SIZE + 11];
}

float getGeometryVolumeFlag(int shapeIndex)
{
    return shapeColors[shapeIndex*GEOMETRY_COLOR_STRUCT_SIZE + 12];
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

bool isInsideSphere(vec3 point, GeometrySphere sphere, float errMod)
{
    return distance(point, vec3(sphere.xCenter, sphere.yCenter, sphere.zCenter)) <= sphere.radius - errMod;
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
    float t1 = ( discriminant - B)/2;

    if (t0 < 0 && t1 < 0)
        return vec4(origin.x + direction.x*t, origin.y + direction.y*t, origin.z + direction.z*t, NOT_INTERSECTED);  

    if (t0 <= t1 && t0 >= 0)
        t = t0;
    else
        t = t1;

    return vec4(origin.x + direction.x*t, origin.y + direction.y*t, origin.z + direction.z*t, INTERSECTED);   
}

vec3 sphereNormal(vec3 point, GeometrySphere sphere, float errMod)
{
    float insideFactor = 1;
    if (isInsideSphere(point, sphere, errMod))
        insideFactor = -1;
    
    return insideFactor*normalize(vec3((point.x - sphere.xCenter)/sphere.radius, (point.y - sphere.yCenter)/sphere.radius, (point.z - sphere.zCenter)/sphere.radius));
}

// https://cadxfem.org/inf/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf
vec4 rayTriangleIntersection(vec3 origin, vec3 direction, GeometryTriangle triangle)
{
    vec3 ab = triangle.vertexB - triangle.vertexA;
    vec3 ac = triangle.vertexC - triangle.vertexA;

    vec3  pvec = cross(direction, ac);
    float det  = dot(ab, pvec);

    if(det > -0.000001 && det < 0.000001)
        return vec4(0, 0, 0, NOT_INTERSECTED); 

    vec3  tvec = origin - triangle.vertexA;
    float u    = dot(tvec, pvec)*(1/det);

    if(u < 0.0 || u > 1.0)
        return vec4(0, 0, 0, NOT_INTERSECTED);

    vec3  qvec = cross(tvec, ab);
    float v    = dot(direction, qvec)*(1/det);

    if(v < 0.0 || u + v > 1.0)
        return vec4(0, 0, 0, NOT_INTERSECTED);

    float t = dot(ac, qvec)*(1/det);

    if(t < 0)
        return vec4(0, 0, 0, NOT_INTERSECTED);

    return vec4(origin.x + direction.x*t, origin.y + direction.y*t, origin.z + direction.z*t, INTERSECTED);  
}

vec3 triangleNormal(GeometryTriangle triangle)
{
    return triangle.normal;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * Illumination Models Functions * * * * * * * * * * * * * * * * * * * * * * * * * */
float phongIlluminationChannel(vec2 light, vec2 difC, vec2 speC, float normA, float obsA, float specPer, float transp)
{
    return light[1]*light[0]*(difC[1]*difC[0]*abs(cos(normA))*(1-transp) + speC[1]*speC[0]*pow(abs(cos(obsA)), specPer));
}

vec4 phongIllumination(vec3 lightC, float lightAtt, vec4 difC, vec4 speC, float normA, float obsA, float specPer, float transp)
{
    float r = phongIlluminationChannel(vec2(lightC.x, lightAtt), vec2(difC.x, difC.w), vec2(speC.x, speC.w), normA, obsA, specPer, transp);
    float g = phongIlluminationChannel(vec2(lightC.y, lightAtt), vec2(difC.y, difC.w), vec2(speC.y, speC.w), normA, obsA, specPer, transp);
    float b = phongIlluminationChannel(vec2(lightC.z, lightAtt), vec2(difC.z, difC.w), vec2(speC.z, speC.w), normA, obsA, specPer, transp);

    return vec4(r, g, b, 1);
}

vec4 phongIlluminationAmbientLight(vec3 ambientColor, float ambientConstant, vec4 difC, float transp)
{
    return vec4(ambientColor.r*difC.r*ambientConstant*(1-transp), ambientColor.g*difC.g*ambientConstant*(1-transp), ambientColor.b*difC.b*ambientConstant*(1-transp), 1);
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * Raytracing Algorithm Functions * * * * * * * * * * * * * * * * * * * * * * * * * */
GeometryIntersection rayIntersection(vec3 origin, vec3 direction, int objectID, float errMod)
{
    GeometryIntersection intersection;
    vec4                 intersect;
    vec3                 normal;

    if (shapeTypes[objectID] == GEOMETRY_TYPE_SPHERE)
    {
        GeometrySphere sphere = makeGeometrySphere(objectID);
        intersect             = raySphereIntersection(origin, direction, sphere);
        normal                = sphereNormal(vec3(intersect.x, intersect.y, intersect.z), sphere, errMod);
        return GeometryIntersection(intersect.w, vec3(intersect.x, intersect.y, intersect.z), normal); 
    }
    else if (shapeTypes[objectID] == GEOMETRY_TYPE_TRIANGLE)
    {
        GeometryTriangle triangle = makeGeometryTriangle(objectID);
        intersect                 = rayTriangleIntersection(origin, direction, triangle);
        normal                    = triangleNormal(triangle);
        return GeometryIntersection(intersect.w, vec3(intersect.x, intersect.y, intersect.z), normal); 
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
    float minDistance = 999999999999.9;
    GeometryIntersection intersection;
    RayCasted raycasted = RayCasted(0, 0, GeometryIntersection(NOT_INTERSECTED, vec3(0,0,0), vec3(0,0,0)));

    for(int objectID = 0; objectID < SHAPES_NUMBER; objectID++)
    {
        intersection = rayIntersection(origin, direction, objectID, 0.001);

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

vec4 shadowRayCastColor(vec3 intersectionPoint, vec3 intersectionDirection, vec3 normal, int objectID)
{
    vec4 rgb = vec4(0.0, 0.0, 0.0, 1);

    for(int lightID = 0; lightID < LIGHT_SOUCES_NUMBER; lightID++)
    {
        vec3 lightDirection       = getLightDirection(intersectionPoint, getLightSourcePosition(lightID));
        RayCasted shadowRaycasted = rayCast(intersectionPoint, lightDirection, IS_SHADOW_RAY);

        // Continue, since the light source is not ocluded
        if(hasIntersected(shadowRaycasted.intersection))
            continue;

        vec3 reflectedRay = reflect(lightDirection, normal);

        rgb += phongIllumination
              ( 
                getLightSourceColor(lightID),
                getLightSourceAttenuation(lightID), 
                getGeometryDiffuseColor(objectID),
                getGeometrySpecularColor(objectID),
                angleBetweenVec3(normal, lightDirection), 
                angleBetweenVec3(reflectedRay, intersectionDirection),
                getGeometryShininess(objectID),
                getGeometryTransparency(objectID)
              );
    }

    return rgb;
}

struct Witted
{
    vec3      origin                      ;
    vec3      direction                   ;
    RayCasted raycasted                   ;
    vec3      reflectionOrigin            ;
    vec3      refractionOrigin            ;
    vec3      reflectionDirection         ;
    vec3      refractionDirection         ;
    float     reflectionRefractionConstant;
    float     refractionRefractionConstant;
    vec3      localColor                  ;
};

float getCriticalRefractionAngle(float internalRefractionConstant, float externalRefractionConstant)
{
    return asin(externalRefractionConstant/internalRefractionConstant);
}

bool isTotalInternalReflection(float angle, float internalRefractionConstant, float externalRefractionConstant)
{
    return (internalRefractionConstant > externalRefractionConstant) && (angle > getCriticalRefractionAngle(internalRefractionConstant, externalRefractionConstant));
}

vec3 getRefractionDirection(vec3 direction, vec3 normal, int objectID, bool isTotalInternallyReflected, float internalRefractionConstant, float externalRefractionConstant)
{
    if(isTotalInternallyReflected)
        return reflect(direction, normal);
    else
        return refract(direction, normal, externalRefractionConstant/internalRefractionConstant);
}

vec3 getAjustedIntersectionPoint(vec3 origin, vec3 direction, vec3 intersectionPoint)
{
    return intersectionPoint + direction*REFLECTION_ERR*sqrt(distance(origin, intersectionPoint));
}

vec3 getReflectionOrigin(vec3 origin, vec3 direction, vec3 intersectionPoint)
{
    return intersectionPoint + direction*REFLECTION_ERR*sqrt(distance(origin, intersectionPoint));
}

vec3 getRefractionOrigin(vec3 origin, vec3 direction, vec3 intersectionPoint, bool isTotalInternallyReflected)
{
    if (isTotalInternallyReflected)
        return getReflectionOrigin(origin, direction, intersectionPoint);
    else
        return intersectionPoint + direction*REFRACTION_ERR*sqrt(distance(origin, intersectionPoint));
}

float getRefractedRayRefractionConstant(float internalRefractionConstant, float externalRefractionConstant, bool isTotalInternallyReflected, float hasVolume)
{
    if(isTotalInternallyReflected || (hasVolume == GEOMETRY_HAS_NO_VOLUME))
        return internalRefractionConstant;
    else
        return externalRefractionConstant;
}

// TODO: keep track of the ray origin`s material refraction constant
// update it every time it hits a surface, keep it in the stack
Witted witted(vec3 origin, vec3 direction, bool isRefractedRay, float currentRefractionConstant)
{
    RayCasted raycasted = rayCast(origin, direction, IS_NOT_SHADOW_RAY);
    
    if(hasNotIntersected(raycasted.intersection))
        return Witted(origin, direction, raycasted, raycasted.intersection.point, raycasted.intersection.point, direction, direction, currentRefractionConstant, currentRefractionConstant, UNRESOLVED_COLOR);

    bool isTotalInternallyReflected = isTotalInternalReflection(angleBetweenVec3(raycasted.intersection.normal, -direction), currentRefractionConstant, getGeometryRefractionConstant(raycasted.objectID));

    float newRefractionConstant;

    vec3 reflectionDirection = reflect(direction, raycasted.intersection.normal);
    vec3 refractionDirection = getRefractionDirection(direction, raycasted.intersection.normal, raycasted.objectID, isTotalInternallyReflected, getGeometryRefractionConstant(raycasted.objectID), currentRefractionConstant);
    vec3 reflectionOrigin    = getReflectionOrigin(origin, direction, raycasted.intersection.point);
    vec3 refractionOrigin    = getRefractionOrigin(origin, direction, raycasted.intersection.point, isTotalInternallyReflected);

    vec4 rgb = shadowRayCastColor(getAjustedIntersectionPoint(origin, direction, raycasted.intersection.point), direction, raycasted.intersection.normal, raycasted.objectID);

    rgb += phongIlluminationAmbientLight(ambientLight, getGeometryAmbientConstant(raycasted.objectID), getGeometryDiffuseColor(raycasted.objectID), getGeometryTransparency(raycasted.objectID));

    float refractedRayRefractionConstant = getRefractedRayRefractionConstant(currentRefractionConstant, getGeometryRefractionConstant(raycasted.objectID), isTotalInternallyReflected, getGeometryVolumeFlag(raycasted.objectID));

    return Witted(origin, direction, raycasted, reflectionOrigin, refractionOrigin, reflectionDirection, refractionDirection, currentRefractionConstant, refractedRayRefractionConstant, min(vec3(rgb.x, rgb.y, rgb.z), vec3(1.0, 1.0, 1.0)));
}

// https://stackoverflow.com/questions/35909453/c-c-power-of-two-macro
#define pwrtwo(x) (1 << (x))
#define MAX_RAYTRACE_DEPTH 5

struct Ray
{
    int    status;
    int    childSign;
    Witted witted;
    vec3   leftChildColor;
    vec3   rightChildColor;   
};

struct RayStack
{
    Ray stack[MAX_RAYTRACE_DEPTH];
    int size;
    int maxSize;
};

RayStack rayStack;

void rayStackPush(Ray ray)
{
    if(rayStack.size < rayStack.maxSize)
    {
        rayStack.stack[rayStack.size] = ray;
        rayStack.size += 1;
    }
}

void rayStackPop()
{
    if(rayStack.size > 0)
        rayStack.size -= 1;
}

Ray rayStackPeekAt(int pos)
{
    if(rayStack.size > pos)
        return rayStack.stack[pos];
    else
        return rayStack.stack[0];  
}

Ray rayStackPeek()
{
    return rayStackPeekAt(rayStack.size-1);
}

void raySetStatus(int status)
{
    if(rayStack.size > 0)
        rayStack.stack[rayStack.size-1].status = status;
}

void rayWriteColorParent(int childSign, vec3 resolvedColor)
{
    if(rayStack.size >= 2)
    {
        if(childSign == CHILD_SIGN_REFLECTION)
            rayStack.stack[rayStack.size-2].leftChildColor = resolvedColor;
        else if(childSign == CHILD_SIGN_REFRACTION)
            rayStack.stack[rayStack.size-2].rightChildColor = resolvedColor;
    }
}

#define STATUS_REFLECT 1
#define STATUS_REFRACT 2
#define STATUS_DONE    3

bool rayIsDone()
{
    return rayStack.stack[rayStack.size-1].status == STATUS_DONE;
}

bool rayIsReflected()
{
    return rayStack.stack[rayStack.size-1].status == STATUS_REFLECT;
}

bool rayIsRefracted()
{
    return rayStack.stack[rayStack.size-1].status == STATUS_REFRACT;
}

bool stackIsMaxSize()
{
    return rayStack.size == rayStack.maxSize;
}

bool stackIsRoot()
{
    return rayStack.size == 1;
}

#define IS_REFRACTED true
#define IS_REFLECTED false

float getInitialRefractionConstant()
{
    return 1.0;
}

vec3 rayTrace(vec3 origin, vec3 direction)
{
    rayStack.size    = 0;
    rayStack.maxSize = MAX_RAYTRACE_DEPTH; 

    float safetyCounter = pwrtwo(MAX_RAYTRACE_DEPTH-1)-1 + (pwrtwo(max(MAX_RAYTRACE_DEPTH-2, 0))-1)*3;
    safetyCounter = 100;

    rayStackPush(Ray(STATUS_REFLECT, CHILD_SIGN_ROOT, witted(origin, direction, IS_REFLECTED, getInitialRefractionConstant()), UNRESOLVED_COLOR, UNRESOLVED_COLOR));

    while(true)
    {
        if(safetyCounter == 0)
            return vec3(0.5, 0.5, 0.5);

        safetyCounter--;

        Ray ray      = rayStackPeek();
        int objectID = ray.witted.raycasted.objectID;

        if(hasNotIntersected(ray.witted.raycasted.intersection))
            raySetStatus(STATUS_DONE);

        if(stackIsMaxSize())
            raySetStatus(STATUS_DONE);

        if(rayIsDone())
        {                       
            vec3 rgb = ray.witted.localColor;

            if(hasIntersected(ray.witted.raycasted.intersection))
            {
                vec4 diffuseColor  = getGeometryDiffuseColor(objectID);
                vec4 specularColor = getGeometrySpecularColor(objectID);

                vec4 reflectionColor = phongIllumination
                ( 
                    ray.leftChildColor, 1.0, diffuseColor, specularColor,
                    angleBetweenVec3(ray.witted.raycasted.intersection.normal, ray.witted.reflectionDirection), 
                    angleBetweenVec3(ray.witted.reflectionDirection, ray.witted.direction),
                    0, getGeometryTransparency(objectID)
                );
                // Uncomment to render relections
                rgb += vec3(reflectionColor.x,  reflectionColor.y,  reflectionColor.z)*vec3(specularColor.x, specularColor.y, specularColor.z) ;
                rgb += ray.rightChildColor*vec3(diffuseColor.x, diffuseColor.y, diffuseColor.z);
            }

            if(stackIsRoot())
                return rgb;

            rayWriteColorParent(ray.childSign, rgb);
            rayStackPop();
        }
        else if(rayIsReflected())
        {
            raySetStatus(STATUS_REFRACT);
            rayStackPush(Ray(STATUS_REFLECT, CHILD_SIGN_REFLECTION, witted(ray.witted.reflectionOrigin, ray.witted.reflectionDirection, IS_REFLECTED, ray.witted.reflectionRefractionConstant), UNRESOLVED_COLOR, UNRESOLVED_COLOR));
        }
        else if(rayIsRefracted())
        {
            raySetStatus(STATUS_DONE);

            if(getGeometryTransparency(objectID) == 0.0)
                continue;

            // DEBUG: should be STATUS_REFLECT but something is wrong upstairs...
            rayStackPush(Ray(STATUS_REFLECT, CHILD_SIGN_REFRACTION, witted(ray.witted.refractionOrigin, ray.witted.refractionDirection, IS_REFRACTED, ray.witted.refractionRefractionConstant), UNRESOLVED_COLOR, UNRESOLVED_COLOR));
        }
    }
}

/*
vec4 rayCastOnce(vec3 origin, vec3 direction)
{
    RayCasted raycasted = rayCast(origin, direction, IS_NOT_SHADOW_RAY);
    
    if(hasNotIntersected(raycasted.intersection))
        return vec4(UNRESOLVED_COLOR.x, UNRESOLVED_COLOR.y, UNRESOLVED_COLOR.z, 1.0);

    // heuristic err modification
    float delta = 0.0001*sqrt(distance(origin, raycasted.intersection.point));
    vec3  intersectionDirection = getIntersectDirection(origin, raycasted.intersection.point);
    vec3  intersectionPointMod  = raycasted.intersection.point + delta*intersectionDirection;
    vec4  rgb = shadowRayCastColor(intersectionPointMod, intersectionDirection, raycasted.intersection.normal, raycasted.objectID);

    rgb += phongIlluminationAmbientLight(ambientLight, getGeometryAmbientConstant(raycasted.objectID), getGeometryDiffuseColor(raycasted.objectID), getGeometryTransparency(raycasted.objectID));

    return min(rgb, vec4(1.0, 1.0, 1.0, 1.0));
}
*/

void main()
{
    gl_Position = vec4(pixelPos.x, pixelPos.y, 0, 1.0);

    vec3 rgb = rayTrace(getOrigin(), getRayDirection());
    pixelNewColor = vec4(rgb.x, rgb.y, rgb.z, 1.0);
}
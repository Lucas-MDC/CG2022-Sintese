#version 330 core
layout (location = 0) in vec3 pixelPos;
layout (location = 1) in vec3 rayDirection;
uniform vec3 rayOrigin;

/*
    x, y, z position 
    radius
    r, g, b difuse   color
    difuse coefficient
    r, g, b specular color
    specular coefficient
    specularity perfection
*/
uniform float geometry[13];

/*
    r, g, b intensities
    ambient light coefficient
*/
uniform vec4 ambientLight;

/*
    x, y, z position
    r, g, b intensity
    light attenuation coefficient
*/
uniform float light[7];
out vec4 pixelNewColor;

#define INTERSECT     1.0
#define NOT_INTERSECT 0.0

/*
    r0 = ray origin    (x, y, z)
    rD = ray direction (x, y, z)
    s  = sphere (x center, y center, z center, radius)
*/
vec4 raySphereIntersection(vec3 rO, vec3 rD, vec4 s)
{
    float B = 2.0 * (rD.x * (rO.x - s.x) + rD.y * (rO.y - s.y) + rD.z * (rO.z - s.z));
    float C = pow(rO.x - s.x, 2) + pow(rO.y - s.y, 2) + pow(rO.z - s.z, 2) - pow(s.w, 2); 

    float discriminant = pow(B, 2) - 4*C;

    if (discriminant < 0)
        return vec4(0, 0, 0, NOT_INTERSECT);

    discriminant = sqrt(discriminant);

    float t  = 0;
    float t0 = (-discriminant - B)/2;
    float t1 = (discriminant  - B)/2;

    if(t0 <= t1)
        t = t0;
    else
        t = t1;

    if(t < 0)
        return vec4(0, 0, 0, NOT_INTERSECT);

    return vec4(rO.x + rD.x*t, rO.y + rD.y*t, rO.z + rD.z*t, INTERSECT);   
}

/*
    p = point in the spehre (x, y, z)
    s = sphere (x center, y center, z center, radius)
*/
vec3 sphereNormal(vec3 p, vec4 s)
{
    return normalize(vec3((p.x - s.x)/s.w, (p.y - s.y)/s.w, (p.z - s.z)/s.w));
}

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
float phongIlluminationChannel(vec2 amb, vec2 light, vec2 difC, vec2 speC, float normA, float obsA, float specPer)
{
    //return amb[0]*amb[1]*difC[0] + light[1]*light[0]*(difC[1]*difC[0]*cos(normA) + speC[1]*speC[0]*pow(cos(obsA), specPer));
    //debug code, remove abs after shadow ray application
    return amb[0]*amb[1]*difC[0] + light[1]*light[0]*(difC[1]*difC[0]*cos(abs(normA)) + speC[1]*speC[0]*pow(abs(cos(obsA)), specPer));
}

vec4 phongIllumination(vec4 ambL, vec4 light, vec4 difC, vec4 speC, float normA, float obsA, float specPer)
{
    float r = phongIlluminationChannel(vec2(ambL.x, ambL.w), vec2(light.x, light.w), vec2(difC.x, difC.w), vec2(speC.x, speC.w), normA, obsA, specPer);
    float g = phongIlluminationChannel(vec2(ambL.y, ambL.w), vec2(light.y, light.w), vec2(difC.y, difC.w), vec2(speC.y, speC.w), normA, obsA, specPer);
    float b = phongIlluminationChannel(vec2(ambL.z, ambL.w), vec2(light.z, light.w), vec2(difC.z, difC.w), vec2(speC.z, speC.w), normA, obsA, specPer);

    return vec4(r, g, b, 1);
}

float angleBetweenVec3(vec3 a, vec3 b)
{
    return acos(dot(a, b)/(length(a)*length(b)));
}

vec4 rayCast(vec3 rO, vec3 rD, vec4 ambientLight, float light[7], float sphere[13])
{
    vec4  sphereGeometry   = vec4(sphere[0], sphere[1], sphere[2] , sphere[3] );
    vec4  colorDiffuse     = vec4(sphere[4], sphere[5], sphere[6] , sphere[7] );
    vec4  colorSpecular    = vec4(sphere[8], sphere[9], sphere[10], sphere[11]);
    float reflectionPerf   = sphere[12];
    vec3  lightCoordiantes = vec3(light[0], light[1], light[2]);
    vec4  lightProperties  = vec4(light[3] , light[4] , light[5] , light[6]);

    vec4  rayIntersect = raySphereIntersection(rO, rD, sphereGeometry);
    
    float intersected  = rayIntersect.w;
    vec3  intersect    = vec3(rayIntersect.x, rayIntersect.y, rayIntersect.z);

    if(intersected == NOT_INTERSECT)
        return vec4(1, 1, 1, 0);

    vec3 normal       = sphereNormal(intersect, sphereGeometry);
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
    vec4 shadowRayIntersect = raySphereIntersection(intersect + delta*observerRay, shadowRay, sphereGeometry);
    if (shadowRayIntersect.w == INTERSECT)
    {
        return phongIllumination(ambientLight, vec4(0, 0, 0, 0), colorDiffuse, colorSpecular, 0, 0, 0);
    }
    
    vec3  reflectedRay  = reflect(shadowRay, normal);
    float normalAngle   = angleBetweenVec3(normal, shadowRay);
    float observerAngle = angleBetweenVec3(reflectedRay, observerRay);

    return phongIllumination(ambientLight, lightProperties, colorDiffuse, colorSpecular, normalAngle, observerAngle, reflectionPerf);
}

void main()
{
    gl_Position   = vec4(pixelPos.x, pixelPos.y, 0, 1.0);
    pixelNewColor = rayCast(rayOrigin, rayDirection, ambientLight, light, geometry);
}
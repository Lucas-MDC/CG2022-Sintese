#version 330 core
layout (location = 0) in vec3 pixelPos;
layout (location = 1) in vec3 pixelColor;
out vec3 pixelNewColor;
void main()
{
    gl_Position   = vec4(pixelPos.x, pixelPos.y, 0, 1.0);
    pixelNewColor = pixelColor;
}
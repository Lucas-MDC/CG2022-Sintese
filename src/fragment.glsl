#version 330 core
out vec4 FragColor;
in vec4  pixelNewColor;

void main()
{
    vec4(0.5, 0.0, 0.0, 1.0);
    FragColor =  vec4(pixelNewColor.r, pixelNewColor.g, pixelNewColor.b, pixelNewColor.a);
}
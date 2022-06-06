#version 330 core
out vec4 FragColor;
in vec3 pixelNewColor;
uniform float arr[5];
uniform float test[480000];

void main()
{
    float db = 1.0/480000.0;
    float b = 0;
    float f = 0;
    for(int i = 0; i < arr.length; i++)
        f += arr[i];

    for(int i = 0; i < test.length; i++)
        b+=db;

    FragColor =  vec4(pixelNewColor.r, pixelNewColor.g, b, 1.0);
}
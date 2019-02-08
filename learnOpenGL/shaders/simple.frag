#version 330 core

in vec4 normalColor;

out vec4 FragColor;

uniform vec4 ourColor;

void main()
{
    FragColor = normalColor;
    // FragColor = vec4(1,1,1,1);
}
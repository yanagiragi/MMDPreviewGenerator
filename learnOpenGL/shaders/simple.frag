#version 330 core

in vec2 uv;
in vec4 normalColor;

uniform vec4 ourColor;

uniform vec4 diffuseColor;
uniform vec3 specularColor;
uniform float gloss;
uniform vec3 ambientColor;

uniform sampler2D mainTex;

out vec4 FragColor;


void main()
{
    vec4 albedo = texture2D(mainTex, uv);

    FragColor = albedo;
    // FragColor = vec4(1,1,1,1);
}
#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D depthTexture;

void main()
{ 
	vec3 col = texture(screenTexture, TexCoords).rgb;
	
	vec4 tex = vec4(TexCoords, 1.0, 1.0);

	float depth = texture(depthTexture, TexCoords).r;

	//vec4 color = vec4(depth, depth, depth, 1.0);
	vec4 color = vec4(col, 1.0);
	//vec4 color = tex + vec4(col, 1.0);

    FragColor = color;
}
#version 330 

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 incol;
layout (location = 3) in vec2 vtexcoord0;


//uniform mat4 MVP;
out vec2 texcoord0;

out vec3 outcol;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 MVP;
void main()
{
    FragPos = aPos;
	Normal = normalize( aNormal);  
    outcol =  incol;
    gl_Position = MVP*vec4(aPos, 1.0);
    texcoord0 = vtexcoord0;
}
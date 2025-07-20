#version 450

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(set = 0, binding = 0) uniform ModelViewProj
{
	mat4	m_view;
	mat4	m_proj;
} mvp;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main()
{
	gl_Position = mvp.m_proj * mvp.m_view * inPosition;
	fragColor = inColor;
	fragTexCoord = inTexCoord;
}
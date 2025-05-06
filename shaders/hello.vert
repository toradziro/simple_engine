#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(binding = 0) uniform ModelViewProj
{
	mat4	m_model;
	mat4	m_view;
	mat4	m_proj;
} mvp;

layout(location = 0) out vec3 fragColor;

void main()
{
	gl_Position = mvp.m_model * mvp.m_view * mvp.m_proj * vec4(inPosition, 0.0, 1.0);
	fragColor = inColor;
}
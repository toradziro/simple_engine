#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(set = 0, binding = 0) uniform ModelViewProj
{
	mat4	m_model;
	mat4	m_view;
	mat4	m_proj;
} mvp;

layout(location = 0) out vec3 fragColor;

void main()
{
	vec4 worldPosition = mvp.m_model * vec4(inPosition, 0.0, 1.0);
	gl_Position = mvp.m_proj * mvp.m_view * worldPosition;
	fragColor = inColor;
}
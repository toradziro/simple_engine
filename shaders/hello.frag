#version 450

in vec4 v_color;
layout(location = 0) out vec4 color;

void main()
{
	o_color = v_color;
}
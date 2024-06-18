#version 330 core
layout (location = 0) in mat4 aModel;

out mat4 model;

void main()
{
	model = aModel;
}
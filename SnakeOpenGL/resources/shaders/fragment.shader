#version 460 core
#extension all : disable

uniform vec4 colorFromCPU;

layout(location = 0) out vec4 color;

void main() {
	color = colorFromCPU;
}
#version 460 core
#extension all : disable

layout(location = 0) in vec4 position;		  

void main() {
	gl_Position = position;	
}

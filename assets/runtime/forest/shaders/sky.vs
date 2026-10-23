#version 100
attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
uniform mat4 mvp;
varying vec2 uv;
void main(){uv=vertexTexCoord;gl_Position=mvp*vec4(vertexPosition,1.0);}

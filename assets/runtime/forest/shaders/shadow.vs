#version 100
precision highp float;
attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
uniform mat4 mvp;
uniform float uTime;
uniform float uWind;
varying vec2 uv;
void main(){
    vec3 pos=vertexPosition;
    float phase=pos.x*0.71+pos.z*0.49;
    float wind=sin(uTime*1.2+phase)+0.35*sin(uTime*2.6+phase*2.0);
    pos.xz+=vec2(0.06,0.025)*wind*uWind;
    uv=vertexTexCoord;
    gl_Position=mvp*vec4(pos,1.0);
}

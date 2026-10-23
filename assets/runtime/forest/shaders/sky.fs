#version 100
precision highp float;
varying vec2 uv;
uniform sampler2D texture0;
uniform vec3 eyeForward;
uniform vec3 eyeRight;
uniform vec3 eyeUp;
uniform float aspectRatio;
uniform float fovScale;
const float PI=3.141592653589793;
void main(){
    vec2 p=vec2((uv.x*2.0-1.0)*aspectRatio,1.0-uv.y*2.0)*fovScale;
    vec3 direction=normalize(eyeForward+eyeRight*p.x+eyeUp*p.y);
    vec2 hdrUv=vec2(0.5+atan(direction.z,direction.x)/(2.0*PI),0.5-asin(clamp(direction.y,-1.0,1.0))/PI);
    gl_FragColor=vec4(texture2D(texture0,hdrUv).rgb,1.0);
}

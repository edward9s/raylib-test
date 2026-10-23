#version 100
precision highp float;
attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec3 vertexNormal;
attribute vec4 vertexTangent;
attribute vec4 vertexColor;
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 lightVPNear;
uniform mat4 lightVPFar;
uniform float uTime;
uniform float uWind;
varying vec2 uv;
varying vec3 worldPos;
varying vec3 normal;
varying vec4 tangent;
varying vec4 tint;
varying vec4 shadowPosNear;
varying vec4 shadowPosFar;
void main() {
    vec3 pos = vertexPosition;
    float phase = pos.x*0.71 + pos.z*0.49;
    float wind = sin(uTime*1.2 + phase) + 0.35*sin(uTime*2.6 + phase*2.0);
    pos.xz += vec2(0.06, 0.025) * wind * uWind;
    vec4 world = matModel * vec4(pos, 1.0);
    worldPos = world.xyz;
    normal = normalize(mat3(matModel)*vertexNormal);
    tangent = vec4(normalize(mat3(matModel)*vertexTangent.xyz), vertexTangent.w);
    uv = vertexTexCoord;
    tint = vertexColor;
    shadowPosNear = lightVPNear * world;
    shadowPosFar = lightVPFar * world;
    gl_Position = mvp * vec4(pos, 1.0);
}

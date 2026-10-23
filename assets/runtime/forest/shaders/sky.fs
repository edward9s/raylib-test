#version 100
precision highp float;
varying vec2 uv;
uniform sampler2D texture0;
uniform vec3 eyeForward;
uniform vec3 eyeRight;
uniform vec3 eyeUp;
uniform vec3 sunDir;
uniform float aspectRatio;
uniform float fovScale;
const float PI=3.141592653589793;
void main() {
    vec2 p=vec2((uv.x*2.0-1.0)*aspectRatio,1.0-uv.y*2.0)*fovScale;
    vec3 direction=normalize(eyeForward+eyeRight*p.x+eyeUp*p.y);
    vec2 hdrUv=vec2(0.5+atan(direction.z,direction.x)/(2.0*PI),
                    0.5-asin(clamp(direction.y,-1.0,1.0))/PI);
    vec3 sky=texture2D(texture0,hdrUv).rgb;

    // Sun has a 0.53 degree angular diameter, matching the Filament setup.
    // The wider aureole approximates atmospheric forward scattering and bloom
    // without requiring HDR postprocessing on low-end GLES2 devices.
    float mu=clamp(dot(direction,normalize(sunDir)),-1.0,1.0);
    float disk=smoothstep(cos(0.0070),cos(0.0045),mu);
    float aureole=0.78*pow(max(mu,0.0),220.0)
                 +0.18*pow(max(mu,0.0),22.0);
    sky+=vec3(1.0,0.78,0.52)*aureole;
    sky=mix(sky,vec3(1.0,0.97,0.86),disk*0.98);
    gl_FragColor=vec4(clamp(sky,0.0,1.0),1.0);
}

#version 100
precision highp float;
varying vec2 uv;
varying vec3 worldPos;
varying vec3 normal;
varying vec4 tangent;
varying vec4 tint;
varying vec4 shadowPosNear;
varying vec4 shadowPosFar;
uniform sampler2D texture0;
uniform sampler2D shadowMapNear;
uniform sampler2D shadowMapFar;
uniform vec3 cameraPos;
uniform vec3 sunDir;
uniform vec3 fogColor;
uniform float shadowTexel;
uniform float uCanopy;
float decodeDepth(vec4 rgba) {
    return dot(rgba,vec4(1.0,1.0/255.0,1.0/65025.0,1.0/16581375.0));
}
float sampleCascade(sampler2D depthTex,vec3 projected,float bias) {
    float sum=0.0;
    for(int x=0;x<2;x++)for(int y=0;y<2;y++) {
        vec2 offset=(vec2(float(x),float(y))-vec2(0.5))*shadowTexel;
        vec2 coord=clamp(projected.xy+offset,
                         vec2(shadowTexel),vec2(1.0-shadowTexel));
        float depth=decodeDepth(texture2D(depthTex,coord));
        sum+=step(projected.z-bias,depth);
    }
    return sum*0.25;
}
float shadowVisibility(vec3 n) {
    vec3 nearP=shadowPosNear.xyz/shadowPosNear.w*0.5+0.5;
    vec3 farP=shadowPosFar.xyz/shadowPosFar.w*0.5+0.5;
    float bias=max(0.00012,0.0008*(1.0-abs(dot(n,sunDir))));
    float farShadow=1.0;
    if(farP.x>0.0&&farP.x<1.0&&farP.y>0.0&&farP.y<1.0&&
       farP.z>0.0&&farP.z<1.0)
        farShadow=sampleCascade(shadowMapFar,farP,bias);
    if(nearP.z<=0.0||nearP.z>=1.0)return farShadow;
    float edge=min(min(nearP.x,1.0-nearP.x),min(nearP.y,1.0-nearP.y));
    if(edge<=0.0)return farShadow;
    float nearShadow=sampleCascade(shadowMapNear,nearP,bias);
    // Prevent a hard line between near and far shadow resolutions.
    return mix(farShadow,nearShadow,smoothstep(0.015,0.065,edge));
}
void main() {
    vec3 albedo=tint.rgb;
    if(uCanopy>0.5) {
        vec4 pine=texture2D(texture0,uv);
        if(pine.a<0.40) discard;
        albedo=pow(pine.rgb,vec3(2.2));
    } else {
        float vein=smoothstep(0.015,0.045,abs(uv.x-0.5));
        float ribs=0.94+0.06*sin((uv.y+abs(uv.x-0.5)*0.6)*100.0);
        albedo*=mix(1.25,ribs,vein);
    }
    vec3 n=normalize(normal);
    float diff=abs(dot(n,sunDir));
    float shadow=shadowVisibility(n);
    vec3 transmitted=vec3(0.19,0.30,0.08)*diff*(1.0-shadow*0.5);
    vec3 color=albedo*(vec3(0.36)+vec3(1.08,0.97,0.77)*diff*shadow)+transmitted*albedo;
    float distanceToEye=length(cameraPos-worldPos);
    float fog=1.0-exp(-0.004*pow(max(distanceToEye-18.0,0.0),1.25));
    color=mix(color,fogColor,clamp(fog,0.0,0.55));
    gl_FragColor=vec4(pow(color/(vec3(1.0)+color*0.22),vec3(1.0/2.2)),1.0);
}

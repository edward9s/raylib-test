#version 100
precision highp float;
varying vec2 uv;
varying vec3 worldPos;
varying vec3 normal;
varying vec4 tangent;
varying vec4 tint;
varying vec4 shadowPos;
uniform sampler2D texture0;
uniform sampler2D shadowMap;
uniform vec3 cameraPos;
uniform vec3 sunDir;
uniform vec3 fogColor;
uniform float shadowTexel;
uniform float uCanopy;
float decodeDepth(vec4 rgba) {
    return dot(rgba, vec4(1.0,1.0/255.0,1.0/65025.0,1.0/16581375.0));
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
    vec3 projected=shadowPos.xyz/shadowPos.w*0.5+0.5;
    float shadow=1.0;
    if(projected.x>0.0&&projected.x<1.0&&projected.y>0.0&&projected.y<1.0&&projected.z>0.0&&projected.z<1.0) {
        float sum=0.0;
        for(int x=0;x<2;x++)for(int y=0;y<2;y++) {
            vec2 tap=(vec2(float(x),float(y))-vec2(0.5))*shadowTexel;
            vec2 coord=clamp(projected.xy+tap,vec2(shadowTexel),
                             vec2(1.0-shadowTexel));
            float depth=decodeDepth(texture2D(shadowMap,coord));
            sum+=(projected.z-0.0004<depth)?1.0:0.0;
        }
        shadow=sum*0.25;
    }
    vec3 transmitted=vec3(0.19,0.30,0.08)*diff*(1.0-shadow*0.5);
    vec3 color=albedo*(vec3(0.36)+vec3(1.08,0.97,0.77)*diff*shadow)+transmitted*albedo;
    float distanceToEye=length(cameraPos-worldPos);
    float fog=1.0-exp(-0.004*pow(max(distanceToEye-18.0,0.0),1.25));
    color=mix(color,fogColor,clamp(fog,0.0,0.55));
    gl_FragColor=vec4(pow(color/(vec3(1.0)+color*0.22),vec3(1.0/2.2)),1.0);
}

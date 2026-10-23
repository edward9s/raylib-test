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
uniform mat4 lightVP;
uniform vec3 cameraPos;
uniform vec3 sunDir;
uniform vec3 fogColor;
uniform float shadowTexel;
uniform float uCanopy;
float decodeDepth(vec4 rgba) {
    return dot(rgba,vec4(1.0,1.0/255.0,1.0/65025.0,1.0/16581375.0));
}
float visibilityAt(vec3 pos, float bias) {
    if(pos.x<=0.0 || pos.x>=1.0 || pos.y<=0.0 || pos.y>=1.0 ||
       pos.z<=0.0 || pos.z>=1.0) return 1.0;
    return step(pos.z-bias,decodeDepth(texture2D(shadowMap,pos.xy)));
}
float sunlightInAir() {
    vec3 ray=worldPos-cameraPos;
    float fullLength=length(ray);
    if(fullLength<0.001) return 0.0;
    float distance=min(fullLength,32.0);
    vec3 stepVec=ray/fullLength*(distance/6.0);
    float sum=0.0;
    for(int i=0;i<6;i++) {
        vec3 samplePos=cameraPos+stepVec*(float(i)+0.5);
        vec4 lightClip=lightVP*vec4(samplePos,1.0);
        vec3 projected=lightClip.xyz/lightClip.w*0.5+0.5;
        float illuminated=visibilityAt(projected,0.0018);
        sum+=illuminated*exp(-max(samplePos.y-0.9,0.0)*0.28);
    }
    float forward=max(dot(ray/fullLength,sunDir),0.0);
    return distance/6.0*sum*(0.028+0.050*pow(forward,3.0));
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
        for(int x=-1;x<=1;x++)for(int y=-1;y<=1;y++) {
            sum+=visibilityAt(projected+vec3(vec2(float(x),float(y))*shadowTexel,0.0),0.0015);
        }
        shadow=sum/9.0;
    }
    // Warm front lighting and green transmitted light on foliage.
    vec3 transmitted=vec3(0.19,0.30,0.08)*diff*(1.0-shadow*0.5);
    vec3 color=albedo*(vec3(0.28)+vec3(1.38,1.16,0.84)*diff*shadow)
              +transmitted*albedo;
    float distanceToEye=length(cameraPos-worldPos);
    float fog=1.0-exp(-0.004*pow(max(distanceToEye-18.0,0.0),1.25));
    color=mix(color,fogColor,clamp(fog,0.0,0.55));
    color+=vec3(1.0,0.78,0.51)*sunlightInAir();
    gl_FragColor=vec4(pow(max(color/(vec3(1.0)+color*0.22),vec3(0.0)),vec3(1.0/2.2)),1.0);
}

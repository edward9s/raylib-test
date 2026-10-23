#version 100
precision highp float;
varying vec2 uv;
varying vec3 worldPos;
varying vec3 normal;
varying vec4 tangent;
varying vec4 tint;
varying vec4 shadowPos;
uniform sampler2D texture0;
uniform sampler2D normalMap;
uniform sampler2D armMap;
uniform sampler2D shadowMap;
uniform mat4 lightVP;
uniform vec3 cameraPos;
uniform vec3 sunDir;
uniform vec3 fogColor;
uniform float shadowTexel;
float decodeDepth(vec4 rgba) {
    return dot(rgba,vec4(1.0,1.0/255.0,1.0/65025.0,1.0/16581375.0));
}
float visibilityAt(vec3 lightCoord, float bias) {
    if(lightCoord.x<=0.0 || lightCoord.x>=1.0 ||
       lightCoord.y<=0.0 || lightCoord.y>=1.0 ||
       lightCoord.z<=0.0 || lightCoord.z>=1.0) return 1.0;
    return step(lightCoord.z-bias,decodeDepth(texture2D(shadowMap,lightCoord.xy)));
}
float shadowVisibility(vec3 n) {
    vec3 projected=shadowPos.xyz/shadowPos.w*0.5+0.5;
    if(projected.x<0.0 || projected.x>1.0 || projected.y<0.0 || projected.y>1.0 ||
       projected.z<=0.0 || projected.z>=1.0) return 1.0;
    float bias=max(0.00045,0.0028*(1.0-dot(n,sunDir)));
    float sum=0.0;
    for(int x=-1;x<=1;x++) for(int y=-1;y<=1;y++) {
        sum+=visibilityAt(projected+vec3(vec2(float(x),float(y))*shadowTexel,0.0),bias);
    }
    return sum/9.0;
}
// Integrate warm airborne sunlight along the camera ray. Shadow-map lookups
// at intermediate world positions reveal sunlit gaps between trees/ruins and
// make actual, scene-dependent light shafts instead of a screen-space overlay.
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
        float mistDensity=exp(-max(samplePos.y-0.9,0.0)*0.28);
        sum+=illuminated*mistDensity;
    }
    float forward=max(dot(ray/fullLength,sunDir),0.0);
    return distance/6.0*sum*(0.028+0.050*pow(forward,3.0));
}
void main() {
    vec3 n=normalize(normal),t=normalize(tangent.xyz-n*dot(n,tangent.xyz));
    vec3 b=normalize(cross(n,t))*tangent.w;
    vec3 sampled=texture2D(normalMap,uv).xyz*2.0-1.0;
    n=normalize(mat3(t,b,n)*sampled);
    vec3 maps=texture2D(armMap,uv).rgb;
    vec3 albedo=pow(texture2D(texture0,uv).rgb,vec3(2.2))*tint.rgb;

    float diff=max(dot(n,sunDir),0.0);
    float shadow=shadowVisibility(n);
    float ambient=(0.23+0.18*max(n.y,0.0))*maps.r;
    vec3 sunlight=vec3(1.0,0.87,0.69)*(diff*1.95*shadow);
    vec3 viewDir=normalize(cameraPos-worldPos);
    vec3 halfDir=normalize(sunDir+viewDir);
    float spec=pow(max(dot(n,halfDir),0.0),mix(56.0,7.0,maps.g))
              *(1.0-maps.g)*shadow*0.19;
    vec3 color=albedo*(vec3(ambient)+sunlight)+vec3(1.0,0.90,0.75)*spec;
    float distanceToEye=length(cameraPos-worldPos);
    float fog=1.0-exp(-0.004*pow(max(distanceToEye-18.0,0.0),1.25));
    color=mix(color,fogColor,clamp(fog,0.0,0.55));
    color+=vec3(1.0,0.78,0.51)*sunlightInAir();
    color=color/(vec3(1.0)+color*0.22);
    gl_FragColor=vec4(pow(max(color,vec3(0.0)),vec3(1.0/2.2)),1.0);
}

#version 100
precision highp float;
uniform sampler2D texture0;
uniform float uMasked;
varying vec2 uv;
vec4 encodeDepth(float depth){
    vec4 enc=fract(depth*vec4(1.0,255.0,65025.0,16581375.0));
    enc-=enc.yzww*vec4(1.0/255.0,1.0/255.0,1.0/255.0,0.0);
    return enc;
}
void main(){
    if(uMasked>0.5&&texture2D(texture0,uv).a<0.4)discard;
    gl_FragColor=encodeDepth(gl_FragCoord.z);
}

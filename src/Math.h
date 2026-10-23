#pragma once
#include <cmath>
#include <cstddef>

namespace forest {
struct float2 {
    float x = 0, y = 0;
    constexpr float2() = default;
    constexpr float2(float a, float b): x(a), y(b) {}
    constexpr explicit float2(float a): x(a), y(a) {}
};
struct float3 {
    float x = 0, y = 0, z = 0;
    constexpr float3() = default;
    constexpr float3(float a, float b, float c): x(a), y(b), z(c) {}
    constexpr explicit float3(float a): x(a), y(a), z(a) {}
    float& operator[](std::size_t n) { return (&x)[n]; }
    float operator[](std::size_t n) const { return (&x)[n]; }
    float3& operator+=(float3 b) { x+=b.x; y+=b.y; z+=b.z; return *this; }
};
struct float4 {
    float x = 0, y = 0, z = 0, w = 0;
    constexpr float4() = default;
    constexpr float4(float a, float b, float c, float d): x(a), y(b), z(c), w(d) {}
};
inline float2 operator+(float2 a,float2 b) {return {a.x+b.x,a.y+b.y};}
inline float2 operator-(float2 a,float2 b) {return {a.x-b.x,a.y-b.y};}
inline float2 operator/(float2 a,float s) {return {a.x/s,a.y/s};}
inline float3 operator+(float3 a,float3 b) {return {a.x+b.x,a.y+b.y,a.z+b.z};}
inline float3 operator-(float3 a,float3 b) {return {a.x-b.x,a.y-b.y,a.z-b.z};}
inline float3 operator*(float3 a,float s) {return {a.x*s,a.y*s,a.z*s};}
inline float3 operator*(float s,float3 a) {return a*s;}
inline float3 operator*(float3 a,float3 b) {return {a.x*b.x,a.y*b.y,a.z*b.z};}
inline float3 operator/(float3 a,float s) {return a*(1.f/s);}
inline float dot(float3 a,float3 b) {return a.x*b.x+a.y*b.y+a.z*b.z;}
inline float3 cross(float3 a,float3 b) {return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
inline float length(float3 v) {return std::sqrt(dot(v,v));}
inline float3 normalize(float3 v) {const float n=length(v); return n>1e-8f?v/n:float3{0,1,0};}
} // namespace forest

#pragma once
#include "Math.h"
#include <cstdint>
#include <vector>
namespace forest {
struct Vertex {
    float3 position;
    float3 normal;
    float4 tangent;  // xyz tangent, w handedness for OpenGL normal map
    float2 uv;
    float4 color;
};
enum class Surface { Ground, Stone, Bark, Leaves, Canopy };
struct Mesh {
    Surface surface;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};
float elevation(float x, float z);
std::vector<Mesh> generate();
}

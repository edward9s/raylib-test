#include "Geometry.h"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
int main() {
    const auto a=forest::generate();
    const auto b=forest::generate();
    assert(a.size()==b.size());
    size_t vertices=0,triangles=0;
    bool surfaces[5]={};
    for(size_t i=0;i<a.size();++i) {
        const auto &x=a[i],&y=b[i];
        assert(x.surface==y.surface);
        assert(x.vertices.size()==y.vertices.size());
        assert(x.indices==y.indices);
        assert(x.vertices.size()<=65535);
        surfaces[static_cast<int>(x.surface)]=true;
        vertices+=x.vertices.size();triangles+=x.indices.size()/3;
        for(size_t j=0;j<x.vertices.size();++j) {
            const auto &v=x.vertices[j],&w=y.vertices[j];
            assert(v.position.x==w.position.x&&v.position.y==w.position.y&&v.position.z==w.position.z);
            assert(std::isfinite(v.position.x)&&std::isfinite(v.normal.x));
            assert(std::isfinite(v.tangent.w)&&std::isfinite(v.uv.x));
        }
        for(uint32_t index:x.indices)assert(index<x.vertices.size());
    }
    for(bool exists:surfaces)assert(exists);
    assert(triangles>200000);
    std::cout<<"OK: "<<a.size()<<" batches, "<<vertices<<" vertices, "<<triangles<<" triangles\n";
}

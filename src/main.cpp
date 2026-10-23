#include "Geometry.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <new>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
constexpr int SHADOW_SIZE=1024;
constexpr float PI=3.14159265358979323846f;
constexpr int MATERIALS=5;

#ifdef PLATFORM_ANDROID
constexpr const char *ROOT="forest/";
#else
constexpr const char *ROOT=".generated/assets/forest/";
#endif

std::string path(const char* filename) {return std::string(ROOT)+filename;}
Texture2D loadTexture(const char* name) {
    const std::string filename=path(name);
    Texture2D t=LoadTexture(filename.c_str());
    if(!IsTextureValid(t))throw std::runtime_error("Texture missing: "+filename);
    GenTextureMipmaps(&t);
    SetTextureFilter(t,TEXTURE_FILTER_TRILINEAR);
    SetTextureWrap(t,TEXTURE_WRAP_REPEAT);
    return t;
}
Shader loadShader(const char *vs,const char* fs) {
    const auto vert=path(vs),frag=path(fs);
    Shader shader=LoadShader(vert.c_str(),frag.c_str());
    if(!IsShaderValid(shader) || shader.id==rlGetShaderIdDefault())
        throw std::runtime_error("Shader failed to compile or link: "+frag);
    return shader;
}
int uniform(Shader shader,const char *name) {
    int loc=GetShaderLocation(shader,name);
    if(loc<0) throw std::runtime_error(std::string("Shader uniform missing: ")+name);
    return loc;
}
void scalar(Shader s,const char *name,float v) {SetShaderValue(s,uniform(s,name),&v,SHADER_UNIFORM_FLOAT);}
void vector3(Shader s,const char *name,Vector3 v) {SetShaderValue(s,uniform(s,name),&v,SHADER_UNIFORM_VEC3);}

struct Piece {
    ::Mesh mesh{};
    forest::Surface surface{};
};
class Forest {
public:
    Forest() {
        lit_=loadShader("shaders/forest.vs","shaders/surface.fs");
        leaf_=loadShader("shaders/forest.vs","shaders/foliage.fs");
        depth_=loadShader("shaders/shadow.vs","shaders/shadow.fs");
        sky_=loadShader("shaders/sky.vs","shaders/sky.fs");
        // DrawMesh binds texture units from MaterialMap indices. Shader.locs is
        // shared with the Material copy; use the original pointers, not copies.
        for(Shader *s:{&lit_,&leaf_}) {
            s->locs[SHADER_LOC_MAP_ALBEDO]=uniform(*s,"texture0");
            s->locs[SHADER_LOC_MAP_OCCLUSION]=uniform(*s,"shadowMap");
            uniform(*s,"lightVP");
            uniform(*s,"cameraPos");
            uniform(*s,"uWind");
            uniform(*s,"sunDir");
        }
        lit_.locs[SHADER_LOC_MAP_NORMAL]=uniform(lit_,"normalMap");
        lit_.locs[SHADER_LOC_MAP_ROUGHNESS]=uniform(lit_,"armMap");
        depth_.locs[SHADER_LOC_MAP_ALBEDO]=uniform(depth_,"texture0");
        sky_.locs[SHADER_LOC_MAP_ALBEDO]=uniform(sky_,"texture0");
        for (int i=0;i<10;++i) {
            const char *names[]={
                "ground_albedo.png","ground_normal.png","ground_arm.png",
                "stone_albedo.png","stone_normal.png","stone_arm.png",
                "bark_albedo.png","bark_normal.png","bark_arm.png","canopy.png"
            };
            textures_[i]=loadTexture(names[i]);
        }
        panorama_=loadTexture("environment.png");
        shadow_=LoadRenderTexture(SHADOW_SIZE,SHADOW_SIZE);
        if(!IsRenderTextureValid(shadow_))throw std::runtime_error("Shadow render target unavailable");
        SetTextureFilter(shadow_.texture,TEXTURE_FILTER_POINT);

        for(int i=0;i<MATERIALS;++i) {
            materials_[i]=LoadMaterialDefault();
            materials_[i].shader=i<3?lit_:leaf_;
            if(i<3) {
                materials_[i].maps[MATERIAL_MAP_ALBEDO].texture=textures_[i*3];
                materials_[i].maps[MATERIAL_MAP_NORMAL].texture=textures_[i*3+1];
                materials_[i].maps[MATERIAL_MAP_ROUGHNESS].texture=textures_[i*3+2];
            } else if(i==4) {
                materials_[i].maps[MATERIAL_MAP_ALBEDO].texture=textures_[9];
            }
            materials_[i].maps[MATERIAL_MAP_OCCLUSION].texture=shadow_.texture;
        }

        shadowMaterial_=LoadMaterialDefault();
        shadowMaterial_.shader=depth_;
        shadowCanopyMaterial_=LoadMaterialDefault();
        shadowCanopyMaterial_.shader=depth_;
        shadowCanopyMaterial_.maps[MATERIAL_MAP_ALBEDO].texture=textures_[9];
        lightCamera_={{15.36f,40.96f,20.5f},{0,1.6f,-3},{0,1,0},76.f,CAMERA_ORTHOGRAPHIC};
        const Matrix view=MatrixLookAt(lightCamera_.position,lightCamera_.target,{0,1,0});
        const Matrix proj=MatrixOrtho(-38,38,-38,38,0.1,120.0);
        const Matrix lightVP=MatrixMultiply(view,proj);
        for(Shader s:{lit_,leaf_}) {
            SetShaderValueMatrix(s,uniform(s,"lightVP"),lightVP);
            vector3(s,"sunDir",{0.32f,0.82f,0.47f});
            vector3(s,"fogColor",{0.36f,0.43f,0.44f});
            scalar(s,"shadowTexel",1.0f/SHADOW_SIZE);
        }
        auto geometry=forest::generate();
        pieces_.reserve(geometry.size());
        for(const auto& source:geometry) {
            if(source.indices.empty())continue;
            if(source.vertices.size()>65535)throw std::runtime_error("Forest tile exceeds 16-bit index limit");
            Piece piece;
            piece.surface=source.surface;
            ::Mesh &mesh=piece.mesh;
            mesh.vertexCount=static_cast<int>(source.vertices.size());
            mesh.triangleCount=static_cast<int>(source.indices.size()/3);
            mesh.vertices=static_cast<float*>(MemAlloc(mesh.vertexCount*3*sizeof(float)));
            mesh.normals=static_cast<float*>(MemAlloc(mesh.vertexCount*3*sizeof(float)));
            mesh.tangents=static_cast<float*>(MemAlloc(mesh.vertexCount*4*sizeof(float)));
            mesh.texcoords=static_cast<float*>(MemAlloc(mesh.vertexCount*2*sizeof(float)));
            mesh.colors=static_cast<unsigned char*>(MemAlloc(mesh.vertexCount*4));
            mesh.indices=static_cast<unsigned short*>(MemAlloc(source.indices.size()*sizeof(unsigned short)));
            if(!mesh.vertices||!mesh.normals||!mesh.tangents||!mesh.texcoords||!mesh.colors||!mesh.indices)
                throw std::bad_alloc();
            for(int j=0;j<mesh.vertexCount;++j) {
                const auto &v=source.vertices[j];
                for(int k=0;k<3;++k) {
                    mesh.vertices[3*j+k]=v.position[k];
                    mesh.normals[3*j+k]=v.normal[k];
                }
                mesh.tangents[j*4]=v.tangent.x;
                mesh.tangents[j*4+1]=v.tangent.y;
                mesh.tangents[j*4+2]=v.tangent.z;
                mesh.tangents[j*4+3]=v.tangent.w;
                mesh.texcoords[j*2]=v.uv.x;mesh.texcoords[j*2+1]=v.uv.y;
                const float c[]={v.color.x,v.color.y,v.color.z,v.color.w};
                for(int k=0;k<4;++k)mesh.colors[4*j+k]=static_cast<unsigned char>(std::clamp(c[k],0.f,1.f)*255.f+0.5f);
            }
            for(size_t j=0;j<source.indices.size();++j)
                mesh.indices[j]=static_cast<unsigned short>(source.indices[j]);
            UploadMesh(&mesh,false);
            pieces_.push_back(piece);
        }
    }
    ~Forest() {
        for(auto &p:pieces_)UnloadMesh(p.mesh);
        // UnloadMaterial would unload textures and shaders that are owned centrally.
        for(auto &m:materials_)MemFree(m.maps);
        MemFree(shadowMaterial_.maps);
        MemFree(shadowCanopyMaterial_.maps);
        UnloadRenderTexture(shadow_);
        for(auto &t:textures_)UnloadTexture(t);
        UnloadTexture(panorama_);
        UnloadShader(sky_);UnloadShader(depth_);UnloadShader(leaf_);UnloadShader(lit_);
    }
    void draw(Camera3D camera,float seconds) {
        rlSetClipPlanes(0.1,120.0);
        BeginDrawing();
        BeginTextureMode(shadow_);
        ClearBackground(WHITE);
        BeginMode3D(lightCamera_);
        rlDisableBackfaceCulling();
        for(const auto &p:pieces_) {
            if(p.surface==forest::Surface::Ground)continue;
            float wind=p.surface==forest::Surface::Canopy?1.f:p.surface==forest::Surface::Leaves?.3f:0.f;
            scalar(depth_,"uTime",seconds);
            scalar(depth_,"uWind",wind);
            scalar(depth_,"uMasked",p.surface==forest::Surface::Canopy?1.f:0.f);
            const ::Material &m=p.surface==forest::Surface::Canopy?shadowCanopyMaterial_:shadowMaterial_;
            DrawMesh(p.mesh,m,MatrixIdentity());
        }
        rlEnableBackfaceCulling();
        EndMode3D();
        EndTextureMode();

        ClearBackground({63,76,75,255});
        drawSky(camera);
        BeginMode3D(camera);
        for(Shader s:{lit_,leaf_}) {
            vector3(s,"cameraPos",camera.position);
            scalar(s,"uTime",seconds);
        }
        for(const auto& p:pieces_) {
            const bool canopy=p.surface==forest::Surface::Canopy;
            const bool leaves=p.surface==forest::Surface::Leaves;
            const Shader s=(canopy||leaves)?leaf_:lit_;
            scalar(s,"uWind",canopy?1.f:leaves?.3f:0.f);
            if(canopy||leaves)scalar(s,"uCanopy",canopy?1.f:0.f);
            if(canopy||leaves)rlDisableBackfaceCulling();
            else rlEnableBackfaceCulling();
            DrawMesh(p.mesh,materials_[static_cast<int>(p.surface)],MatrixIdentity());
        }
        rlEnableBackfaceCulling();
        EndMode3D();
        DrawFPS(14,14);
        EndDrawing();
    }
private:
    void drawSky(Camera3D camera) {
        const Vector3 f=Vector3Normalize(Vector3Subtract(camera.target,camera.position));
        const Vector3 right=Vector3Normalize(Vector3CrossProduct(f,camera.up));
        const Vector3 up=Vector3Normalize(Vector3CrossProduct(right,f));
        const float aspect=float(GetRenderWidth())/std::max(1,GetRenderHeight());
        const float scale=std::tan(camera.fovy*PI/360.f);
        vector3(sky_,"eyeForward",f);
        vector3(sky_,"eyeRight",right);
        vector3(sky_,"eyeUp",up);
        scalar(sky_,"aspectRatio",aspect);
        scalar(sky_,"fovScale",scale);
        BeginShaderMode(sky_);
        DrawTexturePro(panorama_,{0,0,(float)panorama_.width,(float)panorama_.height},
            {0,0,(float)GetRenderWidth(),(float)GetRenderHeight()},{0,0},0,WHITE);
        EndShaderMode();
    }
    Shader lit_{},leaf_{},depth_{},sky_{};
    std::array<Texture2D,10> textures_{};
    Texture2D panorama_{};
    RenderTexture2D shadow_{};
    std::array<::Material,MATERIALS> materials_{};
    ::Material shadowMaterial_{},shadowCanopyMaterial_{};
    Camera3D lightCamera_{};
    std::vector<Piece> pieces_;
};
struct Orbit {
    float yaw=.24f,pitch=.12f,distance=13.f;
    Vector2 previous[2]{};
    int previousCount=0;
    void update() {
        const int count=GetTouchPointCount();
        if(count==1) {
            Vector2 current=GetTouchPosition(0);
            if(previousCount==1)turn((current.x-previous[0].x)/std::max(GetScreenWidth(),1),
                                      (current.y-previous[0].y)/std::max(GetScreenHeight(),1));
            previous[0]=current;
        } else if(count>=2) {
            Vector2 a=GetTouchPosition(0),b=GetTouchPosition(1);
            if(previousCount>=2) {
                float old=Vector2Distance(previous[0],previous[1]);
                float now=Vector2Distance(a,b);
                if(now>1&&old>1)distance=std::clamp(distance*old/now,4.f,24.f);
            }
            previous[0]=a;previous[1]=b;
        } else {
            if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                const Vector2 delta=GetMouseDelta();
                turn(delta.x/std::max(GetScreenWidth(),1),delta.y/std::max(GetScreenHeight(),1));
            }
            distance=std::clamp(distance-GetMouseWheelMove(),4.f,24.f);
        }
        previousCount=count;
    }
    void turn(float dx,float dy) {
        yaw-=dx*4.f; pitch=std::clamp(pitch+dy*2.f,.03f,.85f);
    }
    Camera3D camera() const {
        float h=distance*std::cos(pitch);
        return {{h*std::sin(yaw),1.6f+distance*std::sin(pitch),-3+h*std::cos(yaw)},
                {0,1.6f,-3},{0,1,0},52,CAMERA_PERSPECTIVE};
    }
};
} // namespace

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1280,720,"Forest Ruins - raylib 6.2");
    SetTargetFPS(60);
    int result=0;
    try {
        Forest forest;
        Orbit orbit;
        while(!WindowShouldClose()) {
            orbit.update();
            forest.draw(orbit.camera(),static_cast<float>(GetTime()));
        }
    } catch(const std::exception &e) {
        TraceLog(LOG_ERROR,"Forest failed: %s",e.what());
        result=1;
    }
    CloseWindow();
    return result;
}

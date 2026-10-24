#include "bsp/avoid_zone_dyn_hull.hpp"
#include <algorithm>
#include <array>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <limits>
#include <memory>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using V = OceanVec3;
// Explicit x87 arithmetic retains the extended intermediates and binary32
// stores visible in this Dyn build. Valid finite inputs only; see the report.
float sum_products(float a, float b, float c, float d, float e, float f) {
    float r;
    __asm {
        fld a
        fmul b
        fld c
        fmul d
        faddp st(1), st(0)
        fld e
        fmul f
        faddp st(1), st(0)
        fstp r
    }
    return r;
}
float product_difference(float a, float b, float c, float d) {
    float r;
    __asm {
        fld a
        fmul b
        fld c
        fmul d
        fsubp st(1), st(0)
        fstp r
    }
    return r;
}
float fadd(float a, float b) { float r;
    __asm fld a
    __asm fadd b
    __asm fstp r
    return r;
}
float fsub(float a, float b) { float r;
    __asm fld a
    __asm fsub b
    __asm fstp r
    return r;
}
float fmul(float a, float b) { float r;
    __asm fld a
    __asm fmul b
    __asm fstp r
    return r;
}
float fdiv(float a, float b) { float r;
    __asm fld a
    __asm fdiv b
    __asm fstp r
    return r;
}
float double_mul(float a, double b) { float r;
    __asm fld a
    __asm fmul b
    __asm fstp r
    return r;
}
float magnitude(V a) {
    const float q = sum_products(a.x,a.x,a.y,a.y,a.z,a.z);
    float r;
    __asm fld q
    __asm fsqrt
    __asm fstp r
    return r;
}
V add(V a,V b) { return {fadd(a.x,b.x),fadd(a.y,b.y),fadd(a.z,b.z)}; }
V sub(V a,V b) { return {fsub(a.x,b.x),fsub(a.y,b.y),fsub(a.z,b.z)}; }
V mul(V a,float b) { return {fmul(a.x,b),fmul(a.y,b),fmul(a.z,b)}; }
V negate(V a) { return {0.0f-a.x,0.0f-a.y,0.0f-a.z}; }
V cross(V a,V b) {
    return {product_difference(a.y,b.z,a.z,b.y),
            product_difference(a.z,b.x,a.x,b.z),
            product_difference(a.x,b.y,a.y,b.x)};
}
float dot(V a,V b) { return sum_products(a.x,b.x,a.y,b.y,a.z,b.z); }
V normalize_00c482b0(V v) {
    float length=magnitude(v);
    if (length==0.0f) length=0.1f;
    return mul(v,fdiv(1.0f,length));
}
V normal_00c48170(V a,V b,V c) {
    V n=cross(sub(b,a),sub(c,b));
    const float length=magnitude(n);
    if (length==0.0f) return {1,0,0};
    return mul(n,fdiv(1.0f,length));
}
V orthogonal_00c517d0(V v) {
    const V a=cross(v,{0,0,1}), b=cross(v,{0,1,0});
    return normalize_00c482b0(magnitude(a)>magnitude(b)?a:b);
}
void sine_cosine(float angle,float& sine,float& cosine) {
    float s,c;
    __asm fld angle
    __asm fsin
    __asm fstp s
    __asm fld angle
    __asm fcos
    __asm fstp c
    sine=s;cosine=c;
}

// 0040EF40: equality keeps the first allowed point; this is not a > rewrite
// for unordered values. Such exceptional inputs are outside this API domain.
int max_direction_filtered_0040ef40(const std::vector<V>& p,V direction,
                                   const std::vector<int>& allow) {
    int best=-1;
    for (std::size_t i=0;i<p.size();++i) {
        if (!allow[i]) continue;
        if (best!=-1 && dot(direction,p[i])<=dot(direction,p[best])) continue;
        best=static_cast<int>(i);
    }
    if (best==-1) throw std::domain_error("Dyn hull has no allowed support vertex");
    return best;
}
// 0040C790: confirm a support vertex by the native angular neighborhood;
// disabled candidates stay disabled for subsequent simplex/extrusion queries.
int max_direction_0040c790(const std::vector<V>& p,V direction,
                          std::vector<int>& allow) {
    for (;;) {
        const int best=max_direction_filtered_0040ef40(p,direction,allow);
        if (allow[best]==3) return best;
        const V u=orthogonal_00c517d0(direction), v=cross(u,direction);
        int previous=-1;
        const auto sample=[&](float degrees) {
            // Native double is the float PI constant widened then divided by180.
            const double radians_per_degree=0.017453293005625408;
            const float angle=double_mul(degrees,radians_per_degree);
            float sine,cosine;
            sine_cosine(angle,sine,cosine);
            const V around=add(mul(u,sine),mul(v,cosine));
            return max_direction_filtered_0040ef40(p,add(direction,mul(around,0.025f)),allow);
        };
        for (float angle=0;angle<=360;angle=fadd(angle,45)) {
            const int current=sample(angle);
            if (previous==best && current==best) { allow[best]=3;return best; }
            if (previous!=-1 && previous!=current) {
                for(float fine=fsub(angle,40);fine<=angle;fine=fadd(fine,5)) {
                    const int candidate=sample(fine);
                    if(previous==best && candidate==best) { allow[best]=3;return best; }
                    previous=candidate;
                }
            }
            previous=current;
        }
        allow[best]=0;
    }
}
std::array<int,4> simplex_00c58920(const std::vector<V>& p,std::vector<int>& allow) {
    const std::array<int,4> fail{-1,-1,-1,-1};
    const int p0=max_direction_0040c790(p,{0.01f,0.02f,1},allow);
    const int p1=max_direction_0040c790(p,{-0.01f,-0.02f,-1},allow);
    const V basis=sub(p[p0],p[p1]);
    if(p0==p1 || dot(basis,basis)==0) return fail;
    const V u=cross({1,0.02f,0},basis), v=cross({-0.02f,1,0},basis);
    V direction=normalize_00c482b0(magnitude(u)>magnitude(v)?u:v);
    int p2=max_direction_0040c790(p,direction,allow);
    if(p2==p0 || p2==p1) p2=max_direction_0040c790(p,negate(direction),allow);
    if(p2==p0 || p2==p1) return fail;
    direction=normalize_00c482b0(cross(sub(p[p2],p[p0]),basis));
    int p3=max_direction_0040c790(p,direction,allow);
    if(p3==p0 || p3==p1 || p3==p2)
        p3=max_direction_0040c790(p,negate(direction),allow);
    if(p3==p0 || p3==p1 || p3==p2) return fail;
    if(dot(sub(p[p3],p[p0]),cross(sub(p[p1],p[p0]),sub(p[p2],p[p0])))<0)
        std::swap(p2,p3);
    return {p0,p1,p2,p3};
}
struct Triangle {
    int vertex[3];
    int neighbor[3];
    int id;
    int farthest=-1;
    float rise=0;
};
using Triangles=std::vector<std::unique_ptr<Triangle>>;
int append_triangle_004060e0(Triangles& t,int a,int b,int c) {
    const int id=static_cast<int>(t.size());
    t.push_back(std::make_unique<Triangle>(Triangle{{a,b,c},{-1,-1,-1},id,-1,0}));
    return id;
}
int& neighbor_00c36730(Triangle& t,int a,int b) {
    for(int i=0;i<3;++i) {
        const int j=(i+1)%3;
        if((t.vertex[i]==a && t.vertex[j]==b) || (t.vertex[i]==b && t.vertex[j]==a))
            return t.neighbor[(i+2)%3];
    }
    throw std::domain_error("Dyn triangle neighbor edge missing (native fallback E1750C)");
}
bool contains(const Triangle& t,int v) {
    return t.vertex[0]==v || t.vertex[1]==v || t.vertex[2]==v;
}
void splice_back_to_back_00c38f90(Triangles& all,Triangle& a,Triangle& b) {
    for(int i=0;i<3;++i) {
        const int u=a.vertex[(i+1)%3],v=a.vertex[(i+2)%3];
        const int bn=neighbor_00c36730(b,v,u);
        const int an=neighbor_00c36730(a,u,v);
        neighbor_00c36730(*all[an],v,u)=bn;
        const int an_again=neighbor_00c36730(a,u,v);
        const int bn_again=neighbor_00c36730(b,v,u);
        neighbor_00c36730(*all[bn_again],u,v)=an_again;
    }
}
void extrude_00c51480(Triangles& all,int source,int v) {
    const Triangle old=*all[source];
    const int first=static_cast<int>(all.size());
    int added[3];
    for(int i=0;i<3;++i) {
        added[i]=append_triangle_004060e0(all,v,old.vertex[(i+1)%3],old.vertex[(i+2)%3]);
        Triangle& t=*all[added[i]];
        t.neighbor[0]=old.neighbor[i];
        t.neighbor[1]=first+(i+1)%3;
        t.neighbor[2]=first+(i+2)%3;
        neighbor_00c36730(*all[old.neighbor[i]],old.vertex[(i+1)%3],old.vertex[(i+2)%3])=added[i];
    }
    for(int i=0;i<3;++i) {
        Triangle& t=*all[added[i]];
        const int other=t.neighbor[0];
        if(contains(*all[other],v)) {
            splice_back_to_back_00c38f90(all,t,*all[other]);
            all[added[i]].reset();all[other].reset();
        }
    }
    all[source].reset();
}
V triangle_normal(const std::vector<V>& p,const Triangle& t) {
    return normal_00c48170(p[t.vertex[0]],p[t.vertex[1]],p[t.vertex[2]]);
}
bool above_00c51740(const std::vector<V>& p,const Triangle& t,V point,float epsilon) {
    return epsilon<dot(triangle_normal(p,t),sub(point,p[t.vertex[0]]));
}
int extrudable_00c32860(const Triangles& all,float epsilon) {
    int best=-1;
    for(std::size_t i=0;i<all.size();++i)
        if(all[i] && (best<0 || all[best]->rise<all[i]->rise)) best=static_cast<int>(i);
    if(best<0) throw std::domain_error("Dyn hull has no triangle");
    return all[best]->rise<=epsilon?-1:best;
}
bool hull_00c5cae0(const std::vector<V>& p,Triangles& all,int limit) {
    if(p.size()<4) return false;
    if(!limit) limit=1000000000;
    V minimum=p[0],maximum=p[0];
    std::vector<int> extreme(p.size(),0),allow(p.size(),1);
    for(V v:p) {
        minimum={std::min(minimum.x,v.x),std::min(minimum.y,v.y),std::min(minimum.z,v.z)};
        maximum={std::max(maximum.x,v.x),std::max(maximum.y,v.y),std::max(maximum.z,v.z)};
    }
    const float epsilon=fmul(magnitude(sub(maximum,minimum)),0.001f);
    const auto s=simplex_00c58920(p,allow);
    if(s[0]==-1) return false;
    const V center=mul(add(add(add(p[s[1]],p[s[0]]),p[s[2]]),p[s[3]]),0.25f);
    append_triangle_004060e0(all,s[2],s[3],s[1]);
    append_triangle_004060e0(all,s[3],s[2],s[0]);
    append_triangle_004060e0(all,s[0],s[1],s[3]);
    append_triangle_004060e0(all,s[1],s[0],s[2]);
    const int neighbors[4][3]={{2,3,1},{3,2,0},{0,1,3},{1,0,2}};
    for(int i=0;i<4;++i) {
        std::copy(neighbors[i],neighbors[i]+3,all[i]->neighbor);
        extreme[s[i]]=1;
        const V n=triangle_normal(p,*all[i]);
        all[i]->farthest=max_direction_0040c790(p,n,allow);
        all[i]->rise=dot(n,sub(p[all[i]->farthest],p[all[i]->vertex[0]]));
    }
    for(limit-=4;limit>0;--limit) {
        const int selected=extrudable_00c32860(all,epsilon);
        if(selected<0) break;
        const int vertex=all[selected]->farthest;
        extreme[vertex]=1;
        const float plane_epsilon=fmul(epsilon,0.01f);
        for(int j=static_cast<int>(all.size());j--;) {
            if(all[j] && above_00c51740(p,*all[j],p[vertex],plane_epsilon))
                extrude_00c51480(all,j,vertex);
        }
        // Native restarts this backward pass after every repair extrusion.
        for(int j=static_cast<int>(all.size());j--;) {
            if(!all[j]) continue;
            const Triangle& t=*all[j];
            if(!contains(t,vertex)) break;
            const V area=cross(sub(p[t.vertex[1]],p[t.vertex[0]]),sub(p[t.vertex[2]],p[t.vertex[1]]));
            const double area_limit=static_cast<double>(epsilon)*epsilon*static_cast<double>(0.1f);
            if(above_00c51740(p,t,center,plane_epsilon) || magnitude(area)<area_limit) {
                extrude_00c51480(all,t.neighbor[0],vertex);
                j=static_cast<int>(all.size());
            }
        }
        for(int j=static_cast<int>(all.size());j--;) {
            if(!all[j]) continue;
            Triangle& t=*all[j];
            if(t.farthest>=0) break;
            const V n=triangle_normal(p,t);
            t.farthest=max_direction_0040c790(p,n,allow);
            if(extreme[t.farthest]) t.farthest=-1;
            else t.rise=dot(n,sub(p[t.farthest],p[t.vertex[0]]));
        }
    }
    return true;
}

void bounds(const std::vector<V>& p,V& lo,V& hi) {
    lo={FLT_MAX,FLT_MAX,FLT_MAX};hi={-FLT_MAX,-FLT_MAX,-FLT_MAX};
    for(V v:p) {
        lo={std::min(lo.x,v.x),std::min(lo.y,v.y),std::min(lo.z,v.z)};
        hi={std::max(hi.x,v.x),std::max(hi.y,v.y),std::max(hi.z,v.z)};
    }
}
void degenerate_box(std::vector<V>& p,V center,V extent,bool strict) {
    constexpr float epsilon=0.000001f;
    float smallest=FLT_MAX;
    for(float value:{extent.x,extent.y,extent.z})
        if((strict?epsilon<value:epsilon<=value) && value<smallest) smallest=value;
    if(smallest==FLT_MAX) extent={0.01f,0.01f,0.01f};
    else {
        if(extent.x<epsilon) extent.x=fmul(smallest,0.05f);
        if(extent.y<epsilon) extent.y=fmul(smallest,0.05f);
        if(extent.z<epsilon) extent.z=fmul(smallest,0.05f);
    }
    const V lo=sub(center,extent),hi=add(center,extent);
    p={{lo.x,lo.y,lo.z},{hi.x,lo.y,lo.z},{hi.x,hi.y,lo.z},{lo.x,hi.y,lo.z},
       {lo.x,lo.y,hi.z},{hi.x,lo.y,hi.z},{hi.x,hi.y,hi.z},{lo.x,hi.y,hi.z}};
}
// 00C35E80 as 00C5DDD0's descriptor has 00C5DAE0 call it: packed XYZ, epsilon
// 0.001, a scale output. The whole body is dyn_hull_cleanup_points_00c35e80
// below, which avoid_zone_dyn_hull_generate_00c5dae0 calls directly. No caller
// remains; kept for reference because the ledger's cleanup_00c35e80 fragment
// row (00C35E80) names it.
std::vector<V> cleanup_00c35e80(const V* input,std::uint32_t count,V& scale) {
    std::vector<V> clean(count>8?count:8);
    std::uint32_t kept=0;
    dyn_hull_cleanup_points_00c35e80(clean.data(),count,input,sizeof(V),&kept,0.001f,&scale);
    clean.resize(kept);
    return clean;
}
// One axis of the bounds loops 00C35F23-00C35F5D and 00C362AB-00C362E5:
// min > v takes v, then v > max takes v (FCOMIP / JBE, so a NaN is skipped).
void widen(float v,float& lo,float& hi) {
    if(lo>v) lo=v;
    if(v>hi) hi=v;
}
// The two inline box tails, 00C3655F-00C36711 (strict) and 00C36368-00C36509.
void cleanup_box_00c35e80(V* out,V center,V extent,bool strict) {
    std::vector<V> box;
    degenerate_box(box,center,extent,strict);
    std::copy(box.begin(),box.end(),out);
}
void* allocate(const AvoidZoneDynHullMemory& memory,std::size_t bytes) {
    if(!memory.allocate || !memory.release) throw std::invalid_argument("Dyn hull allocator required");
    void* result=memory.allocate(memory.context,bytes);
    if(!result) throw std::bad_alloc();
    return result;
}
void release(const AvoidZoneDynHullMemory& memory,void* p) {
    if(p) memory.release(memory.context,p);
}
AvoidZoneDynHullData* new_data(const AvoidZoneDynHullMemory& m) {
    auto* data=static_cast<AvoidZoneDynHullData*>(allocate(m,sizeof(AvoidZoneDynHullData)));
    // Do not invent initial values for the native unspecified tail.
    std::memset(data,0,0x18);return data;
}
void reserve_vertices(AvoidZoneDynHullData& data,std::uint32_t count,const AvoidZoneDynHullMemory& m) {
    if(data.vertex_capacity>=count) return;
    auto* next=static_cast<AvoidZoneDynHullVertex*>(allocate(m,count*sizeof(AvoidZoneDynHullVertex)));
    if(data.vertex_count) std::memcpy(next,data.vertices,data.vertex_count*sizeof(*next));
    release(m,data.vertices);data.vertices=next;data.vertex_capacity=count;
}
void reserve_adjacency(AvoidZoneDynHullData& data,std::uint32_t count,const AvoidZoneDynHullMemory& m) {
    if(data.adjacency_capacity>=count) return;
    auto* next=static_cast<std::uint16_t*>(allocate(m,count*sizeof(std::uint16_t)));
    if(data.adjacency_count) std::memcpy(next,data.adjacency,data.adjacency_count*sizeof(*next));
    release(m,data.adjacency);data.adjacency=next;data.adjacency_capacity=count;
}
} // namespace

bool dyn_hull_cleanup_points_00c35e80(V* out,std::uint32_t count,const void* source,
                                      std::uint32_t stride,std::uint32_t* count_out,
                                      float epsilon,V* scale_out) {
    constexpr float tiny=0.000001f;                    // [00D7A288]
    *count_out=0;                                       // 00C35E94
    if(scale_out) *scale_out={1.0f,1.0f,1.0f};          // 00C35E9C-00C35EAD, [00D7A24C]
    // 00C35EB2-00C35F64: bounds of the strided source, from +-FLT_MAX.
    const auto* bytes=static_cast<const unsigned char*>(source);
    V lo{FLT_MAX,FLT_MAX,FLT_MAX},hi{-FLT_MAX,-FLT_MAX,-FLT_MAX};
    for(std::uint32_t i=0;i<count;++i) {
        V v;std::memcpy(&v,bytes+static_cast<std::size_t>(i)*stride,sizeof v);
        widen(v.x,lo.x,hi.x);widen(v.y,lo.y,hi.y);widen(v.z,lo.z,hi.z);
    }
    // 00C35F66-00C35FED: extent = max - min, center = min + 0.5 * extent (the
    // 0.5 product, through DC C9 at 00C35FC5, is exact).
    V extent=sub(hi,lo);
    V center=add(mul(extent,0.5f),lo);
    // 00C35FF1-00C3600F: tiny > extent on any axis, or fewer than three points.
    if(tiny>extent.x || tiny>extent.y || tiny>extent.z || count<3) {
        cleanup_box_00c35e80(out,center,extent,true);
        *count_out=8;                                   // 00C3663A
        return true;                                    // AL = 1 on every path
    }
    // 00C36015-00C3606A: with a scale output, store the extent there and work in
    // the unit box: 1 / extent (FDIVRP / FDIVP) and the center scaled by it.
    V inverse{};
    if(scale_out) {
        *scale_out=extent;
        inverse={fdiv(1.0f,extent.x),fdiv(1.0f,extent.y),fdiv(1.0f,extent.z)};
        center={fmul(inverse.x,center.x),fmul(inverse.y,center.y),fmul(inverse.z,center.z)};
    }
    // 00C36072-00C36260: per source point, scaled when there is a scale output.
    // The first kept point within epsilon on all three axes (|kept - v| < epsilon,
    // FCOMI / JBE) is replaced when v lies farther from the center; otherwise v
    // is appended and the count published at once (00C36259).
    for(std::uint32_t i=0;i<count;++i) {
        V v;std::memcpy(&v,bytes+static_cast<std::size_t>(i)*stride,sizeof v);
        if(scale_out) v={fmul(v.x,inverse.x),fmul(v.y,inverse.y),fmul(v.z,inverse.z)};
        const std::uint32_t kept=*count_out;
        std::uint32_t j=0;
        for(;j<kept;++j) {
            const V old=out[j];
            if(epsilon>std::fabs(fsub(old.x,v.x)) && epsilon>std::fabs(fsub(old.y,v.y)) &&
               epsilon>std::fabs(fsub(old.z,v.z))) break;
        }
        if(j==kept) {
            out[kept]=v;
            *count_out=kept+1;
        } else {
            // 00C36187-00C3622C: (dx^2 + dy^2) + dz^2 of each, rounded to float.
            const V moved=sub(v,center),held=sub(out[j],center);
            if(dot(moved,moved)>dot(held,held)) out[j]=v;
        }
    }
    // 00C3626A-00C36351: the bounds again over the kept points; a full box or
    // three or more points returns as is.
    lo={FLT_MAX,FLT_MAX,FLT_MAX};hi={-FLT_MAX,-FLT_MAX,-FLT_MAX};
    for(std::uint32_t j=0;j<*count_out;++j) {
        widen(out[j].x,lo.x,hi.x);widen(out[j].y,lo.y,hi.y);widen(out[j].z,lo.z,hi.z);
    }
    extent=sub(hi,lo);
    if(!(tiny>extent.x || tiny>extent.y || tiny>extent.z || *count_out<3)) return true;
    // 00C36368-00C36509: the box around the kept points, the >= tiny smallest axis.
    cleanup_box_00c35e80(out,add(mul(extent,0.5f),lo),extent,false);
    *count_out=8;                                       // 00C364FD
    return true;
}

// 00C5DAE0 DYN_ConvexHull_GenerateTriangles, whole body (00C5DAE0..00C5DDCD).
// ABI: EAX = output record, [ESP+4] = descriptor, RET 4; EAX result is the
// frame word [ESP+20h]: 1 until the output is filled at 00C5DC1A, then 0.
// Steps: 00C35E80 cleanup (count, points +08h, stride +0Ch, epsilon +10h, the
// scale output, EDX = max(count,8)*12 scratch), x87 rescale 00C5DB81..00C5DB9F
// with one FSTP float per product, 00C5D900 (ECX count, EDX points, limit
// +18h), 00C32510 compaction (ESI = &count). Flags byte bit 0 selects the flat
// list (output byte 0 = 0) over the {3,a,b,c} polygon list (byte 0 = 1,
// 00C5DCEF); bit 1 reverses each triangle's winding. Descriptor +14h/+1Ch are
// not read. EH: none in this frame; the C++ failure exits leak nothing (the
// native 00C5D900-failure exit frees only the scratch buffer).
// Uncertainty: finite point domain only; operator new failure is not modelled.
std::uint32_t avoid_zone_dyn_hull_generate_00c5dae0(const AvoidZoneDynHullDescriptor& descriptor,
                                                    AvoidZoneDynHullGenerated& output) {
    const std::uint32_t count=descriptor.count;
    if(!count) return 1;                                         // 00C5DB3B
    if(!descriptor.points) throw std::invalid_argument("Dyn hull points are null");
    const auto* base=static_cast<const unsigned char*>(descriptor.points);
    for(std::uint32_t i=0;i<count;++i) {
        V v;std::memcpy(&v,base+static_cast<std::size_t>(i)*descriptor.stride,sizeof v);
        if(!std::isfinite(v.x) || !std::isfinite(v.y) || !std::isfinite(v.z))
            throw std::domain_error("Dyn hull exceptional coordinate domain unresolved");
    }
    // 00C5DB06..00C5DB1A: the scratch is max(count, 8) points.
    std::vector<V> cleaned(count>8?count:8);
    std::uint32_t kept=0;V scale;
    dyn_hull_cleanup_points_00c35e80(cleaned.data(),count,descriptor.points,descriptor.stride,&kept,
                                     descriptor.epsilon,&scale);   // 00C5DB5B, AL is always 1
    cleaned.resize(kept);
    for(V& p:cleaned) p={fmul(p.x,scale.x),fmul(p.y,scale.y),fmul(p.z,scale.z)};
    Triangles triangles;
    if(!hull_00c5cae0(cleaned,triangles,static_cast<int>(descriptor.limit))) return 1;   // 00C5DBBE
    // 00C5D900 preserves live triangle-array order, then 00C32510 compacts in
    // index encounter order, storing index+1 in the zero-initialized map.
    std::vector<V> points;std::vector<std::uint32_t> flat;
    std::vector<std::uint32_t> remap(cleaned.size(),0);
    for(const auto& t:triangles) if(t) for(int v:t->vertex) {
        if(!remap[v]) { points.push_back(cleaned[v]);remap[v]=static_cast<std::uint32_t>(points.size()); }
        flat.push_back(remap[v]-1);
    }
    const auto triangle_count=static_cast<std::uint32_t>(flat.size()/3);
    const bool reverse=(descriptor.flags&2)!=0;
    AvoidZoneDynHullGenerated result;
    result.vertex_count=static_cast<std::uint32_t>(points.size());   // 00C5DC2A
    result.vertices=std::move(points);
    result.triangle_count=triangle_count;
    if(descriptor.flags&1) {
        result.polygon_list=false;                               // 00C5DC3A
        result.index_count=triangle_count*3;
        result.indices.reserve(result.index_count);
        for(std::uint32_t i=0;i<triangle_count;++i) for(int k=0;k<3;++k)
            result.indices.push_back(flat[i*3+(reverse?2-k:k)]);   // memcpy or the 00C5DCA6 loop
    } else {
        result.polygon_list=true;                                // 00C5DCEF
        result.index_count=triangle_count*3+triangle_count;
        result.indices.reserve(result.index_count);
        for(std::uint32_t i=0;i<triangle_count;++i) {
            result.indices.push_back(3);
            for(int k=0;k<3;++k) result.indices.push_back(flat[i*3+(reverse?2-k:k)]);
        }
    }
    output=std::move(result);return 0;
}

bool avoid_zone_dyn_hull_triangles_00c5dae0(const V* points,std::uint32_t count,
                                          AvoidZoneDynHullTriangles& output) {
    // 00C5DDD0's descriptor: flags 1, stride 0Ch, epsilon [00D7A23C] = 0.001f,
    // +14h [00D7A238] = 0.01f, limits 1000h.
    AvoidZoneDynHullDescriptor descriptor;
    descriptor.flags=1;descriptor.count=count;descriptor.points=points;
    descriptor.stride=sizeof(V);descriptor.epsilon=0.001f;descriptor.field_14=0.01f;
    descriptor.limit=0x1000;descriptor.field_1c=0x1000;
    AvoidZoneDynHullGenerated generated;
    if(avoid_zone_dyn_hull_generate_00c5dae0(descriptor,generated)) return false;
    output.points=std::move(generated.vertices);
    output.indices=std::move(generated.indices);
    return true;
}
void avoid_zone_dyn_hull_data_00c389c0(AvoidZoneDynHullData& data,
    const AvoidZoneDynHullTriangles& hull,const AvoidZoneDynHullMemory& m) {
    if(hull.indices.size()%3) throw std::invalid_argument("Dyn hull triangle index count");
    const auto count=static_cast<std::uint32_t>(hull.points.size());
    reserve_vertices(data,count,m);data.vertex_count=count;
    bounds(hull.points,data.minimum,data.maximum);
    for(std::uint32_t i=0;i<count;++i) data.vertices[i].point=hull.points[i];
    std::vector<std::vector<std::uint16_t>> neighbors(count);
    const auto add_neighbor=[&](std::uint32_t a,std::uint32_t b) {
        if(a>=count || b>=count) throw std::out_of_range("Dyn hull triangle vertex");
        auto& list=neighbors[a];const auto value=static_cast<std::uint16_t>(b);
        if(std::find(list.begin(),list.end(),value)==list.end()) list.push_back(value);
    };
    for(std::size_t i=0;i<hull.indices.size();i+=3) {
        const auto a=hull.indices[i],b=hull.indices[i+1],c=hull.indices[i+2];
        add_neighbor(a,b);add_neighbor(b,a);add_neighbor(a,c);
        add_neighbor(c,a);add_neighbor(b,c);add_neighbor(c,b);
    }
    const auto required=count+static_cast<std::uint32_t>(hull.indices.size());
    reserve_adjacency(data,required,m);
    if(required>data.adjacency_count)
        std::fill(data.adjacency+data.adjacency_count,data.adjacency+required,std::uint16_t{});
    data.adjacency_count=required;
    std::uint32_t offset=0;
    for(std::uint32_t i=0;i<count;++i) {
        data.vertices[i].adjacency_offset=static_cast<std::uint16_t>(offset);
        data.adjacency[offset++]=static_cast<std::uint16_t>(neighbors[i].size());
        for(auto neighbor:neighbors[i]) data.adjacency[offset++]=neighbor;
    }
    // The declared adjacency allocation can exceed the written prefix;
    // native growth initialization above leaves this tail zero on first build.
    for(int x=-1;x<=1;++x) for(int y=-1;y<=1;++y) for(int z=-1;z<=1;++z) {
        if(!x && !y && !z) continue;
        const V direction{static_cast<float>(x),static_cast<float>(y),static_cast<float>(z)};
        int best=-1;float score=-FLT_MAX;
        for(std::uint32_t i=0;i<count;++i) {
            const float candidate=dot(data.vertices[i].point,direction);
            if(score<candidate) {score=candidate;best=static_cast<int>(i);}
        }
        data.support_seed[(x+1)*9+(y+1)*3+z+1]=static_cast<std::uint16_t>(best);
    }
}
void avoid_zone_dyn_hull_copy_data_004039d0(AvoidZoneDynHullData& destination,
    const AvoidZoneDynHullData& source,const AvoidZoneDynHullMemory& m) {
    destination.vertex_count=0;reserve_vertices(destination,source.vertex_count,m);
    destination.vertex_count=source.vertex_count;
    if(source.vertex_count) std::memcpy(destination.vertices,source.vertices,source.vertex_count*sizeof(*source.vertices));
    destination.adjacency_count=0;reserve_adjacency(destination,source.adjacency_count,m);
    destination.adjacency_count=source.adjacency_count;
    if(source.adjacency_count) std::memcpy(destination.adjacency,source.adjacency,source.adjacency_count*sizeof(*source.adjacency));
    destination.minimum=source.minimum;destination.maximum=source.maximum;
    std::memcpy(destination.support_seed,source.support_seed,sizeof(source.support_seed));
}
void avoid_zone_dyn_hull_destroy_00c37450(AvoidZoneDynHullHandle& handle,const AvoidZoneDynHullMemory& m) {
    // Native captures [handle] once in ESI. A release can republish handle.data
    // (or end the handle's lifetime); later frees still use the captured data.
    auto* const data=handle.data;
    if(!data) return;
    release(m,data->adjacency);
    // Keep this field read after the first release, as at 00C37467.
    release(m,data->vertices);
    release(m,data);
}
void avoid_zone_dyn_hull_replace_00c5deb0(AvoidZoneDynHullHandle& handle,
    const V* points,std::uint32_t count,const AvoidZoneDynHullMemory& m) {
    avoid_zone_dyn_hull_destroy_00c37450(handle,m);
    auto* data=new_data(m);
    AvoidZoneDynHullTriangles triangles;
    // 00C5DDD0 ignores the generator status and consumes the initially empty
    // result on failure, yielding a real internal record with zero vertices.
    avoid_zone_dyn_hull_triangles_00c5dae0(points,count,triangles);
    avoid_zone_dyn_hull_data_00c389c0(*data,triangles,m);
    handle.data=data;
}
void avoid_zone_dyn_hull_construct_00c5df30(AvoidZoneDynHullHandle& handle,
    const V* points,std::uint32_t count,const AvoidZoneDynHullMemory& m) {
    handle.data=nullptr;handle.field_04=0;
    avoid_zone_dyn_hull_replace_00c5deb0(handle,points,count,m);
}
void avoid_zone_dyn_hull_copy_00c40f50(AvoidZoneDynHullHandle& destination,
    const AvoidZoneDynHullHandle& source,const AvoidZoneDynHullMemory& m) {
    AvoidZoneDynHullData* data=nullptr;
    if(source.data) {data=new_data(m);avoid_zone_dyn_hull_copy_data_004039d0(*data,*source.data,m);}
    destination.data=data;destination.field_04=source.field_04;
}
std::uint32_t avoid_zone_dyn_hull_vertex_count_00c32d20(const AvoidZoneDynHullHandle& h) noexcept {
    return h.data->vertex_count;
}
} // namespace bsp

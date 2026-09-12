#include "bsp/avoid_zone_polygon_partition.hpp"
#include "bsp/camera_position_modes.hpp"
#include <limits>
#include <stdexcept>

namespace bsp {
namespace {
const double partition_length_cutoff = 1e-10; //00CE3820=3DDB7CDFD9D7BDBB
const double partition_short_length = 1e-5; //00CE3C70=3EE4F8B588E368F1

//004F2F40: retained binary32 stores, x87 comparison (unordered takes the
//short-length branch), and actual CRT sqrt. EDX adds the required CRT binding.
__declspec(naked) void __fastcall normalize_partition_vector(
    float*, const CameraAxesCrtAccess*) {
    __asm {
        push ebx
        mov ebx, edx
        sub esp, 0ch
        push esi
        mov esi, ecx
        fld dword ptr [esi+4]
        fstp dword ptr [esp+8]
        fld dword ptr [esp+8]
        fld dword ptr [esi]
        fstp dword ptr [esp+4]
        fld dword ptr [esp+4]
        fmul st(0), st(0)
        fld st(1)
        fmulp st(2), st(0)
        faddp st(1), st(0)
        fstp dword ptr [esp+0ch]
        fld qword ptr [partition_length_cutoff]
        fld dword ptr [esp+0ch]
        fcomi st(0), st(1)
        fstp st(1)
        jbe short_vector
        mov ecx, ebx
        call native_crt_sqrt_st0_00bf7030
        fstp dword ptr [esp+0ch]
        fld dword ptr [esp+0ch]
        jmp have_length
    short_vector:
        fstp st(0)
        fld qword ptr [partition_short_length]
    have_length:
        fstp dword ptr [esp+0ch]
        fld dword ptr [esp+4]
        fld dword ptr [esp+0ch]
        fld st(0)
        fdivp st(2), st(0)
        fxch st(1)
        fstp dword ptr [esi]
        fdivr dword ptr [esp+8]
        fstp dword ptr [esi+4]
        pop esi
        add esp, 0ch
        pop ebx
        ret
    }
}

float subtract(float a, float b) noexcept {
    float result;
    __asm {
        fld a
        fsub b
        fstp result
    }
    return result;
}

//004F4C60 (and the inlined schedule at004F4FC8..004F50CD). Subtractions,
//normalizations and cross each spill exactly once before the existing asin.
float partition_angle(const AvoidZoneDraftPoint& origin,
    const AvoidZoneDraftPoint& first, const AvoidZoneDraftPoint& second,
    const CameraAxesCrtAccess& crt) {
    AvoidZoneDraftPoint a{subtract(first[0], origin[0]), subtract(first[1], origin[1])};
    normalize_partition_vector(a.data(), &crt);
    AvoidZoneDraftPoint b{subtract(second[0], origin[0]), subtract(second[1], origin[1])};
    normalize_partition_vector(b.data(), &crt);
    const float first_x=a[0], first_y=a[1], second_x=b[0], second_y=b[1];
    float cross;
    __asm {
        fld second_y
        fmul first_x
        fld first_y
        fmul second_x
        fsubp st(1), st(0)
        fstp cross
    }
    return camera_asin_clamped_0042cf10(cross);
}

//004F3EF0 retains its native register outputs and RET24h. Keeping the exact
//x87 lifetime matters: several dot-product operands remain on stack across
//binary32 stores, and the final numerators divide without an extra spill.
__declspec(naked) void __fastcall partition_barycentric(
    float*, float*, float, float, float, float, float, float, float, float, float*) {
    __asm {
        sub esp, 10h
        fld dword ptr [esp+1ch]
        fld dword ptr [esp+14h]
        fld st(0)
        fsubp st(2), st(0)
        fxch st(1)
        fstp dword ptr [esp]
        fld dword ptr [esp+20h]
        fld dword ptr [esp+18h]
        fld st(0)
        fsubp st(2), st(0)
        fxch st(1)
        fstp dword ptr [esp+4]
        fld dword ptr [esp+24h]
        fsub st(0), st(2)
        fstp dword ptr [esp+1ch]
        fld dword ptr [esp+28h]
        fsub st(0), st(1)
        fstp dword ptr [esp+20h]
        fld dword ptr [esp+2ch]
        fsubrp st(2), st(0)
        fxch st(1)
        fstp dword ptr [esp+8]
        fsubr dword ptr [esp+30h]
        fstp dword ptr [esp+0ch]
        fld dword ptr [esp]
        fld st(0)
        fld dword ptr [esp+4]
        fld st(0)
        fmul st(0), st(0)
        fld st(2)
        fmulp st(3), st(0)
        faddp st(2), st(0)
        fxch st(1)
        fstp dword ptr [esp+24h]
        fld dword ptr [esp+20h]
        fld st(0)
        fmul st(0), st(2)
        fld dword ptr [esp+1ch]
        fld st(0)
        fmul st(0), st(5)
        faddp st(2), st(0)
        fxch st(1)
        fstp dword ptr [esp+1ch]
        fld st(1)
        fld st(1)
        fmul st(0), st(0)
        fld st(1)
        fmulp st(2), st(0)
        faddp st(1), st(0)
        fstp dword ptr [esp+14h]
        fld dword ptr [esp+8]
        fld st(0)
        fmulp st(5), st(0)
        fld dword ptr [esp+0ch]
        fld st(0)
        fmulp st(5), st(0)
        fxch st(5)
        faddp st(4), st(0)
        fxch st(3)
        fstp dword ptr [esp+2ch]
        fxch st(3)
        fmulp st(1), st(0)
        fxch st(1)
        fmulp st(2), st(0)
        faddp st(1), st(0)
        fstp dword ptr [esp]
        fld dword ptr [esp+1ch]
        fld st(0)
        fld dword ptr [esp+14h]
        fld st(0)
        fld dword ptr [esp+24h]
        fld st(0)
        fmulp st(2), st(0)
        fld st(3)
        fmulp st(4), st(0)
        fxch st(1)
        fsubrp st(3), st(0)
        fxch st(2)
        fstp dword ptr [esp+1ch]
        fld dword ptr [esp+2ch]
        fld st(0)
        fmulp st(2), st(0)
        mov eax, dword ptr [esp+34h]
        fld dword ptr [esp]
        fld st(0)
        fmul st(0), st(5)
        fsubp st(3), st(0)
        fld dword ptr [esp+1ch]
        fld st(0)
        fdivp st(4), st(0)
        fxch st(3)
        fstp dword ptr [edx]
        fmulp st(3), st(0)
        fmulp st(3), st(0)
        fxch st(1)
        fsubrp st(2), st(0)
        fdivp st(1), st(0)
        fstp dword ptr [esp+1ch]
        fld dword ptr [esp+1ch]
        fst dword ptr [eax]
        fld dword ptr [edx]
        fld1
        fsubrp st(1), st(0)
        fsubrp st(1), st(0)
        fstp dword ptr [ecx]
        add esp, 10h
        ret 24h
    }
}

bool in_triangle(float u, float v) noexcept {
    //004F4E50/5B reject unordered or negative;004F4E60..6E compares the
    //unspilled x87 sum against one, inclusive at the triangle boundary.
    if (!(u >= 0.0f) || !(v >= 0.0f)) return false;
    unsigned char inside;
    __asm {
        fld u
        fadd v
        fld1
        fcomip st(0), st(1)
        fstp st(0)
        setae inside
    }
    return inside != 0;
}

class PartitionState {
public:
    const std::vector<AvoidZoneDraftPoint>& points;
    const CameraAxesCrtAccess& crt;
    const float tolerance;
    std::vector<std::uint32_t> next, previous, piece;
    std::uint32_t current=0, first=0, last=0;
    AvoidZonePolygonPartition result;

    PartitionState(const std::vector<AvoidZoneDraftPoint>& input, float tol,
        const CameraAxesCrtAccess& access): points(input), crt(access), tolerance(tol),
        next(input.size()), previous(input.size()) {
        //004F65F0 produces the index layout before the constructor's count<3.
        result.remaining_count=static_cast<std::int32_t>(points.size());
        for (std::uint32_t i=0; i<points.size(); ++i) {
            next[i]=i+1; previous[i]=i-1;
        }
        previous[0]=static_cast<std::uint32_t>(points.size()-1);
        next.back()=0;
    }

    bool triangle_empty(std::uint32_t begin, std::uint32_t end,
        std::uint32_t a, std::uint32_t b, std::uint32_t c) const {
        //004F4CD0 walks [begin,end) through the active next links. It rejects
        //points on triangle edges too; no epsilon/determinant fallback exists.
        for (auto i=begin; i!=end; i=next[i]) {
            const auto& pa=points[a]; const auto& pb=points[b];
            const auto& pc=points[c]; const auto& p=points[i];
            float remaining, u, v;
            partition_barycentric(&remaining,&u,pa[0],pa[1],pb[0],pb[1],
                pc[0],pc[1],p[0],p[1],&v);
            if (in_triangle(u,v)) return false;
        }
        return true;
    }

    bool select_seed() {
        //004F4EC0 scans exactly one remaining cycle; strict > retains the
        //first equal-score seed.00D7A244 is negative FLT_MAX, not zero.
        const auto start=current;
        std::uint32_t selected=std::numeric_limits<std::uint32_t>::max();
        float best=-std::numeric_limits<float>::max();
        do {
            const auto n=next[current], p=previous[current];
            const float score=partition_angle(points[p],points[current],points[n],crt);
            if (score>best && triangle_empty(next[n],p,p,current,n)) {
                selected=current; best=score;
            }
            current=next[current];
        } while (current!=start);
        current=selected;
        if (selected==std::numeric_limits<std::uint32_t>::max()) return false;
        //004F6560 seeds the current vector in previous/current/next order.
        first=previous[current]; last=next[current];
        piece.push_back(first); piece.push_back(current); piece.push_back(last);
        return true;
    }

    bool extend_previous() const {
        //004F5500 tests both new endpoint angles then outside-chain points.
        const auto p=previous[first];
        const float a=partition_angle(points[p],points[first],points[next[first]],crt);
        if (-tolerance>a) return false; //native JA: unordered does not reject
        const float b=partition_angle(points[previous[last]],points[last],points[p],crt);
        if (-tolerance>b) return false;
        return triangle_empty(next[last],p,p,first,last);
    }

    bool extend_next() const {
        //004F5790 is attempted only after004F5500 rejects.
        const auto n=next[last];
        const float a=partition_angle(points[previous[last]],points[last],points[n],crt);
        if (-tolerance>a) return false;
        const float b=partition_angle(points[n],points[first],points[next[first]],crt);
        if (-tolerance>b) return false;
        return triangle_empty(next[n],first,first,last,n);
    }

    AvoidZonePolygonPartition run() {
        while (result.remaining_count>=3) {
            if (!select_seed()) break;
            auto additional=result.remaining_count-3;
            while (additional!=0) {
                if (extend_previous()) {
                    first=previous[first]; piece.insert(piece.begin(),first);
                } else if (extend_next()) {
                    last=next[last]; piece.push_back(last);
                } else {
                    //004F7077 writes {native+40h,native+3Ch} in that order.
                    result.cuts.push_back({first,last});
                    break;
                }
                --additional;
            }
            next[first]=last; previous[last]=first;
            //004F6E90 deep-copies the native10h (16-byte) vector element:
            //allocator word + begin/end/capacity, not16 float/index entries.
            result.pieces.push_back(piece);
            result.remaining_count+=2-static_cast<std::int32_t>(piece.size());
            piece.clear();
            current=last;
        }
        return std::move(result);
    }
};
} //namespace

AvoidZonePolygonPartition avoid_zone_polygon_partition_004f6f20(
    const std::vector<AvoidZoneDraftPoint>& points, float angular_tolerance,
    const CameraAxesCrtAccess& crt) {
    if (points.empty()) throw std::invalid_argument("native polygon has no index zero");
    if (points.size()>static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()/8))
        throw std::length_error("native polygon byte span exceeds signed Win32 range");
    if (!crt.dispatch_bypass_0109dd78 || !crt.except_00c27489)
        throw std::invalid_argument("polygon partition requires actual CRT access");
    return PartitionState(points,angular_tolerance,crt).run();
}
} //namespace bsp

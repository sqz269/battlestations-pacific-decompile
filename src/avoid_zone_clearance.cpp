#include "bsp/avoid_zone_clearance.hpp"
#include "bsp/avoid_zone_segment_math.hpp"
#include "bsp/geometry_helpers.hpp"
#include "bsp/random_threads.hpp"
#include "bsp/ship_ai_nav_circle_tangent.hpp"
#include <limits>
#include <new>

namespace bsp {
namespace {
const double clearance_half = 0.5; // 00D7A280
const float clearance_one = 1.0f; // 00D7A24C
const double clearance_length_cutoff = 1e-10; // 00CE3820

template<class Segment> Segment* traversal_next(Segment* segment) noexcept {
    return segment->closes_run == 0 && segment->next != nullptr
        ? segment->next : segment->next_run;
}

// Private actual-CRT adaptation of the existing00414C60 schedule. Its native
// interface has ECX=vector; this bridge adds required CRT in EDX.
__declspec(naked) float __fastcall clearance_cutoff_length(
    const float*, const CameraAxesCrtAccess*) {
    __asm {
        push ebx
        mov ebx, edx
        push ecx
        fld dword ptr [ecx + 4]
        fld dword ptr [ecx]
        fmul st(0), st(0)
        fld st(1)
        fmulp st(2), st(0)
        faddp st(1), st(0)
        fstp dword ptr [esp]
        fld qword ptr [clearance_length_cutoff]
        fld dword ptr [esp]
        fcomi st(0), st(1)
        fstp st(1)
        jbe cutoff_zero
        mov ecx, ebx
        call native_crt_sqrt_st0_00bf7030
        fstp dword ptr [esp]
        fld dword ptr [esp]
        fstp dword ptr [esp]
        fld dword ptr [esp]
        pop ecx
        pop ebx
        ret
    cutoff_zero:
        xorps xmm0, xmm0
        fstp st(0)
        movss dword ptr [esp], xmm0
        fld dword ptr [esp]
        pop ecx
        pop ebx
        ret
    }
}

struct ClearanceGate {
    TrackedCriticalSection* section;
    explicit ClearanceGate(TrackedCriticalSection* actual) : section(actual) {
        if (section) {
            EnterCriticalSection(&section->native);
            section->depth = static_cast<std::int32_t>(
                static_cast<std::uint32_t>(section->depth) + 1u);
        }
    }
    ~ClearanceGate() {
        if (section) {
            section->depth = static_cast<std::int32_t>(
                static_cast<std::uint32_t>(section->depth) - 1u);
            LeaveCriticalSection(&section->native);
        }
    }
};
struct SelectedSegmentOwner {
    const AvoidZoneAllocationAccess& allocation;
    AvoidZoneSelectedSegment* head{};
    ~SelectedSegmentOwner() {
        avoid_zone_selected_segments_clear_004158a0(head, allocation);
    }
};

std::array<float, 2> offset_point(const std::array<float, 2>& base,
    const ShipAiPathLateralRecord& record, float radius) noexcept {
    // Native separately spills both products before adding the stored base.
    const float dx = radius * record.offset_dir_x;
    const float dz = radius * record.offset_dir_z;
    return {dx + base[0], dz + base[1]};
}
} // namespace

__declspec(naked) bool __fastcall native_segment_crossing_004f3730(
    const float*, const float*, const float*, const float*, float*) noexcept {
    __asm {
        SUB ESP,0x18 // 004f3730
        FLD dword ptr [ECX] // 004f3733
        MOV EAX,dword ptr [ESP + 0x1c] // 004f3735
        FSTP dword ptr [ESP] // 004f3739
        FLD dword ptr [EDX] // 004f373c
        FSUB dword ptr [ESP] // 004f373e
        FSTP dword ptr [ESP + 0x8] // 004f3741
        FLD dword ptr [ECX + 0x4] // 004f3745
        FSTP dword ptr [ESP + 0x4] // 004f3748
        FLD dword ptr [EDX + 0x4] // 004f374c
        MOV EDX,dword ptr [ESP + 0x20] // 004f374f
        FSUB dword ptr [ESP + 0x4] // 004f3753
        FSTP dword ptr [ESP + 0xc] // 004f3757
        FLD dword ptr [EDX] // 004f375b
        FSUB dword ptr [EAX] // 004f375d
        FSTP dword ptr [ESP + 0x10] // 004f375f
        FLD dword ptr [EDX + 0x4] // 004f3763
        LEA EDX,[ESP + 0x1c] // 004f3766
        FSUB dword ptr [EAX + 0x4] // 004f376a
        PUSH EDX // 004f376d
        LEA EDX,[ESP + 0x24] // 004f376e
        PUSH EDX // 004f3772
        LEA EDX,[ESP + 0x18] // 004f3773
        FSTP dword ptr [ESP + 0x1c] // 004f3777
        PUSH EDX // 004f377b
        PUSH EAX // 004f377c
        LEA EDX,[ESP + 0x18] // 004f377d
        CALL native_segment_parameters_004f3630 // 004f3781
        TEST AL,AL // 004f3786
        JZ K_004f37fa // 004f3788
        FLDZ // 004f378a
        FLD dword ptr [ESP + 0x20] // 004f378c
        FCOMI st(0),st(1) // 004f3790
        JC K_004f37f6 // 004f3792
        MOVSS XMM0,dword ptr [clearance_one] // 004f3794
        COMISS XMM0,dword ptr [ESP + 0x20] // 004f379c
        JC K_004f37f6 // 004f37a1
        FLD dword ptr [ESP + 0x1c] // 004f37a3
        FCOMIP st(0),st(2) // 004f37a7
        FSTP st(1) // 004f37a9
        JC K_004f37f8 // 004f37ab
        COMISS XMM0,dword ptr [ESP + 0x1c] // 004f37ad
        JC K_004f37f8 // 004f37b2
        FLD dword ptr [ESP + 0x8] // 004f37b4
        MOV EAX,dword ptr [ESP + 0x24] // 004f37b8
        FMUL st(0), st(1) // 004f37bc
        FSTP dword ptr [ESP + 0x10] // 004f37be
        FMUL dword ptr [ESP + 0xc] // 004f37c2
        FSTP dword ptr [ESP + 0x14] // 004f37c6
        FLD dword ptr [ESP] // 004f37ca
        FADD dword ptr [ESP + 0x10] // 004f37cd
        FSTP dword ptr [ESP + 0x8] // 004f37d1
        FLD dword ptr [ESP + 0x4] // 004f37d5
        FADD dword ptr [ESP + 0x14] // 004f37d9
        FSTP dword ptr [ESP + 0xc] // 004f37dd
        FLD dword ptr [ESP + 0x8] // 004f37e1
        FSTP dword ptr [EAX] // 004f37e5
        FLD dword ptr [ESP + 0xc] // 004f37e7
        FSTP dword ptr [EAX + 0x4] // 004f37eb
        MOV AL,0x1 // 004f37ee
        ADD ESP,0x18 // 004f37f0
        RET 0xc // 004f37f3
    K_004f37f6:
        FSTP st(0) // 004f37f6
    K_004f37f8:
        FSTP st(0) // 004f37f8
    K_004f37fa:
        XOR AL,AL // 004f37fa
        ADD ESP,0x18 // 004f37fc
        RET 0xc // 004f37ff
    }
}

__declspec(naked) bool __fastcall native_box_meets_segment_0085c910(
    const float*, const float*, const float*, const float*) noexcept {
    __asm {
        SUB ESP,0x20 // 0085c910
        PUSH ESI // 0085c913
        MOV ESI,dword ptr [ESP + 0x2c] // 0085c914
        FLD dword ptr [ESI] // 0085c918
        PUSH EDI // 0085c91a
        MOV EDI,dword ptr [ESP + 0x2c] // 0085c91b
        FSTP dword ptr [ESP + 0x30] // 0085c91f
        FLD dword ptr [EDI] // 0085c923
        FSTP dword ptr [ESP + 0x2c] // 0085c925
        FLD dword ptr [ESP + 0x30] // 0085c929
        FLD st(0) // 0085c92d
        FLD dword ptr [ESP + 0x2c] // 0085c92f
        FLD st(0) // 0085c933
        FSUBP st(2),st(0) // 0085c935
        FLD qword ptr [clearance_half] // 0085c937
        FMUL st(2), st(0) // 0085c93d: DC CA writes ST2, not ST0
        FXCH st(2) // 0085c93f
        FSTP dword ptr [ESP + 0x8] // 0085c941
        FLD dword ptr [EDX] // 0085c945
        MOVSS XMM0,dword ptr [ESP + 0x8] // 0085c947
        FSTP dword ptr [ESP + 0x30] // 0085c94d
        FLD dword ptr [ECX] // 0085c951
        FSTP dword ptr [ESP + 0x2c] // 0085c953
        FLD dword ptr [ESP + 0x30] // 0085c957
        MOVSS dword ptr [ESP + 0x30],XMM0 // 0085c95b
        FLD st(0) // 0085c961
        MOV EAX,dword ptr [ESP + 0x30] // 0085c963
        FLD dword ptr [ESP + 0x2c] // 0085c967
        AND EAX,0x7fffffff // 0085c96b
        FLD st(0) // 0085c970
        MOV dword ptr [ESP + 0x10],EAX // 0085c972
        FSUBP st(2),st(0) // 0085c976
        FXCH  st(1) // 0085c978
        FMUL st(0), st(4) // 0085c97a
        FSTP dword ptr [ESP + 0x18] // 0085c97c
        FXCH st(2) // 0085c980
        FADDP st(4),st(0) // 0085c982
        FADDP  st(1), st(0) // 0085c984
        FSUBP st(2),st(0) // 0085c986
        FMUL st(1), st(0) // 0085c988: DC C9 writes ST1, not ST0
        FXCH  st(1) // 0085c98a
        FSTP dword ptr [ESP + 0x20] // 0085c98c
        FLD dword ptr [ESP + 0x20] // 0085c990
        FST dword ptr [ESP + 0x30] // 0085c994
        MOV EAX,dword ptr [ESP + 0x30] // 0085c998
        AND EAX,0x7fffffff // 0085c99c
        MOV dword ptr [ESP + 0x30],EAX // 0085c9a1
        FLD dword ptr [ESP + 0x30] // 0085c9a5
        FLD dword ptr [ESP + 0x10] // 0085c9a9
        FADD dword ptr [ESP + 0x18] // 0085c9ad
        FXCH  st(1) // 0085c9b1
        FCOMIP st(0),st(1) // 0085c9b3
        FSTP st(0) // 0085c9b5
        JBE K_0085c9c7 // 0085c9b7
        FSTP st(1) // 0085c9b9
        FSTP st(0) // 0085c9bb
    K_0085c9bd:
        POP EDI // 0085c9bd
        XOR AL,AL // 0085c9be
        POP ESI // 0085c9c0
        ADD ESP,0x20 // 0085c9c1
        RET 0x8 // 0085c9c4
    K_0085c9c7:
        FLD dword ptr [ESI + 0x4] // 0085c9c7
        FSTP dword ptr [ESP + 0x30] // 0085c9ca
        FLD dword ptr [EDI + 0x4] // 0085c9ce
        FSTP dword ptr [ESP + 0x2c] // 0085c9d1
        FLD dword ptr [ESP + 0x30] // 0085c9d5
        FLD st(0) // 0085c9d9
        FLD dword ptr [ESP + 0x2c] // 0085c9db
        FLD st(0) // 0085c9df
        FSUBP st(2),st(0) // 0085c9e1
        FXCH  st(1) // 0085c9e3
        FMUL st(0), st(4) // 0085c9e5
        FSTP dword ptr [ESP + 0xc] // 0085c9e7
        FLD dword ptr [EDX + 0x4] // 0085c9eb
        MOVSS XMM0,dword ptr [ESP + 0xc] // 0085c9ee
        FSTP dword ptr [ESP + 0x30] // 0085c9f4
        FLD dword ptr [ECX + 0x4] // 0085c9f8
        FSTP dword ptr [ESP + 0x2c] // 0085c9fb
        FLD dword ptr [ESP + 0x30] // 0085c9ff
        MOVSS dword ptr [ESP + 0x30],XMM0 // 0085ca03
        FLD st(0) // 0085ca09
        MOV ECX,dword ptr [ESP + 0x30] // 0085ca0b
        FLD dword ptr [ESP + 0x2c] // 0085ca0f
        AND ECX,0x7fffffff // 0085ca13
        FLD st(0) // 0085ca19
        MOV dword ptr [ESP + 0x14],ECX // 0085ca1b
        FSUBP st(2),st(0) // 0085ca1f
        FXCH  st(1) // 0085ca21
        FMUL st(0), st(6) // 0085ca23
        FSTP dword ptr [ESP + 0x1c] // 0085ca25
        FXCH st(2) // 0085ca29
        FADDP st(3),st(0) // 0085ca2b
        FADDP  st(1), st(0) // 0085ca2d
        FSUBP  st(1), st(0) // 0085ca2f
        FMULP st(2) , st(0) // 0085ca31
        FXCH  st(1) // 0085ca33
        FSTP dword ptr [ESP + 0x24] // 0085ca35
        FLD dword ptr [ESP + 0x24] // 0085ca39
        FST dword ptr [ESP + 0x30] // 0085ca3d
        MOV EDX,dword ptr [ESP + 0x30] // 0085ca41
        AND EDX,0x7fffffff // 0085ca45
        MOV dword ptr [ESP + 0x30],EDX // 0085ca4b
        FLD dword ptr [ESP + 0x30] // 0085ca4f
        FLD dword ptr [ESP + 0x14] // 0085ca53
        FLD st(0) // 0085ca57
        FLD dword ptr [ESP + 0x1c] // 0085ca59
        FLD st(0) // 0085ca5d
        FADDP st(2),st(0) // 0085ca5f
        FXCH st(3) // 0085ca61
        FCOMIP st(0),st(1) // 0085ca63
        FSTP st(0) // 0085ca65
        JBE K_0085ca7b // 0085ca67
        FSTP st(2) // 0085ca69
        POP EDI // 0085ca6b
        FSTP st(1) // 0085ca6c
        XOR AL,AL // 0085ca6e
        FSTP st(0) // 0085ca70
        POP ESI // 0085ca72
        FSTP st(0) // 0085ca73
        ADD ESP,0x20 // 0085ca75
        RET 0x8 // 0085ca78
    K_0085ca7b:
        FLD dword ptr [ESP + 0x8] // 0085ca7b
        FMULP st(3) , st(0) // 0085ca7f
        FLD dword ptr [ESP + 0xc] // 0085ca81
        FMULP st(4) , st(0) // 0085ca85
        FXCH st(2) // 0085ca87
        FSUBRP st(3),st(0) // 0085ca89
        FXCH st(2) // 0085ca8b
        FSTP dword ptr [ESP + 0x30] // 0085ca8d
        MOV EAX,dword ptr [ESP + 0x30] // 0085ca91
        AND EAX,0x7fffffff // 0085ca95
        MOV dword ptr [ESP + 0x30],EAX // 0085ca9a
        FLD dword ptr [ESP + 0x30] // 0085ca9e
        FLD dword ptr [ESP + 0x10] // 0085caa2
        FMULP st(3) , st(0) // 0085caa6
        FLD dword ptr [ESP + 0x18] // 0085caa8
        FMULP st(2) , st(0) // 0085caac
        FXCH st(2) // 0085caae
        FADDP  st(1), st(0) // 0085cab0
        FXCH  st(1) // 0085cab2
        FCOMIP st(0),st(1) // 0085cab4
        FSTP st(0) // 0085cab6
        JA K_0085c9bd // 0085cab8
        POP EDI // 0085cabe
        MOV AL,0x1 // 0085cabf
        POP ESI // 0085cac1
        ADD ESP,0x20 // 0085cac2
        RET 0x8 // 0085cac5
    }
}

AvoidZoneSelectedSegment& avoid_zone_segment_initialize_00415190(
    AvoidZoneSelectedSegment& segment, const std::array<float, 2>& start,
    const std::array<float, 2>& end) noexcept {
    segment.start[0] = start[0];
    segment.start[1] = start[1];
    segment.end[0] = end[0];
    segment.end[1] = end[1];
    segment.previous = nullptr;
    segment.next = nullptr;
    segment.next_run = nullptr;
    segment.closes_run = 0;
    return segment;
}

AvoidZoneSelectedSegment* avoid_zone_select_segments_00417630(
    const AvoidZoneNativeStorage& zone, const std::array<float, 4>& bounds,
    const AvoidZoneAllocationAccess& allocation) {
    // JA rejects ordered separation only; unordered comparisons proceed.
    if (zone.min_x > bounds[2] || bounds[0] > zone.max_x ||
        zone.min_z > bounds[3] || bounds[1] > zone.max_z) return nullptr;
    const auto* previous = zone.corners.records[zone.corners.count - 1];
    std::array<float, 2> start{previous->x, previous->z};
    AvoidZoneSelectedSegment* first = nullptr;
    AvoidZoneSelectedSegment* current_run = nullptr;
    AvoidZoneSelectedSegment* previous_run = nullptr;
    AvoidZoneSelectedSegment* tail = nullptr;
    bool selected_last = false;
    for (std::int32_t index = 0; index != zone.corners.count; ++index) {
        const auto* corner = zone.corners.records[index];
        const std::array<float, 2> end{corner->x, corner->z};
        // FCOMIP+JC selects the swap branch for less OR unordered.
        const bool swap_x = !(end[0] >= start[0]);
        const bool swap_z = !(end[1] >= start[1]);
        const float minimum_x = swap_x ? end[0] : start[0];
        const float maximum_x = swap_x ? start[0] : end[0];
        const float minimum_z = swap_z ? end[1] : start[1];
        const float maximum_z = swap_z ? start[1] : end[1];
        selected_last = maximum_x > bounds[0] && bounds[2] > minimum_x &&
            maximum_z > bounds[1] && bounds[3] > minimum_z &&
            native_box_meets_segment_0085c910(bounds.data(), bounds.data() + 2,
                start.data(), end.data());
        if (selected_last) {
            auto* storage = allocation.allocate_record_00bf681b(allocation.context, 0x20);
            auto* segment = ::new (storage) AvoidZoneSelectedSegment;
            avoid_zone_segment_initialize_00415190(*segment, start, end);
            if (!current_run) {
                tail = current_run = segment;
                for (auto* link = previous_run; link; link = link->next)
                    link->next_run = segment;
            } else {
                tail->next = segment;
                segment->previous = tail;
                tail = segment;
            }
            if (!first) first = segment;
        } else if (current_run) {
            previous_run = current_run;
            tail = current_run = nullptr;
        }
        start = end;
    }
    if (selected_last && first) {
        first->previous = tail;
        tail->next = first;
        if (first == current_run) tail->closes_run = 1;
        else first = first->next_run;
        for (auto* link = current_run; link;) {
            link->next_run = nullptr;
            if (link->closes_run) break;
            link = link->next;
        }
    }
    return first;
}

AvoidZoneSelectedSegment* avoid_zone_group_select_segments_00417a40(
    const AvoidZoneClearanceGroupView& group, const std::array<float, 4>& bounds,
    const AvoidZoneAllocationAccess& allocation) {
    AvoidZoneSelectedSegment* first = nullptr;
    AvoidZoneSelectedSegment* tail_run = nullptr;
    for (std::uint32_t index = 0; index != group.count; ++index) {
        auto* selected = avoid_zone_select_segments_00417630(*group.zones[index],
            bounds, allocation);
        if (!selected) continue;
        if (tail_run) {
            tail_run->next_run = selected;
            while (tail_run->next && !tail_run->closes_run) {
                tail_run = tail_run->next;
                tail_run->next_run = selected;
            }
        } else first = tail_run = selected;
        while (tail_run->next_run) tail_run = tail_run->next_run;
    }
    return first;
}

void avoid_zone_selected_segments_clear_004158a0(AvoidZoneSelectedSegment*& head,
    const AvoidZoneAllocationAccess& allocation) noexcept {
    while (head) {
        auto* next = traversal_next(head);
        allocation.free_record_00bf65ac(allocation.context, head);
        // Recovered004158C8: ADD ESP,4; TEST EDI,EDI; MOV [ESI],EDI;
        // JNZ004158B0; POP EDI. This is not a one-element destructor.
        head = next;
    }
}

AvoidZoneSelectedSegment* avoid_zone_selected_segments_hit_004158e0(
    AvoidZoneSelectedSegment* head, const std::array<float, 2>& start,
    const std::array<float, 2>& end, std::array<float, 2>& output) noexcept {
    if (!head) return nullptr;
    output[0] = end[0];
    output[1] = end[1];
    AvoidZoneSelectedSegment* hit = nullptr;
    for (auto* segment = head; segment; segment = traversal_next(segment)) {
        std::array<float, 2> intersection;
        if (native_segment_crossing_004f3730(start.data(), output.data(),
            segment->start.data(), segment->end.data(), intersection.data())) {
            output = intersection;
            hit = segment;
        }
    }
    return hit;
}

float avoid_zone_selected_segments_distance_00419fc0(
    const AvoidZoneSelectedSegment* head, const std::array<float, 2>& query,
    const CameraAxesCrtAccess& crt) {
    float result = std::numeric_limits<float>::max(); // 00D7A248=7F7FFFFF
    for (auto* segment = head; segment; segment = traversal_next(segment)) {
        const float distance = avoid_zone_segment_distance_00419ab0(
            segment->start, segment->end, query, crt);
        if (distance < result) result = distance;
    }
    return result;
}

void avoid_zone_ensure_corner_clearance_00423190(const AvoidZoneNativeStorage& zone,
    ShipAiPathLateralRecord& record, AvoidZoneClearanceAccess& access,
    const AvoidZoneAllocationAccess& allocation, const CameraAxesCrtAccess& crt) {
    if (record.clearance_scale > 0.0f) return; // COMISS+JA; NaN recomputes.
    ClearanceGate gate(access.manager_critical_section_004218e0());
    const std::array<float, 2> base{
        record.offset_dir_x + record.x, record.offset_dir_z + record.z};
    float radius = 800.0f;
    auto candidate = offset_point(base, record, radius);
    const std::array<float, 2> maximum{candidate[0] + 800.0f, candidate[1] + 800.0f};
    std::array<float, 4> bounds{candidate[0] - 800.0f, candidate[1] - 800.0f,
        candidate[0] - 800.0f, candidate[1] - 800.0f};
    include_point_00415010(bounds, maximum.data());
    SelectedSegmentOwner selected{allocation};
    // Native initializes layer to zero, then changes it. This clear is called
    // for a nonzero layer even though the freshly constructed head is empty.
    if (zone.layer != 0) avoid_zone_selected_segments_clear_004158a0(selected.head, allocation);
    if (selected.head) avoid_zone_selected_segments_clear_004158a0(selected.head, allocation);
    const auto group = access.selected_group_004120d0(zone.layer);
    selected.head = avoid_zone_group_select_segments_00417a40(group, bounds, allocation);
    const auto far_point = offset_point(base, record, 1600.0f);
    std::array<float, 2> hit;
    if (avoid_zone_selected_segments_hit_004158e0(selected.head, base, far_point, hit)) {
        const std::array<float, 2> delta{base[0] - hit[0], base[1] - hit[1]};
        radius = clearance_cutoff_length(delta.data(), &crt) * 0.5f;
        candidate = offset_point(base, record, radius);
    }
    float distance = avoid_zone_selected_segments_distance_00419fc0(selected.head, candidate, crt);
    // Native holds radius-1 in x87; a double holds this float subtraction
    // exactly over this routine's radius range, without an extra float spill.
    while (static_cast<double>(distance) < static_cast<double>(radius) - 1.0) {
        const float subtract_fifty = radius - 50.0f;
        const float midpoint = static_cast<float>((static_cast<double>(radius) + distance) * 0.5);
        radius = midpoint < subtract_fifty ? midpoint : subtract_fifty;
        if (radius < 25.0f) { radius = 20.0f; break; }
        candidate = offset_point(base, record, radius);
        distance = avoid_zone_selected_segments_distance_00419fc0(selected.head, candidate, crt);
    }
    record.clearance_scale = radius;
    // Declaration order destroys the list before releasing the gate.
}
} // namespace bsp

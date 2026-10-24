#include "bsp/unit_hull_extents.hpp"
#include <cstddef>
#include <cstring>

namespace bsp {
// Evidence: complete 00810F60..008110CF export and listing; prefix calls
// 0077E830/008366D0, class/model extent fragment, resource-kind1/3 gate and
// game mode!=2 notification. Original __thiscall ECX=unit, plain RET.
// The extent pair has the same field order as the existing x87 fragment;
// names are hypotheses, external calls explicit, no game-host integration.
void initialize_unit_director_and_hull_00810f60(void* unit, void* const& game,
    UnitHullInitializationHost& host) {
    auto* bytes = static_cast<unsigned char*>(unit);
    auto pointer = [&](std::size_t offset) -> void* {
        void* value; std::memcpy(&value, bytes + offset, sizeof value); return value;
    };
    host.attach_lua_self_0077e830(unit);
    void* memory = host.allocate_00bf681b(0x250);
    void* director = memory ? host.construct_director_008366d0(memory, unit) : nullptr;
    std::memcpy(bytes + 0x738, &director, sizeof director);
    if (auto* klass = static_cast<unsigned char*>(pointer(0x538))) {
        UnitHullExtentClassInputs inputs;
        void* model; std::memcpy(&model, klass + 0x50, sizeof model);
        if (model) {
            inputs.model_local_box = reinterpret_cast<HitQueryBounds*>(
                static_cast<unsigned char*>(model) + 0x28);
        } else {
            std::memcpy(&inputs.length_00a0, klass + 0xA0, 4);
            std::memcpy(&inputs.width_00a4, klass + 0xA4, 4);
        }
        produce_unit_hull_extents_00810faf(
            *reinterpret_cast<UnitHullExtents*>(bytes + 0x9C8), &inputs);
    }
    if (void* resource = pointer(0xC0)) {
        auto kind = *reinterpret_cast<std::int32_t*>(static_cast<unsigned char*>(resource) + 4);
        if (kind == 1) host.instantiate_devices_0095a880(unit, 0, 1, 0);
        else if (kind != 3) return;
        if (*reinterpret_cast<std::int32_t*>(static_cast<unsigned char*>(game) + 0x1FE4) != 2)
            host.notify_slot_210(unit);
    }
}
namespace {
const double twice = 2.0; //00D7A308, bits4000000000000000.
static_assert(offsetof(HitQueryBounds, max) == 12);
static_assert(offsetof(HitQueryPoint, z) == 8);
static_assert(offsetof(UnitHullExtents, width_09cc) == 4);
}

void produce_unit_hull_extents_00810faf(UnitHullExtents& output,
    const UnitHullExtentClassInputs* unit_class) noexcept {
    if (!unit_class) return;
    auto* destination = &output;
    const auto* box = unit_class->model_local_box;
    if (!box) {
        const float class_width = unit_class->width_00a4;
        const float class_length = unit_class->length_00a0;
        __asm {
            mov eax,destination
            fld class_width
            fstp dword ptr [eax+4]
            fld class_length
            fstp dword ptr [eax]
        }
        return;
    }
    float negative_minimum;
    float maximum;
    // Retain the double2 on the x87 stack after the X store, as0081100C
    // DC C9 (FMUL ST(1),ST(0)) does. SSE-only arithmetic would change
    // denormal handling and x87 rounding/status behavior.
    __asm {
        mov ecx,box
        mov eax,destination
        fld dword ptr [ecx]
        fchs
        fstp negative_minimum
        fld dword ptr [ecx+12]
        fstp maximum
        fld negative_minimum
        fld maximum
        fcomip st(0),st(1)
        fstp st(0)
        jbe width_negative
        movss xmm0,maximum
        jmp width_selected
    width_negative:
        movss xmm0,negative_minimum
    width_selected:
        movss negative_minimum,xmm0
        fld negative_minimum
        fld twice
        fmul st(1),st(0)
        fxch
        fstp dword ptr [eax+4]
        fld dword ptr [ecx+8]
        fchs
        fstp negative_minimum
        fld dword ptr [ecx+20]
        fstp maximum
        fld negative_minimum
        fld maximum
        fcomip st(0),st(1)
        fstp st(0)
        jbe length_negative
        fmul maximum
        jmp length_selected
    length_negative:
        fmul negative_minimum
    length_selected:
        fstp dword ptr [eax]
    }
}
}

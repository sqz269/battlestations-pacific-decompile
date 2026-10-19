// Library-tagged game rows, packet cc13-mistag (marker cc13_mistag). docs/CC13_MISTAG.md.
#include "bsp/cc13_mistag.hpp"

#include <cstdio>
#include <cstring>

namespace bsp::cc13_mistag {
namespace {

template <class T>
T& at(void* base, std::size_t off) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + off);
}

void* singleton_scalar_deleting(MistagHost& host, void* object, std::uint32_t flags,
    std::uint32_t global) {
    host.clear_global(global);
    at<std::uint32_t>(object, 0) = kSingletonBaseVtable;
    if ((flags & 1) != 0) host.free_00bf65ac(object);
    return object;
}

// The inline NativeString assign of 0086C5EC..0086C696 on a {length, data} header (the pool
// block holds length + 1 bytes). A new length reallocates and terminates; the copy is
// header-length bytes whenever data is set.
void flare_assign_name(FlareReaderHost& host, void* header, const char* text) {
    const std::uint32_t length = text != nullptr ? static_cast<std::uint32_t>(std::strlen(text)) : 0;
    auto& have = at<std::uint32_t>(header, 0);
    auto& data = at<char*>(header, 4);
    if (length != have) {
        if (length == 0) {
            if (data != nullptr) host.pool_return_00bd1510(data, have + 1);
            data = nullptr;
            have = 0;
        } else {
            auto* const block = static_cast<char*>(host.pool_allocate_00bd1120(length + 1));
            if (data != nullptr) host.pool_return_00bd1510(data, have + 1);
            data = block;
            have = length;
            block[length] = 0;
        }
    }
    if (data != nullptr) std::memcpy(data, text, have);
}

// ScaleSpeed / ScaleDamp / ScaleMul: when the key is a table, its X and Y (default 0.0f) go to
// +off and +off+4; otherwise both are 0.0f. The image looks up Y first and reads X first.
void flare_xy_pair(FlareReaderHost& host, LuaRef value, void* element, std::size_t off, const char* key) {
    const LuaRef probe = host.get_by_name_00b67800(value, key);
    const bool table = host.is_table_00b661b0(probe);
    host.release_00b67700(probe);
    if (!table) {
        at<float>(element, off) = 0.0f;
        at<float>(element, off + 4) = 0.0f;
        return;
    }
    const LuaRef outer_y = host.get_by_name_00b67800(value, key);
    const LuaRef y = host.get_by_name_00b67800(outer_y, "Y");
    const LuaRef outer_x = host.get_by_name_00b67800(value, key);
    const LuaRef x = host.get_by_name_00b67800(outer_x, "X");
    const float fx = host.get_float_or_default_00b66330(x, 0.0f);
    const float fy = host.get_float_or_default_00b66330(y, 0.0f);
    at<float>(element, off) = fx;
    at<float>(element, off + 4) = fy;
    host.release_00b67700(x);
    host.release_00b67700(outer_x);
    host.release_00b67700(y);
    host.release_00b67700(outer_y);
}

}  // namespace

// 0086C240..0086CD08. Element record (operator new 60h; only these fields are written, the rest
// keep the allocator's bytes): +0h mode (WORLDSPACE 0, SCREENSPACE 1, FOREGROUNDSPRITE 2; left
// as allocated for any other State), +4h ScaleBlow, +8h/+0Ch Scale X/Y, +10h Rotation, +14h
// Distance (SCREENSPACE only), +18h Color, +1Ch Color2 (Color when nil), +24h/+28h ScaleSpeed,
// +2Ch/+30h ScaleDamp, +3Ch/+40h ScaleMul, +44h ScalePower, +48h/+50h the two texture names.
void effect_flare_read_lua_0086c240(FlareReaderHost& host, void* component, LuaRef row) {
    host.read_lua_base_00868bf0(component, row);
    const LuaRef elements = host.get_by_name_00b67800(row, "ElementTable");
    {
        const LuaRef clip = host.get_by_name_00b67800(row, "FarPlaneClip");
        at<std::uint8_t>(component, 0x20) = host.get_boolean_or_default_00b662f0(clip, true) ? 1 : 0;
        host.release_00b67700(clip);
    }
    const LuaRef key = host.construct_00b65f50();
    const LuaRef value = host.construct_00b65f50();
    {
        const LuaRef lifetime = host.get_by_name_00b67800(row, "Lifetime");
        at<float>(component, 0x24) = host.get_float_or_default_00b66330(lifetime, 0.0f);
        host.release_00b67700(lifetime);
    }
    void* const vector = static_cast<std::uint8_t*>(component) + 0x28;
    host.dword_vector_resize_zero_fill_0086aa10(vector, 0);
    host.iterate_first_00b67080(elements, key, value);
    while (!host.is_unbound_00b66420(elements, value)) {
        // 0086C382 CMP ESI,EBP / JE 0086C3A7: a null block skips 00BF7CD1 (ESI = 0); the field
        // stores that follow are unguarded, so the image writes through null then.
        void* const e = host.operator_new_00bf681b(0x60);
        if (e != nullptr)
            host.eh_vector_construct_00bf7cd1(static_cast<std::uint8_t*>(e) + 0x48, 8, 2,
                0x00415270, 0x0041DD20);
        {
            const LuaRef outer_y = host.get_by_name_00b67800(value, "Scale");
            const LuaRef y = host.get_by_name_00b67800(outer_y, "Y");
            const LuaRef outer_x = host.get_by_name_00b67800(value, "Scale");
            const LuaRef x = host.get_by_name_00b67800(outer_x, "X");
            const float fx = host.get_number_00b66270(x);
            const float fy = host.get_number_00b66270(y);
            at<float>(e, 0x08) = fx;
            at<float>(e, 0x0C) = fy;
            host.release_00b67700(x);
            host.release_00b67700(outer_x);
            host.release_00b67700(y);
            host.release_00b67700(outer_y);
        }
        {
            const LuaRef rotation = host.get_by_name_00b67800(value, "Rotation");
            at<float>(e, 0x10) = host.get_number_00b66270(rotation);
            host.release_00b67700(rotation);
            const LuaRef color = host.get_by_name_00b67800(value, "Color");
            at<std::uint32_t>(e, 0x18) = host.read_packed_bgra_00b67f10(color);
            host.release_00b67700(color);
        }
        const LuaRef color2 = host.get_by_name_00b67800(value, "Color2");   // held to the end
        at<std::uint32_t>(e, 0x1C) = host.is_nil_00b65fb0(color2)
            ? at<std::uint32_t>(e, 0x18) : host.read_packed_bgra_00b67f10(color2);
        // 0086C54E..0086C6B3: TextureName1 (TextureName when nil) and TextureName2; a nil name
        // assigns "" (0086C754, the image's empty string 00CE3A0C).
        for (std::int32_t i = 0; i < 2; ++i) {
            char name[0x40];
            std::snprintf(name, sizeof name, "TextureName%d", i + 1);
            const LuaRef texture = host.get_by_name_00b67800(value, name);
            if (host.is_nil_00b65fb0(texture) && i == 0) {
                const LuaRef plain = host.get_by_name_00b67800(value, "TextureName");
                host.assign_00b67690(texture, plain);
                host.release_00b67700(plain);
            }
            void* const header = static_cast<std::uint8_t*>(e) + 0x48 + 8 * static_cast<std::size_t>(i);
            flare_assign_name(host, header, host.is_nil_00b65fb0(texture) ? ""
                : host.get_string_00b662b0(texture));
            host.release_00b67700(texture);
        }
        // 0086C6B9..0086C80F: State into a local NativeString (no null test on the string),
        // compared case-blind when its buffer is set.
        alignas(4) std::uint8_t state[8] = {};
        {
            const LuaRef state_ref = host.get_by_name_00b67800(value, "State");
            const char* const text = host.get_string_00b662b0(state_ref);
            host.native_string_resize_0041dd40(state, static_cast<std::uint32_t>(std::strlen(text)));
            if (char* const data = at<char*>(state, 4))
                std::memcpy(data, text, at<std::uint32_t>(state, 0) + 1);
            host.release_00b67700(state_ref);
        }
        if (const char* const mode = at<char*>(state, 4)) {
            if (_stricmp(mode, "WORLDSPACE") == 0) {
                at<std::uint32_t>(e, 0x00) = 0;
            } else if (_stricmp(mode, "SCREENSPACE") == 0) {
                at<std::uint32_t>(e, 0x00) = 1;
                const LuaRef distance = host.get_by_name_00b67800(value, "Distance");
                at<float>(e, 0x14) = host.get_number_00b66270(distance);
                host.release_00b67700(distance);
            } else if (_stricmp(mode, "FOREGROUNDSPRITE") == 0) {
                at<std::uint32_t>(e, 0x00) = 2;
            }
        }
        {
            const LuaRef blow = host.get_by_name_00b67800(value, "ScaleBlow");
            at<std::uint8_t>(e, 0x04) = host.get_boolean_or_default_00b662f0(blow, false) ? 1 : 0;
            host.release_00b67700(blow);
        }
        flare_xy_pair(host, value, e, 0x24, "ScaleSpeed");
        flare_xy_pair(host, value, e, 0x2C, "ScaleDamp");
        flare_xy_pair(host, value, e, 0x3C, "ScaleMul");
        {
            const LuaRef power = host.get_by_name_00b67800(value, "ScalePower");
            at<float>(e, 0x44) = host.get_float_or_default_00b66330(power, 0.0f);
            host.release_00b67700(power);
        }
        // 0086CC19..0086CC47: push_back, growing to max(2 * capacity, 1) when full.
        if (at<std::uint32_t>(vector, 4) == at<std::uint32_t>(vector, 8)) {
            const std::uint32_t doubled = at<std::uint32_t>(vector, 8) * 2;
            host.dword_vector_reserve_0086a280(vector, static_cast<std::int32_t>(doubled) > 1 ? doubled : 1);
        }
        // The slot address is tested for null before the store (0086CC41); the count rises anyway.
        if (void** const slot = at<void**>(vector, 0) + at<std::uint32_t>(vector, 4)) *slot = e;
        at<std::uint32_t>(vector, 4) += 1;
        if (void* const data = at<void*>(state, 4)) host.pool_return_00bd1510(data, at<std::uint32_t>(state, 0) + 1);
        host.release_00b67700(color2);
        host.iterate_next_00b67190(elements, key, value);
    }
    host.release_00b67700(value);
    host.release_00b67700(key);
    host.release_00b67700(elements);
}

// Part B, sorted by address; each row's bytes are C7 01 <vtable> C3 (tools/cc13_mistag_partb_check.py).
const VtResetDestructor kVtResetDestructors[kVtResetDestructorCount] = {
    {0x006CFE70, 0x00CE4974}, {0x006D16E0, 0x00CE4974}, {0x006D1760, 0x00CE4974},
    {0x006D17E0, 0x00CE4974}, {0x006D1860, 0x00CE4974}, {0x00743D80, 0x00CFF354},
    {0x007448F0, 0x00CE4974}, {0x00749AF0, 0x00CE4974}, {0x00749B00, 0x00CE4974},
    {0x0074CEC0, 0x00CE4974}, {0x0074D020, 0x00CE4974}, {0x0074EBF0, 0x00CE4974},
    {0x00759D00, 0x00CE4974}, {0x0075A410, 0x00CE4974}, {0x0075A420, 0x00CE4974},
    {0x0075A430, 0x00CE4974}, {0x0075A460, 0x00CE4974}, {0x0075A500, 0x00CE4974},
    {0x0075A5B0, 0x00CE4974}, {0x0075A800, 0x00CE4974}, {0x0075A810, 0x00CE4974},
    {0x0075A870, 0x00CE4974}, {0x0075A8E0, 0x00CE4974}, {0x0075A950, 0x00CE4974},
    {0x0075A9C0, 0x00CE4974}, {0x0075ABA0, 0x00CE4974}, {0x0075AC10, 0x00CE4974},
    {0x0075AC80, 0x00CE4974}, {0x0075AD20, 0x00CE4974}, {0x0075AD90, 0x00CE4974},
    {0x0075AE40, 0x00CE4974}, {0x0075B250, 0x00CE4974}, {0x0075B420, 0x00CE4974},
    {0x0075BBC0, 0x00CE4974}, {0x0075CBB0, 0x00CE4974}, {0x0075CE30, 0x00CE4974},
    {0x0075D0C0, 0x00CE4974}, {0x0075D340, 0x00CE4974}, {0x0075D460, 0x00CE4974},
    {0x0075D520, 0x00CE4974}, {0x0075D640, 0x00CE4974}, {0x0075DA00, 0x00CE4974},
    {0x0075DBE0, 0x00CE4974}, {0x0075DC90, 0x00CE4974}, {0x0075DD40, 0x00CE4974},
    {0x0075DDF0, 0x00CE4974}, {0x0075DF40, 0x00CE4974}, {0x0075E080, 0x00CE4974},
    {0x0075E1B0, 0x00CE4974}, {0x0075E2E0, 0x00CE4974}, {0x0075E410, 0x00CE4974},
    {0x0075E5B0, 0x00CE4974}, {0x0075E6A0, 0x00CE4974}, {0x0075E7B0, 0x00CE4974},
    {0x0075E8A0, 0x00CE4974}, {0x0075E9B0, 0x00CE4974}, {0x0075EAA0, 0x00CE4974},
    {0x0075EBB0, 0x00CE4974}, {0x0075ED20, 0x00CE4974}, {0x0075EE60, 0x00CE4974},
    {0x0075F070, 0x00CE4974}, {0x0075F210, 0x00CE4974}, {0x0075F3C0, 0x00CE4974},
    {0x0075F5B0, 0x00CE4974}, {0x0075F6E0, 0x00CE4974}, {0x0075F7F0, 0x00CE4974},
    {0x0075F900, 0x00CE4974}, {0x0075FA40, 0x00CE4974}, {0x0075FD00, 0x00CE4974},
    {0x007602E0, 0x00CE4974}, {0x00760410, 0x00CE4974}, {0x00760540, 0x00CE4974},
    {0x007606E0, 0x00CE4974}, {0x00760820, 0x00CE4974}, {0x00760960, 0x00CE4974},
    {0x00760AD0, 0x00CE4974}, {0x00760F10, 0x00CE4974}, {0x00761260, 0x00CE4974},
    {0x007613A0, 0x00CE4974}, {0x00761440, 0x00CE4974}, {0x007614E0, 0x00CE4974},
    {0x00761620, 0x00CE4974}, {0x00761FE0, 0x00CE4974}, {0x00762210, 0x00CE4974},
    {0x007622A0, 0x00CE4974}, {0x00762450, 0x00CE4974}, {0x00762740, 0x00CE4974},
    {0x007628E0, 0x00CE4974}, {0x00762AC0, 0x00CE4974}, {0x00762B80, 0x00CE4974},
    {0x00762CE0, 0x00CE4974}, {0x00763200, 0x00CE4974}, {0x00763BC0, 0x00CE4974},
    {0x00763E30, 0x00CE4974}, {0x007643B0, 0x00CE4974}, {0x00764460, 0x00CE4974},
    {0x00764500, 0x00CE4974}, {0x00764790, 0x00CE4974}, {0x00764910, 0x00CE4974},
    {0x007B3DA0, 0x00D056D0}, {0x007B5910, 0x00D05840},   // row 119 (rev4), see the doc
    {0x009AF970, 0x00D1FDB8}, {0x009BC680, 0x00D20838},
    {0x009C8CE0, 0x00D20E84}, {0x009CD390, 0x00D21148}, {0x009DAAB0, 0x00D056D0},
    {0x009DAC40, 0x00D056D0}, {0x009DAC90, 0x00D056D0}, {0x009DADD0, 0x00D056D0},
    {0x009DAE60, 0x00D056D0}, {0x009DB010, 0x00D056D0}, {0x009DB2B0, 0x00D056D0},
    {0x009DB360, 0x00D056D0}, {0x009DB560, 0x00D056D0}, {0x009DB5B0, 0x00D056D0},
    {0x009DB620, 0x00D056D0}, {0x009DB790, 0x00D056D0}, {0x009DB8D0, 0x00D056D0},
    {0x009F5280, 0x00D21B40},
};

bool vt_reset_destruct(std::uint32_t address, void* object) noexcept {
    for (const auto& row : kVtResetDestructors) {
        if (row.address != address) continue;
        at<std::uint32_t>(object, 0) = row.vtable;
        return true;
    }
    return false;
}

// The irregular rows' five one-instruction virtuals, read from the disk bytes:
// 009BE0E0 33 C0 C3; 009DAAA0 32 C0 C2 04 00; 009DAC30 D9 05 58 39 CE 00 C3; 009DB000 C3;
// 00743E50 8D 81 10 01 00 00 C3.
std::uint32_t virtual_return_zero_009be0e0(void*) noexcept { return 0; }
std::uint8_t virtual_return_false_009daaa0(void*, std::uint32_t) noexcept { return 0; }
float virtual_return_two_009dac30(void*) noexcept { return 2.0f; }
void virtual_no_op_009db000(void*) noexcept {}
void* landfort_create_message_field_110_00743e50(void* message) noexcept {
    return static_cast<std::uint8_t*>(message) + 0x110;
}

// Part C: F6 44 24 04 01 56 8B F1 C7 06 <vtable> 74 09 56 E8 <00BF65AC> 83 C4 04 8B C6 5E C2 04 00.
const VtResetDestructor kVtResetDeletingDestructors[kVtResetDeletingDestructorCount] = {
    {0x006CDD40, 0x00CF88F8}, {0x00735990, 0x00CFE9F4}, {0x00861BA0, 0x00D0D314},
    {0x00862750, 0x00D0D314}, {0x008627F0, 0x00D0D31C},
};

void* vt_reset_scalar_deleting_destruct(MistagHost& host, std::uint32_t address, void* object,
    std::uint32_t flags) {
    for (const auto& row : kVtResetDeletingDestructors) {
        if (row.address != address) continue;
        at<std::uint32_t>(object, 0) = row.vtable;
        if ((flags & 1) != 0) host.free_00bf65ac(object);
        return object;
    }
    return nullptr;
}

// Each row read from the disk bytes: B8 imm32 C3. The slot is the only .rdata dword equal to
// the routine's address (two for 0085EA30).
const TrivialGetter kTrivialGetters[kTrivialGetterCount] = {
    {0x008561D0, 0x00D0C410, 0x00CE544C, "Torpedo"},
    {0x0085EA30, 0x00D0D158, 0x00CEA74C, "WaterMine"},          // also slot 00D7CE88
    {0x009A28A0, 0x00D1F4FC, 0x00CFB630, "closetoship"},
    {0x009ADD20, 0x00D1FCCC, 0x00F8A424, nullptr},
    {0x009B33C0, 0x00D1FFB4, 0x00D20008, "LAND"},
    {0x009B33E0, 0x00D1FFDC, 0x00E08FA0, nullptr},
    {0x009B7B60, 0x00D2026C, 0x00F8A474, nullptr},
    {0x009BA820, 0x00D2039C, 0x00D203F0, "conditioned"},
    {0x009BAA90, 0x00D20494, 0x00D204E8, "gainaltitude"},
    {0x009C3B80, 0x00D20C1C, 0x00E08F78, nullptr},
    {0x009C79A0, 0x00D20E54, 0x00E08F20, nullptr},
    {0x009CC3F0, 0x00D210F4, 0x00CFB4D0, "strafe"},
    {0x009CFA00, 0x00D2123C, 0x00D21290, "takeoff"},
    {0x009DAC20, 0x00D215BC, 0x00E08F70, nullptr},
    {0x00749000, 0x00CFF79C, 0x00CFF7BC, "MLandFort"},
    {0x00951AE0, 0x00D19C00, 0x00D19C20, "MReconPlane"},
    {0x00951B30, 0x00D19C3C, 0x00D19C5C, "MPlaneFighter"},
    {0x00951C10, 0x00D19C7C, 0x00D19F34, "MPlaneDiveBomber"},
    {0x00951CF0, 0x00D19F58, 0x00D1A20C, "MPlaneTorpedoBomber"},
    {0x00951DD0, 0x00D1A230, 0x00D1A4E4, "MPlaneKamikaze"},
    {0x00951E20, 0x00D1A504, 0x00D1A524, "MPlaneBomber"},
    {0x00951E70, 0x00D1A544, 0x00D1A564, "MCommandBuilding"},
    {0x007AB9F0, 0x00D05088, 0x00CFA6C4, "Paratrooper"},
};

std::uint32_t trivial_getter_result(std::uint32_t address) noexcept {
    for (const TrivialGetter& g : kTrivialGetters)
        if (g.address == address) return g.value;
    return 0;
}

// 00735880..007358A8 (Ghidra CG_scalar_deleting_dtor_00735880; vtable slot 00CFEA04). The
// out-of-line base 00735770 is an undefined chunk.
void* singleton_scalar_deleting_destruct_00735880(MistagHost& host, void* object, std::uint32_t flags) {
    return singleton_scalar_deleting(host, object, flags, 0x010904F8);
}

// 00735900..00735928 (slot 00CFEA0C); out-of-line base 007357B0 undefined.
void* singleton_scalar_deleting_destruct_00735900(MistagHost& host, void* object, std::uint32_t flags) {
    return singleton_scalar_deleting(host, object, flags, 0x0109029C);
}

// 00866090..008660B8 (slot 00D0D3C8); out-of-line base 00866060 undefined.
void* singleton_scalar_deleting_destruct_00866090(MistagHost& host, void* object, std::uint32_t flags) {
    return singleton_scalar_deleting(host, object, flags, 0x00F87658);
}

// 00737110..00737147 (slot 00CFEA2C). The singleton base is the +4h subobject (the image takes
// 0 for a null this and would then store through 0; not modelled). Order as in the listing:
// global, +4h vtable, +0h vtable 00CFE9F4, then _free.
void* msar_provider_scalar_deleting_destruct_00737110(MistagHost& host, void* object, std::uint32_t flags) {
    host.clear_global(0x010904F8);
    at<std::uint32_t>(object, 4) = kSingletonBaseVtable;
    at<std::uint32_t>(object, 0) = 0x00CFE9F4;
    if ((flags & 1) != 0) host.free_00bf65ac(object);
    return object;
}

// 00996200..00996264. The two FSTP to [esp+4] round to float; the -0.0f at 00D7A208 is read
// from the image (bytes 00 00 00 80). FCOMIP with JBE: an unordered compare returns 0.
std::uint32_t angle_within_tolerance_00996200(MistagHost& host, void* condition) {
    const float current = host.object_vcall_50_angle(at<void*>(condition, 0x18));
    const float d = host.wrapped_angle_subtract_00438b10(at<float>(condition, 0x1C), current);
    const float magnitude = d > 0.0f ? d : -0.0f - d;     // 0099622A..00996240
    return at<float>(condition, 0x20) > magnitude ? 1u : 0u;
}

// 007B1A80..007B1AFF. The loop re-reads first/last each pass; index i is checked twice as the
// image does (first == null or i >= size -> 00BF6713) before the element is freed and nulled.
void point3_ptr_array_destruct_007b1a80(MistagHost& host, void* array) {
    at<std::uint32_t>(array, 0) = 0x00D053A4;
    for (std::uint32_t i = 0;; ++i) {
        auto* const first = at<void**>(array, 8);
        if (first == nullptr) break;
        const auto size = static_cast<std::uint32_t>(at<void**>(array, 0xC) - first);
        if (i >= size) break;
        void*& element = first[i];
        if (element != nullptr) {
            host.free_00bf65ac(element);
            element = nullptr;
        }
    }
    if (at<void*>(array, 8) != nullptr) host.free_00bf65ac(at<void*>(array, 8));
    at<std::uint32_t>(array, 0x08) = 0;
    at<std::uint32_t>(array, 0x0C) = 0;
    at<std::uint32_t>(array, 0x10) = 0;
}

// 00728B80..00728B8A: GUN_CREATE message (constructor 0072A1D0, sender 0072B640).
void gun_create_message_destruct_00728b80(MistagDtorHost& host, void* message) {
    at<std::uint32_t>(message, 0) = 0x00CFDE1C;
    host.net_entity_create_message_destruct_007668e0(message);
}

// 00858150..00858161: the effect-instance base shared by the trace-line, waterdrops, light and
// splash instances (their destructors store the same pair); +8h is the second subobject.
void effect_instance_base_destruct_00858150(MistagDtorHost& host, void* instance) {
    at<std::uint32_t>(instance, 0) = 0x00D0C88C;
    at<std::uint32_t>(instance, 8) = 0x00D0C888;
    host.ref_counted_destroy_base_00bd30f0(instance);
}

// 009A1BF0..009A1BFD: BotApproachCloseToShip block (constructor 009A1D60); ADD ECX,2Ch then
// JMP 0064A610 on the observed-owner member.
void bot_approach_close_to_ship_destruct_009a1bf0(MistagDtorHost& host, void* block) {
    at<std::uint32_t>(block, 0) = 0x00D1F470;
    host.observed_owner_base_destroy_0064a610(static_cast<std::uint8_t*>(block) + 0x2C);
}

// 009BE2B0..009BE2BD: BotApproachMoveTo block (constructor 009C1C30); ADD ECX,30h then
// JMP 0042BCA0 on the target-observer member.
void bot_approach_move_to_destruct_009be2b0(MistagDtorHost& host, void* block) {
    at<std::uint32_t>(block, 0) = 0x00D209CC;
    host.target_observer_destruct_0042bca0(static_cast<std::uint8_t*>(block) + 0x30);
}

// 007B1BD0..007B1BDA and 007B1BE0..007B1BEA: two byte-identical copies, the path component
// (constructor 007B28C0) over its Point3PtrArray base 007B1A80.
void path_component_destruct_007b1bd0(MistagDtorHost& host, void* component) {
    at<std::uint32_t>(component, 0) = 0x00D053D8;
    host.point3_ptr_array_destruct_007b1a80(component);
}
void path_component_destruct_007b1be0(MistagDtorHost& host, void* component) {
    at<std::uint32_t>(component, 0) = 0x00D053D8;
    host.point3_ptr_array_destruct_007b1a80(component);
}

// 0083B120..0083B34F, in listing order. Vector fields are {allocator, first, last, end} with the
// first pointer named by its offset; RefPtr slots release through InterlockedDecrement(+4h).
void game_settings_destruct_0083b120(SettingsDtorHost& host, void* settings) {
    void* const s = settings;
    at<std::uint32_t>(s, 0x000) = 0x00CE3994;
    // 0083B152..0083B1B5: std::vector<RefPtr> first +664h / last +668h, walked by a checked
    // index; a non-null slot is released and nulled.
    for (std::uint32_t i = 0;; ++i) {
        auto* const first = at<void**>(s, 0x664);
        if (first == nullptr) break;
        if (i >= static_cast<std::uint32_t>(at<void**>(s, 0x668) - first)) break;
        void*& slot = first[i];
        if (slot != nullptr) {
            host.ref_release(slot);
            slot = nullptr;
        }
    }
    // 0083B1B7..0083B209: 30h-byte elements first +740h / last +744h.
    if (auto* const first = at<std::uint8_t*>(s, 0x740)) {
        auto* const last = at<std::uint8_t*>(s, 0x744);                  // EBX, cached at 0083B1C6
        for (std::uint8_t* p = first; p != last; p += 0x30)
            host.element30_destruct_0041e7a0(p);
        host.free_00bf65ac(at<void*>(s, 0x740));
    }
    at<std::uint32_t>(s, 0x740) = 0;
    at<std::uint32_t>(s, 0x744) = 0;
    at<std::uint32_t>(s, 0x748) = 0;
    host.eh_vector_destruct_00bf7c6e(static_cast<std::uint8_t*>(s) + 0x72C, 4, 2, 0x0041DE40);
    host.container_6b8_resize_0041f370(static_cast<std::uint8_t*>(s) + 0x6B8, 0);
    host.free_00bf6989(at<void*>(s, 0x6B8));
    if (at<void*>(s, 0x674) != nullptr) host.free_00bf65ac(at<void*>(s, 0x674));
    at<std::uint32_t>(s, 0x674) = 0;
    at<std::uint32_t>(s, 0x678) = 0;
    at<std::uint32_t>(s, 0x67C) = 0;
    if (at<void*>(s, 0x664) != nullptr) host.free_00bf65ac(at<void*>(s, 0x664));
    at<std::uint32_t>(s, 0x664) = 0;
    at<std::uint32_t>(s, 0x668) = 0;
    at<std::uint32_t>(s, 0x66C) = 0;
    // 0083B26F..0083B2B3: the vector at +3E4h (first +3E8h, last +3ECh).
    void* const v = static_cast<std::uint8_t*>(s) + 0x3E4;
    if (at<void*>(v, 4) != nullptr) {
        host.destroy_range_00421470(v, s, at<void*>(v, 4), at<void*>(v, 8));
        host.free_00bf65ac(at<void*>(v, 4));
    }
    at<std::uint32_t>(v, 4) = 0;
    at<std::uint32_t>(v, 8) = 0;
    at<std::uint32_t>(v, 0xC) = 0;
    // RefPtrs +3A8h then +3A4h: nulled only when they were set.
    for (std::size_t off : {0x3A8u, 0x3A4u}) {
        if (void* const object = at<void*>(s, off)) {
            host.ref_release(object);
            at<void*>(s, off) = nullptr;
        }
    }
    if (at<void*>(s, 0x1E0) != nullptr) host.free_00bf65ac(at<void*>(s, 0x1E0));
    at<std::uint32_t>(s, 0x1E0) = 0;
    at<std::uint32_t>(s, 0x1E4) = 0;
    at<std::uint32_t>(s, 0x1E8) = 0;
    host.clear_game_settings_publication_00f8753c();
    at<std::uint32_t>(s, 0x000) = 0x00CE3818;
}

// 007CFD20..007D0342, in listing order (the SEH state bytes are not modelled). Constants read
// at the load sites: 00D7A260 = -1.0f, 00D7A24C = 1.0f, 00CE4C04 = 10000.0f (461C3C00h),
// 00CE65D8 = -100.0f (C2C80000h), 00CE3804 = 1000.0f (447A0000h). EDI holds -1 from 007CFD69
// to the end, EBX holds 0.
void* plane_unit_instance_construct_007cfd20(PlaneCtorHost& host, void* plane, std::uint32_t arg,
    std::uint32_t cls) {
    void* const p = plane;
    auto* const b = static_cast<std::uint8_t*>(p);
    host.unit_game_object_construct_0095cc90(p, arg, cls);
    host.projectile_shot_base_construct_006e22d0(b + 0x72C);
    at<std::uint32_t>(p, 0x72C) = 0x00D06130;
    at<std::uint32_t>(p, 0x000) = 0x00D05F20;
    at<std::uint32_t>(p, 0x010) = 0x00D05F08;
    at<std::uint32_t>(p, 0x024) = 0x00D05F00;
    at<std::uint32_t>(p, 0x170) = 0x00D05EFC;
    at<std::uint32_t>(p, 0x1E4) = 0x00D05EF4;
    at<std::uint32_t>(p, 0x310) = 0x00D05EDC;
    at<std::uint32_t>(p, 0x38C) = 0x00D05ED8;
    at<std::int32_t>(p, 0x800) = -1;                                     // 007CFDB4
    at<std::uint32_t>(p, 0x808) = 0;
    at<std::uint32_t>(p, 0x80C) = 0;
    host.plane_wanderer_construct_007c4560(b + 0x810, p);                // 007CFDC6
    host.plane_init_control_block_007baf90(b + 0x84C, p);
    at<float>(p, 0x914) = -1.0f;                                         // 007CFDDF
    at<std::uint8_t>(p, 0x8FC) = 0;
    at<std::uint8_t>(p, 0x8FD) = 0;
    at<std::uint8_t>(p, 0x911) = 0;
    at<float>(p, 0x964) = 0.0f;
    for (std::size_t off = 0x994; off <= 0x9B0; off += 4) at<std::uint32_t>(p, off) = 0;
    at<float>(p, 0x9B8) = 0.0f;
    at<std::uint8_t>(p, 0x9C1) = 0;
    at<std::uint8_t>(p, 0x9C0) = 0;
    at<std::uint8_t>(p, 0x9C3) = 1;
    at<std::uint8_t>(p, 0x9C9) = 0;
    at<std::uint8_t>(p, 0x9C8) = 0;
    at<std::uint8_t>(p, 0x9CB) = 1;
    at<std::uint32_t>(p, 0x9D4) = 0;
    at<std::int32_t>(p, 0x9D8) = -1;                                     // 007CFE72, the dword -1
    at<float>(p, 0x9DC) = 1.0f;                                          // 007CFE78 (XMM1 from 007CFDEA)
    at<std::uint8_t>(p, 0x9E0) = 0;
    at<std::uint8_t>(p, 0x9E1) = 0;
    at<std::uint8_t>(p, 0x9E2) = 0;
    // 007CFE92..007CFEB8: the live controls (yaw, pitch, roll, air brake, boost) zero and the
    // throttle +9F0h 1.0f; 007CFEB8..007CFEDE: the pilot command block the same, its throttle
    // +A08h 1.0f, the pending byte +A14h clear.
    for (std::size_t base : {0x9E4u, 0x9FCu}) {
        for (std::size_t i = 0; i < 6; ++i) at<std::uint32_t>(p, base + 4 * i) = 0;
        at<float>(p, base + 0xC) = 1.0f;
    }
    at<std::uint8_t>(p, 0xA14) = 0;
    at<float>(p, 0xA20) = 1.0f;
    at<float>(p, 0xA1C) = 0.0f;
    at<float>(p, 0xA18) = 0.0f;
    at<float>(p, 0xA2C) = 1.0f;
    at<float>(p, 0xA28) = 0.0f;
    at<float>(p, 0xA24) = 0.0f;
    // The {first, last, end} triples of +A30h, +A3Ch and +A4Ch (their allocator dwords untouched).
    for (std::size_t off : {0xA30u, 0xA34u, 0xA38u, 0xA40u, 0xA44u, 0xA48u, 0xA50u, 0xA54u, 0xA58u})
        at<std::uint32_t>(p, off) = 0;
    host.eh_vector_construct_00bf7cd1(b + 0xA5C, 0x10, 4, 0x007C3670, 0x007C3680);
    at<std::uint8_t>(p, 0xA9D) = 0;
    at<std::uint32_t>(p, 0xAA4) = 0;
    at<std::uint8_t>(p, 0xAA8) = 0;
    host.plane_flight_controller_construct_007d7ea0(b + 0xAB0, p);       // 007CFF87
    for (std::size_t off = 0xBB0; off <= 0xBC8; off += 4) at<std::uint32_t>(p, off) = 0;
    for (std::size_t off = 0xBCC; off <= 0xBE4; off += 4) at<float>(p, off) = 1.0f;
    at<float>(p, 0xC04) = 0.0f;                                          // 007D0006 (XORPS 007CFF94)
    at<std::uint32_t>(p, 0xC18) = 5;
    at<float>(p, 0xC2C) = 1.0f;
    at<std::uint32_t>(p, 0xC30) = 0;
    at<std::uint8_t>(p, 0xC41) = 0;
    at<std::uint8_t>(p, 0xC49) = 0;
    at<std::uint32_t>(p, 0xC50) = 0;
    at<std::uint32_t>(p, 0xC58) = 0;
    host.plane_fx_component_construct_007cd520(b + 0xC98);
    at<std::uint8_t>(p, 0x969) = 1;
    at<std::uint32_t>(p, 0x324) = 2;
    at<std::uint32_t>(p, 0xC4C) = 0;
    at<std::uint32_t>(p, 0x900) = 0;                                     // 007D0060, the flight state
    if (at<std::uint8_t>(p, 0x904) != 0) at<std::uint8_t>(p, 0x904) = 0;
    at<float>(p, 0xC10) = 10000.0f;
    at<float>(p, 0xC14) = -1.0f;
    at<std::uint8_t>(p, 0x910) = 0;
    at<float>(p, 0x908) = 0.0f;
    at<float>(p, 0x90C) = 0.0f;
    at<std::uint8_t>(p, 0xDFC) = 0;
    at<float>(p, 0xBC4) = 1.0f;                                          // over 007CFFB7's 0
    at<float>(p, 0xC5C) = -100.0f;
    at<float>(p, 0xC60) = 0.0f;
    // 007D00D5..007D00F2: +968h takes the counter; the counter steps and wraps to 0 at 12
    // (signed byte compare; the stepped value is stored before the reset).
    const std::uint8_t counter = host.read_counter_00f87151();
    at<std::uint8_t>(p, 0x968) = counter;
    const auto next = static_cast<std::uint8_t>(counter + 1);
    host.write_counter_00f87151(next);
    if (!(static_cast<std::int8_t>(next) < 12)) host.write_counter_00f87151(0);
    for (std::size_t off : {0xDDCu, 0xDE0u, 0xDE4u, 0xDE8u}) at<std::uint32_t>(p, off) = 0;
    at<std::uint8_t>(p, 0xC00) = 0;
    at<std::uint8_t>(p, 0xC01) = 0;
    at<std::uint32_t>(p, 0xAAC) = 0x0A;
    at<std::uint32_t>(p, 0x0C4) = 0x0F;                                  // the plane base class id
    const std::uint32_t zero = 0;
    std::memcpy(b + 0xC36, &zero, 4);                                    // unaligned dword store
    at<std::uint8_t>(p, 0xC3A) = 0;                                      // 007D0136
    at<void*>(p, 0x7F8) = p;
    at<std::uint8_t>(p, 0x770) = 1;
    at<float>(p, 0x9BC) = 0.0f;
    for (std::size_t off = 0xC64; off <= 0xC88; off += 4) at<float>(p, off) = 0.0f;
    at<std::uint8_t>(p, 0xC35) = 0;
    at<std::uint32_t>(p, 0xBF4) = 0;
    at<std::uint8_t>(p, 0xBF8) = 0;
    at<std::uint8_t>(p, 0xC02) = 0;
    at<std::uint32_t>(p, 0xDF8) = 1;
    at<std::uint32_t>(p, 0x9D0) = 0;                                     // 007D01C3
    at<std::uint32_t>(p, 0x970) = 0;
    at<std::uint32_t>(p, 0x96C) = 0;
    at<float>(p, 0xE8C) = 0.0f;
    at<std::int32_t>(p, 0xE90) = -1;
    at<float>(p, 0xBFC) = 1000.0f;                                       // 007D01EB
    // 007D01F3..007D0273: an identity 4x4 at +E40h; +E00h = 1.0f and 007D027B..007D0332 copy
    // +E44h..+E7Ch to +E04h..+E3Ch through FLD/FSTP (0.0f and 1.0f copy exactly).
    for (std::size_t i = 0; i < 16; ++i) at<float>(p, 0xE40 + 4 * i) = (i % 5 == 0) ? 1.0f : 0.0f;
    at<float>(p, 0xE00) = 1.0f;
    for (std::size_t i = 1; i < 16; ++i) at<float>(p, 0xE00 + 4 * i) = at<float>(p, 0xE40 + 4 * i);
    return p;
}

// 007D03A0..007D05B3, in listing order. The SEH frame's state bytes (7 down to -1) only track
// which members remain for unwinding.
void plane_unit_instance_destruct_007d03a0(PlaneDtorHost& host, void* plane) {
    void* const p = plane;
    auto* const b = static_cast<std::uint8_t*>(p);
    at<std::uint32_t>(p, 0x000) = 0x00D05F20;
    at<std::uint32_t>(p, 0x010) = 0x00D05F08;
    at<std::uint32_t>(p, 0x024) = 0x00D05F00;
    at<std::uint32_t>(p, 0x170) = 0x00D05EFC;
    at<std::uint32_t>(p, 0x1E4) = 0x00D05EF4;
    at<std::uint32_t>(p, 0x310) = 0x00D05EDC;
    at<std::uint32_t>(p, 0x38C) = 0x00D05ED8;
    at<std::uint32_t>(p, 0x72C) = 0x00D06130;
    // 007D0407: the physics body handle.
    if (at<std::uint32_t>(p, 0x804) != 0) {
        host.queue_body_release_00c34f70(at<std::uint32_t>(p, 0x804));
        at<std::uint32_t>(p, 0x804) = 0;
    }
    if (void* const slot = at<void*>(p, 0xE80)) {
        host.plane_effect_slot_destruct_007e2550(slot);
        host.free_00bf65ac(slot);
        at<void*>(p, 0xE80) = nullptr;
    }
    if (void* const object = at<void*>(p, 0xAA4)) {
        host.ref_release(object);
        at<void*>(p, 0xAA4) = nullptr;
    }
    // 007D0476..007D04B9: four arrays of four owned pointers (+D9Ch + 10h*k) with signed counts
    // at +DDCh + 4*k; each count is decremented in place to 0, freeing from the top. The image
    // indexes as [EDI + 4*(count + 367h + 4k)] (EBP = FFFFF58Bh - EDI + ESI).
    for (std::size_t k = 0; k < 4; ++k) {
        auto& count = at<std::int32_t>(p, 0xDDC + 4 * k);
        while (count > 0) {
            --count;
            host.free_00bf65ac(at<void*>(p, 0xD9C + 0x10 * k + 4 * static_cast<std::size_t>(count)));
        }
    }
    host.plane_fx_component_destruct_007ec490(b + 0xC98);
    host.owned_ptr_holder_destruct_007d7a10(b + 0xAB0);
    // The four 10h-byte wing-tip records at +A5Ch (docs/CC13_BIG47A.md), 007C3680 each.
    host.eh_vector_destruct_00bf7c6e(b + 0xA5C, 0x10, 4, 0x007C3680);
    // Vectors {allocator, first, last, end} at +A4Ch then +A3Ch; the range destroy and the free
    // run only when first is set, the triple is zeroed either way.
    for (std::size_t off : {0xA4Cu, 0xA3Cu}) {
        void* const v = b + off;
        if (at<void*>(v, 4) != nullptr) {
            host.destroy_range_007cd430(v, p, at<void*>(v, 4), at<void*>(v, 8));
            host.free_00bf65ac(at<void*>(v, 4));
        }
        at<std::uint32_t>(v, 4) = 0;
        at<std::uint32_t>(v, 8) = 0;
        at<std::uint32_t>(v, 0xC) = 0;
    }
    // 007D0566: resize to 0, then free the buffer pointer at +A30h with no null test.
    host.live_effect_references_resize_004c9550(b + 0xA30, 0);
    host.free_00bf6989(at<void*>(p, 0xA30));
    host.projectile_shot_destruct_006e25a0(b + 0x72C);
    host.unit_game_object_destruct_00959940(p);
}

// 0099A880..0099A934. Float constants read from the image: 00E0E2EC = 0.24f (3E75C28Fh, a .data
// dword with no writer in the index), 00CF8B3C = 256.0f, 00D7A260 = -1.0f.
void* pilot_bot_construct_0099a880(MistagHost& host, void* bot) {
    host.gun_bot_construct_base_0072bbd0(bot, host.pilot_robots_table_00f8a30c());
    at<std::uint32_t>(bot, 0x00) = 0x00D1F348;
    at<std::uint32_t>(bot, 0x1C) = 0x00D1F32C;
    for (std::size_t off = 0x58; off <= 0x6C; off += 4) at<std::uint32_t>(bot, off) = 0;
    host.eh_vector_construct_00bf7cd1(static_cast<std::uint8_t*>(bot) + 0x84, 0x1C, 2,
        0x0099A630, 0x0099A010);
    at<float>(bot, 0x70) = 0.0f;
    at<float>(bot, 0x78) = 0.24f;
    at<float>(bot, 0x80) = 256.0f;
    at<std::uint8_t>(bot, 0x7C) = 0;
    at<float>(bot, 0x74) = -1.0f;
    return bot;
}

// 0099BE30..0099BED1. base is dereferenced without a null test (+538h, +9D4h). Constants:
// 00D7A24C = 1.0f, 00CE81A8 = 0.12f (3DF5C28Fh), 00E0E2EC = 0.24f.
void* pilot_plan_construct_0099be30(MistagHost& host, void* plan, void* base) {
    host.eh_vector_construct_00bf7cd1(plan, 0xC8, 3, 0x0099BDB0, 0x007B3BA0);
    at<float>(plan, 0x258) = 1.0f;
    at<std::uint32_t>(plan, 0x25C) = 0;
    at<void*>(plan, 0x2F0) = base;
    at<std::uint32_t>(plan, 0x2F4) = at<std::uint32_t>(base, 0x538);
    at<std::uint32_t>(plan, 0x268) = 0;
    at<std::uint32_t>(plan, 0x2F8) = at<std::uint32_t>(base, 0x9D4);
    at<std::uint32_t>(plan, 0x2E0) = 7;
    at<float>(plan, 0x2B8) = 1.0f;
    at<float>(plan, 0x260) = 0.12f;
    at<float>(plan, 0x2EC) = 0.24f;
    at<float>(plan, 0x264) = 0.0f;
    host.pilot_bot_seed_plan_slots_0099b450(plan);
    return plan;
}

// 00816B00..00816C76. The two messages live in the routine's 1B08h-byte stack frame: the
// MT_SHIP_SYNC buffer (4Ch bytes at frame-1B0Ch) and the MT_SHIP_GUNS_SYNC buffer (1AB4h).
// The sync message has no destructor call; the guns message's 64h records are destroyed.
void unit_send_ship_sync_message_00816b00(MistagHost& host, void* unit, void* holder, std::int32_t phase) {
    alignas(4) std::uint8_t sync_buffer[0x4C];
    alignas(4) std::uint8_t guns_buffer[0x1AB4];
    bool full = false;                                                    // [esp+14h] byte
    void* const state = host.ship_sync_state_get_or_create_0080e240(holder);
    at<std::int32_t>(state, 0x14) += 1;
    if (at<std::int32_t>(state, 0x14) >= phase * 3 + 1 || at<std::uint8_t>(state, 0x0F) != 0) {
        full = true;
        at<std::int32_t>(state, 0x14) = 0;
    }
    void* const sync = host.unit_state_message_build_00813ca0(sync_buffer, unit, state, phase, full);
    const std::uint32_t peer = at<std::uint32_t>(holder, 0);
    host.entity_send_message_to_peer_00779fc0(unit, peer, sync);
    host.net_entity_holder_stamp_sync_tick_00779f80(unit, holder);

    std::uint8_t damaged = 0;                                             // [esp+13h]
    bool send_guns;
    if (phase >= 5) {
        send_guns = false;                                                // 00816BE3 -> 00816C4F
    } else if (phase > 0) {
        send_guns = true;                                                 // 00816BE7 -> 00816C09
    } else {
        if (phase == 0 && at<std::uint8_t>(state, 0x18) == 0) {
            // 00816BAA..00816BDE: the first gun part (+48h list, +44h links) answering
            // IsKindOf(22h) that is damaged sets both flags.
            for (void* part = at<void*>(unit, 0x48); part != nullptr; part = at<void*>(part, 0x44)) {
                if (host.is_kind_of_5c(part, 0x22) && host.unit_is_damaged_00859c60(part)) {
                    damaged = 1;
                    at<std::uint8_t>(state, 0x19) = 1;
                    break;
                }
            }
        }
        // 00816BE9 (00816BA8 jumps straight to 00816BF5 when +18h is set).
        send_guns = at<std::uint8_t>(state, 0x18) != 0 || at<std::uint8_t>(state, 0x19) != 0;
    }
    if (send_guns) {
        const bool force = phase == 0 && at<std::uint8_t>(state, 0x18) != 0;   // 00816BF5..00816C09
        void* const guns = host.ship_guns_sync_construct_0080fcf0(guns_buffer, unit, phase, force);
        host.entity_send_message_to_peer_00779fc0(unit, at<std::uint32_t>(holder, 0), guns);
        host.eh_vector_destruct_00bf7c6e(guns_buffer + 0x24, 0x44, 0x64, 0x0075A5B0);
    }
    at<std::uint8_t>(state, 0x19) = damaged;                             // 00816C59
    at<std::uint8_t>(state, 0x18) = phase > 0 ? 1 : 0;                   // SETG at 00816C56
}

}  // namespace bsp::cc13_mistag

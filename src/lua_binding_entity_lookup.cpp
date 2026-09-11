// See include/bsp/lua_binding_entity_lookup.hpp and docs/LUA_BINDING_ENTITY_LOOKUP.md,
// docs/LUA_BINDING_GENERATE_OBJECT.md. Coverage: complete for 0088B1B0 (0088B1B0..0088B30A) and
// for the argument decode and creation selection of 00944FD0 (00945076..009453BB). The Lua object
// plumbing, the NativeString pool and the scene-database creators are contracts, not ports.
#include "bsp/lua_binding_entity_lookup.hpp"

namespace bsp {
namespace {

// The fourteen zero bytes of the table at 0088B318, read with `bsp.py ghidra bytes 0088B310 80`.
constexpr std::int32_t kSearchedKindStorage[kSearchedEntityKindCount] = {
    0x00, 0x09, 0x12, 0x13, 0x14, 0x15, 0x16, 0x3B, 0x3E, 0x3F, 0x40, 0x41, 0x44, 0x47,
};

// 00BF7FBF __stricmp, ASCII only. The CRT routine is a contract; this is the comparison rule the
// lookup needs and nothing more.
char ascii_lower(char c) noexcept {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

bool ascii_iequals(const char* a, const char* b) noexcept {
    while (*a != '\0' && *b != '\0') {
        if (ascii_lower(*a) != ascii_lower(*b)) {
            return false;
        }
        ++a;
        ++b;
    }
    return *a == *b;
}

}  // namespace

const std::int32_t kSearchedEntityKinds[kSearchedEntityKindCount] = {
    kSearchedKindStorage[0],  kSearchedKindStorage[1],  kSearchedKindStorage[2],
    kSearchedKindStorage[3],  kSearchedKindStorage[4],  kSearchedKindStorage[5],
    kSearchedKindStorage[6],  kSearchedKindStorage[7],  kSearchedKindStorage[8],
    kSearchedKindStorage[9],  kSearchedKindStorage[10], kSearchedKindStorage[11],
    kSearchedKindStorage[12], kSearchedKindStorage[13],
};

std::uint32_t world_bucket_base_offset(std::int32_t kind) noexcept {
    return kWorldBucketArrayBase +
           static_cast<std::uint32_t>(kind - kWorldBucketFirstKind) * kWorldBucketStride;
}

std::uint32_t world_bucket_head_offset(std::int32_t kind) noexcept {
    return world_bucket_base_offset(kind) + kWorldBucketHeadField;
}

bool entity_kind_is_searched(std::int32_t kind) noexcept {
    // 0088B1E0 CMP EBX,0x47 / JA is unsigned: a negative kind fails the bound and skips.
    if (kind < 0 || kind > kEntityKindJumpTableLimit) {
        return false;
    }
    for (std::size_t i = 0; i < kSearchedEntityKindCount; ++i) {
        if (kSearchedEntityKinds[i] == kind) {
            return true;
        }
    }
    return false;
}

bool mission_entity_candidate_passes_flags(const MissionEntityCandidate& candidate) noexcept {
    // 0088B213 JZ skip, 0088B221 JNZ skip, 0088B22B JNZ skip, 0088B235 JNZ skip.
    return candidate.active && !candidate.released && !candidate.release_requested &&
           !candidate.unread_gate;
}

bool mission_entity_name_matches(const char* wanted,
                                 std::int32_t wanted_length,
                                 const char* candidate_name) noexcept {
    if (wanted == nullptr) {
        // 0088B248: a null argument matches a null name, and 0088B24C..0088B25B lets an empty
        // name through as well because the measured length is zero.
        return candidate_name == nullptr || candidate_name[0] == '\0';
    }
    if (candidate_name == nullptr) {
        // 0088B29E CMP dword ptr [ESP+0x24],EAX with EAX == 0: the NativeString length field.
        return wanted_length == 0;
    }
    return ascii_iequals(wanted, candidate_name);
}

const void* mission_entity_find_by_name(const MissionEntityListView& lists,
                                        const char* wanted,
                                        std::int32_t wanted_length) noexcept {
    for (std::int32_t kind = kWorldBucketFirstKind; kind <= kWorldBucketLastKind; ++kind) {
        if (!entity_kind_is_searched(kind)) {
            continue;
        }
        const std::size_t count = lists.bucket_size(kind);
        for (std::size_t i = 0; i < count; ++i) {
            const MissionEntityCandidate candidate = lists.bucket_entry(kind, i);
            if (!mission_entity_candidate_passes_flags(candidate)) {
                continue;
            }
            if (mission_entity_name_matches(wanted, wanted_length, candidate.name)) {
                return candidate.entity;  // 0088B25F MOV ESI,[ESI+8], returned in EAX
            }
        }
    }
    return nullptr;  // 0088B2FD XOR EAX,EAX
}

GenerateObjectForm generate_object_form(std::int32_t argument_count,
                                        bool argument1_is_position_table) noexcept {
    if (argument_count > 1 && argument1_is_position_table) {
        return GenerateObjectForm::NamedPlaced;  // 00945210 MOV ESI,1
    }
    if (argument_count > 2) {
        return GenerateObjectForm::NamedPlaced;  // 00945229 MOV ESI,2
    }
    return GenerateObjectForm::NamedOnly;  // 00945223 JLE 009452E8
}

std::int32_t generate_object_position_index(bool argument1_is_position_table) noexcept {
    return argument1_is_position_table ? 1 : 2;
}

std::int32_t lua_binding_find_entity(LuaBindingEntityLookupHost& host) {
    // 00898F20/00898F2F: argument 0 read as a string. A non-string yields an empty result rather
    // than an error, because 00B662B0 is lua_tolstring.
    const std::string name = host.argument_string(0);
    const void* entity = host.find_entity_by_name(name);  // 00898F8B
    if (entity != nullptr) {
        host.push_entity_table(entity);  // 00898F98..00898FE9
    } else {
        host.push_nil();  // 0089903C
    }
    return host.result_count();  // 00899045
}

std::int32_t lua_binding_generate_object(LuaBindingEntityLookupHost& host,
                                         GenerateObjectForm* out_form) {
    const std::string name = host.argument_string(0);  // 009450A0

    // 009450C0..009450EB: argument 1 supplies the second name only when it is a string; otherwise
    // 009451C4 copies the first name into it.
    const std::int32_t count = host.argument_count();
    std::string second_name = name;
    if (count > 1 && host.argument_is_string(1)) {
        second_name = host.argument_string(1);  // 00945136
    }

    // 0094518E..009451BE: argument 1 is also tested for the three-number position table.
    const bool placed_at_one = count > 1 && host.argument_is_position_table(1);
    const GenerateObjectForm form = generate_object_form(count, placed_at_one);
    if (out_form != nullptr) {
        *out_form = form;
    }

    const void* entity = nullptr;
    if (form == GenerateObjectForm::NamedPlaced) {
        const std::int32_t position_index = generate_object_position_index(placed_at_one);
        // 0094522E: the default heading is the float at 00CE38B8; 00945248 takes the number
        // argument only when the frame actually has a slot after the position table.
        float heading = kGenerateObjectDefaultHeading;
        if (count > position_index + 1) {
            heading = host.argument_number(position_index + 1);  // 00945265
        }
        entity = host.create_placed_object(name, second_name, position_index, heading);  // 009452CA
    } else {
        entity = host.create_named_object(name, second_name);  // 00945308
    }

    host.pump_deferred_work(true);   // 00945311, MOV CL,1
    host.push_entity_table(entity);  // 00945316: no null check in the native, see the doc
    host.resolve_deferred_references();  // 009453BB
    return host.result_count();
}

}  // namespace bsp

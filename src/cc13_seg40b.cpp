// Segment 40 leftovers, packet cc13-seg40b (marker cc13_seg40b). See include/bsp/cc13_seg40b.hpp
// and docs/CC13_SEG40B.md. Not bound into bsp_game.

#include "bsp/cc13_seg40b.hpp"

namespace bsp::cc13_seg40b {

namespace {
void store_vtable(void* object, std::uint32_t vtable) noexcept {
    *static_cast<std::uint32_t*>(object) = vtable;   // MOV dword ptr [ECX], imm32
}
}  // namespace

const char* gun_bot_default_name_0072bc70(const void*) noexcept { return ""; }

bool gun_bot_default_false_0072bc90(const void*, std::uint32_t) noexcept { return false; }

float gun_bot_default_one_0072bca0(const void*) noexcept { return 1.0f; }

float gun_bot_default_one_0072bcb0(const void*) noexcept { return 1.0f; }

void gun_bot_default_noop2_0072bcc0(void*, std::uint32_t, std::uint32_t) noexcept {}

std::uint32_t gun_bot_default_zero_0072bcd0(const void*) noexcept { return 0; }

void gun_bot_default_noop1_0072bd00(void*, std::uint32_t) noexcept {}

void session_message_root_store_vtable_00729090(void* object) noexcept {
    store_vtable(object, kSessionMessageRootVtable);
}

void session_message_root_store_vtable_007290a0(void* object) noexcept {
    store_vtable(object, kSessionMessageRootVtable);
}

void session_message_root_store_vtable_007293b0(void* object) noexcept {
    store_vtable(object, kSessionMessageRootVtable);
}

void shot_command_base_store_vtable_00730b50(void* object) noexcept {
    store_vtable(object, kShotCommandBaseVtable);
}

}  // namespace bsp::cc13_seg40b

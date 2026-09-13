#include "bsp/native_input_settings_startup.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdint>
#include <cstring>
#include <initializer_list>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
void* at(void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
template<class T> T read(void* p, Word offset) noexcept {
    T value; std::memcpy(&value, at(p, offset), sizeof value); return value;
}
template<class T> void write(void* p, Word offset, T value) noexcept {
    std::memcpy(at(p, offset), &value, sizeof value);
}
void clear_vector(void* settings, Word offset) noexcept {
    for (Word delta : {4u, 8u, 12u}) write<Word>(settings, offset + delta, 0);
}
// Consumed empty-node allocation contracts69D1F0/240/290/320. Reuse the
// current CRT allocation domain; no general STL implementation or ABI export.
void initialize_tree(void* settings, Word offset, Word size, Word nil) {
    void* node = singleton_lifetime_allocate({SingletonAllocationKind::object, size, size});
    if (node) write<Word>(node, 0, 0);
    if (at(node, 4)) write<Word>(node, 4, 0);
    if (at(node, 8)) write<Word>(node, 8, 0);
    write<std::uint8_t>(node, nil - 1, 1);
    write<std::uint8_t>(node, nil, 0);
    void* tree = at(settings, offset);
    write<void*>(tree, 4, node);
    write<std::uint8_t>(node, nil, 1);
    node = read<void*>(tree, 4); write<void*>(node, 4, node);
    node = read<void*>(tree, 4); write<void*>(node, 0, node);
    node = read<void*>(tree, 4); write<void*>(node, 8, node);
    write<Word>(tree, 8, 0);
}
// Only constructor-prefix unwind, while these members are still empty.
// This is deliberately not exposed as a nonempty settings destructor.
void release_empty_tree(void* settings, Word offset) noexcept {
    void* tree = at(settings, offset);
    singleton_lifetime_free(read<void*>(tree, 4));
    write<void*>(tree, 4, nullptr); write<Word>(tree, 8, 0);
}
void run_path(NativeLuaStateStorage& owner, const char* text,
    NativeInputSettingsScriptServices& services) {
    NativeString path;
    path.assign_0041e870(services.strings, text);
    try { run_native_lua_file_00b69d40(owner, path, 0, services.strings, services.files); }
    catch (...) { destroy_native_string_header_0041dd20(&path, services.strings); throw; }
    destroy_native_string_header_0041dd20(&path, services.strings);
}
} // namespace

void* construct_native_input_settings_prefix_006ab6b0(
    void* settings, void* volatile& publication_00e198e8) {
    unsigned stage = 0;
    write<Word>(settings, 0, 0x00cf81cc);
    try {
        initialize_tree(settings, 8, 0x9c, 0x99); stage = 1;
        clear_vector(settings, 0x14); stage = 2;
        initialize_tree(settings, 0x24, 0x2c, 0x29); stage = 3;
        clear_vector(settings, 0x30); stage = 4;
        clear_vector(settings, 0x40); stage = 5;
        initialize_tree(settings, 0x54, 0x18, 0x15); stage = 6;
        initialize_tree(settings, 0x60, 0x24, 0x21); stage = 7;
        initialize_tree(settings, 0x6c, 0x9c, 0x99); stage = 8;
        construct_native_lua_state_00b66bd0(at(settings, 0x78));
        write<std::uint8_t>(settings, 4, 0);
        write<std::uint8_t>(settings, 5, 0);
    } catch (...) {
        // Prefix stops before any script can populate these members. The real
        // allocator may invoke its CRT new handler; mutations of already-built
        // member ownership from that callback are outside this empty contract.
        if (stage >= 8) release_empty_tree(settings, 0x6c);
        if (stage >= 7) release_empty_tree(settings, 0x60);
        if (stage >= 6) release_empty_tree(settings, 0x54);
        if (stage >= 5) clear_vector(settings, 0x40);
        if (stage >= 4) clear_vector(settings, 0x30);
        if (stage >= 3) release_empty_tree(settings, 0x24);
        if (stage >= 2) clear_vector(settings, 0x14);
        if (stage >= 1) release_empty_tree(settings, 8);
        publication_00e198e8 = nullptr;
        write<Word>(settings, 0, 0x00ce3818);
        throw;
    }
    return settings;
}
NativeInputSettingsScriptFrame::~NativeInputSettingsScriptFrame() noexcept { close(); }
NativeLuaStateStorage& NativeInputSettingsScriptFrame::temporary_lua() noexcept {
    return *reinterpret_cast<NativeLuaStateStorage*>(temporary_);
}
void NativeInputSettingsScriptFrame::construct_temporary() noexcept {
    construct_native_lua_state_00b66bd0(temporary_); active_ = true;
}
void NativeInputSettingsScriptFrame::close() {
    if (active_) { active_ = false; close_native_lua_state_00b669a0(temporary_lua()); }
}
NativeInputSettingsScriptPrefixResult load_native_input_settings_scripts_prefix_006a7be0(
    void* settings, NativeInputSettingsScriptServices& services, NativeInputSettingsScriptFrame& frame) {
    if (read<std::uint8_t>(settings, 4)) return NativeInputSettingsScriptPrefixResult::guard_return;
    auto& persistent = *static_cast<NativeLuaStateStorage*>(at(settings, 0x78));
    write<std::uint8_t>(settings, 4, 1);
    open_native_lua_state_00b6a020(persistent, 1, services.strings, services.bootstrap);
    run_path(persistent, "Scripts/datatables/ControlPresets.lua", services);
    frame.construct_temporary();
    try {
        open_native_lua_state_00b6a020(frame.temporary_lua(), 1, services.strings, services.bootstrap);
        run_path(frame.temporary_lua(), "Scripts\\datatables\\KeyboardSetup.lua", services);
    } catch (...) { frame.close(); throw; }
    return NativeInputSettingsScriptPrefixResult::tables_pending;
}
} // namespace bsp

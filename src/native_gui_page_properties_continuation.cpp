#include "bsp/native_gui_page_properties_continuation.hpp"
#include "bsp/native_lua_service_bindings.hpp"
#include "bsp/native_node_root_propagation.hpp"
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI Page properties tail requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
Word read_word(const void* object, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(
        static_cast<const unsigned char*>(object) + offset);
}
} // namespace

NativeGuiPagePropertiesContinuation::~NativeGuiPagePropertiesContinuation() {
    // Do not inspect possibly interrupted raw graph metadata before this guard.
    if (native_operation_interrupted()) std::terminate();
}

void continue_native_gui_page_properties_00ac6806(
    NativeGuiPageConstructionAcquired& prefix, NativeGuiPageReaderContinuation& reader_stage,
    NativeGuiPagePropertiesContinuation& frame, NativeGuiPagePropertiesContext& context) {
    const auto reader_observation = reader_stage.observation();
    if (frame.phase != NativeGuiPagePropertiesContinuation::Phase::fresh ||
        reader_observation.prefix != &prefix || !prefix.actual_page ||
        prefix.phase != NativeGuiPageConstructionAcquired::Phase::awaiting_properties_continuation ||
        prefix.continuation != 0x00ac6806u || prefix.native_eh_state != 5)
        throw std::logic_error("GUI Page tail requires its SAME normal AC6806 reader boundary");

    frame.prefix = &prefix;
    frame.reader_stage = &reader_stage;
    frame.actual_page = prefix.actual_page;
    frame.phase = NativeGuiPagePropertiesContinuation::Phase::entered;
    try {
        // Arm the complete tail guard BEFORE borrowing/reading any raw backing.
        auto pending = reader_stage.borrow_for_properties_tail();
        frame.actual_reader = &pending.reader;
        frame.actual_lua_owner = &pending.lua_owner;
        NativeLuaServiceBindings::Activation active(pending.lua_services);

        prefix.native_site = 0x00ac6806;
        frame.page_profile = read_word(frame.actual_page);
        const auto* page_table = context.properties.current_page_table(
            frame.actual_page, frame.page_profile);
        if (!page_table)
            throw std::logic_error("GUI Page tail requires its actual current Page table");
        frame.properties_target = page_table[0x18 / 4];
        prefix.native_site = 0x00ac6812;
        frame.properties_entered = true;
        context.properties.call_page_properties_18(frame.actual_page,
            frame.properties_target, pending.reader);
        frame.properties_returned = true;

        // The properties callback may have changed the current reader profile.
        prefix.native_site = 0x00ac6814;
        frame.reader_profile = pending.reader.profile_00;
        const auto* reader_table = context.properties.current_reader_table(
            pending.reader, frame.reader_profile);
        if (!reader_table)
            throw std::logic_error("GUI Page tail requires its actual current reader table");
        frame.leave_target = reader_table[2];
        if (frame.reader_profile != 0x00ce44fcu || frame.leave_target != 0x00bd7a20u)
            throw std::logic_error("GUI Page tail has no genuine current reader leave binding");
        prefix.native_site = 0x00ac681f;
        pending.reader.pop_back_00bd7130(); // BD7A20: ADD ECX,4; JMP BD7130.
        frame.leave_returned = true;

        // Load CURRENT node only AFTER both property/leave callbacks returned.
        prefix.native_site = 0x00ac6821;
        frame.captured_node = reinterpret_cast<void*>(read_word(frame.actual_page, 0x4c));
        prefix.native_site = 0x00ac6825;
        propagate_native_node_root_storage_00b6d890(frame.captured_node, nullptr, context.nodes);
        frame.root_returned = true;

        // Existing genuine raw destructor performs CE44FC, vector tidy at +4,
        // then CE374C. EH metadata is sideband; no original FH3 frame is added.
        prefix.native_site = 0x00ac683e;
        prefix.native_eh_state = 6;
        pending.reader.~NativeLuaReaderStorage();
        frame.reader_retired = true;
        prefix.native_site = 0x00ac6857;
        prefix.native_eh_state = 1;
        close_native_lua_state_00b669a0(pending.lua_owner);
        frame.lua_close_returned = true;

        prefix.native_site = 0x00ac6876;
        prefix.continuation = 0;
        prefix.phase = NativeGuiPageConstructionAcquired::Phase::constructed;
        frame.phase = NativeGuiPagePropertiesContinuation::Phase::constructed;
    } catch (...) {
        // Preserve every partial property/root/reader/Lua effect. Never replay.
        prefix.phase = NativeGuiPageConstructionAcquired::Phase::failed;
        frame.phase = NativeGuiPagePropertiesContinuation::Phase::failed;
        throw;
    }
}
} // namespace bsp

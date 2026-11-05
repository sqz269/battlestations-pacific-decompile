#include "bsp/object_handle_resolvers.hpp"
#include "bsp/gameplay_seg38_cc13.hpp"

#include <cstring>
#include <exception>
#include <stdexcept>
extern "C" {
#include <lua.h>
}

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Object handle reconstruction requires MSVC Win32 pointers.
#endif

namespace bsp {
void* object_from_handle_006ad080(std::uint32_t handle, const ObjectHandleTables& tables) noexcept {
    const auto id = static_cast<std::uint16_t>(handle);
    // bsp.py show 006AD080 / 00720FA0 / 00720FE0 --asm: all three
    // entries use the same signed split, wrapping base subtraction and
    // stride-10h entry+C load. Reuse the existing bodies with this live
    // callback's SAME aliased registry banks; no shadow ID table is built.
    // 006AD080 tests the FULL handle before truncation, whereas 00720FE0
    // tests only its word. Thus a nonzero upper word with low16 == 0 must
    // still enter the unchecked lookup, rather than become null.
    // This is C++ body reuse, not Original ECX/RET or register-ABI proof.
    if (handle == id)
        return gameplay_seg38_cc13::entity_from_message_nonzero_00720fe0(id, tables);
    return gameplay_seg38_cc13::entity_from_message_00720fa0(id, tables);
}

std::uint16_t handle_from_object_006ad0c0(const void* object) noexcept {
    if (object == nullptr) return 0;
    std::uint16_t result;
    std::memcpy(&result, static_cast<const std::byte*>(object) + 0x174, sizeof(result));
    return result;
}

void* lua_object_to_userdata_00b662d0(GuiLuaHost& host, GuiLuaRef object) {
    return host.to_userdata(object);
}

void* object_from_lua_table_00888aa0(GuiLuaHost& host, GuiLuaRef table) {
    struct Temporary {
        GuiLuaHost& host;
        GuiLuaRef object;
        ~Temporary() { host.release(object); }
    } temporary{host, host.get_by_name(table, "Ptr")};
    // Capture before the temporary's destructor; release may have side effects.
    void* result = lua_object_to_userdata_00b662d0(host, temporary.object);
    return result;
}

void set_object_handle_resolvers_00bd4fc0(ObjectHandleResolverSlots& slots,
    HandleToObjectCallback from_handle, ContextValueCallback from_table,
    ObjectToHandleCallback from_object) noexcept {
    slots.handle_to_object = from_handle;
    slots.context_value = from_table;
    slots.object_to_handle = from_object;
}

void install_object_handle_resolvers_006ad0d0(ObjectHandleResolverSlots& slots) noexcept {
    set_object_handle_resolvers_00bd4fc0(slots, object_from_handle_006ad080,
        object_from_lua_table_00888aa0, handle_from_object_006ad0c0);
}

namespace {
std::int32_t pointer_bits(void* object) noexcept {
    std::int32_t bits;
    static_assert(sizeof(bits) == sizeof(object));
    std::memcpy(&bits, &object, sizeof(bits));
    return bits;
}
}
std::int32_t ObjectHandleReaderResolver::resolve_by_number(std::int32_t id) {
    return pointer_bits(slots_.handle_to_object(static_cast<std::uint32_t>(id), tables_));
}
std::int32_t ObjectHandleReaderResolver::resolve_by_table(const GuiTable*) {
    throw std::invalid_argument("Object Handle resolution requires a live Lua table with native userdata");
}
std::int32_t ObjectHandleReaderResolver::resolve_by_live_table(GuiLuaHost& host, GuiLuaRef table) {
    return pointer_bits(slots_.context_value(host, table));
}

namespace {
// Source routing only. The pointed-to invocation is retained by the caller,
// so a skipped C++ destructor cannot leave TLS pointing at an expired scope.
thread_local NativeLuaFieldResolverFrame* active_native_field_resolver{};
}

NativeLuaFieldResolverFrame::~NativeLuaFieldResolverFrame() noexcept {
    if (active_ || phase == Phase::entered || phase == Phase::failed) std::terminate();
}

NativeLuaFieldResolverAdapter::NativeLuaFieldResolverAdapter(
    const ObjectHandleResolverSlots& slots, ObjectHandleTables tables, const char* ptr_literal)
    : slots_(slots), tables_(tables), ptr_literal_(ptr_literal) {
    if (!slots_.handle_to_object || slots_.context_value != &object_from_lua_table_00888aa0 ||
        !ptr_literal_ || std::strcmp(ptr_literal_, "Ptr") != 0)
        throw std::invalid_argument("raw field resolvers require installed source callbacks and the genuine Ptr literal");
}

NativeLuaFieldResolverAdapter::~NativeLuaFieldResolverAdapter() noexcept {
    if (requires_process_retention()) std::terminate();
}

bool NativeLuaFieldResolverAdapter::requires_process_retention() const noexcept {
    return failed_ || active_scopes_ != 0;
}

NativeLuaFieldResolverAdapter::Activation::Activation(
    NativeLuaFieldResolverAdapter& binding, NativeLuaFieldResolverFrame& frame) : frame_(frame) {
    if (binding.failed_ || frame.active_ || frame.phase == NativeLuaFieldResolverFrame::Phase::entered ||
        frame.phase == NativeLuaFieldResolverFrame::Phase::failed)
        throw std::logic_error("raw field resolver activation requires quiescent retained backing");
    frame.binding_ = &binding;
    frame.previous_ = active_native_field_resolver;
    frame.active_ = true;
    ++binding.active_scopes_;
    active_native_field_resolver = &frame;
}

NativeLuaFieldResolverAdapter::Activation::~Activation() noexcept {
    if (active_native_field_resolver != &frame_ || !frame_.active_ ||
        !frame_.binding_ || !frame_.binding_->active_scopes_)
        std::terminate();
    active_native_field_resolver = frame_.previous_;
    --frame_.binding_->active_scopes_;
    frame_.active_ = false;
    frame_.binding_ = nullptr;
    frame_.previous_ = nullptr;
}

NativeLuaFieldResolverFrame& NativeLuaFieldResolverAdapter::active() {
    auto* const frame = active_native_field_resolver;
    if (!frame || !frame->active_ || !frame->binding_ || frame->binding_->failed_ ||
        frame->phase == NativeLuaFieldResolverFrame::Phase::entered ||
        frame->phase == NativeLuaFieldResolverFrame::Phase::failed)
        throw std::logic_error("raw field resolver call has no live serialized activation");
    return *frame;
}

void NativeLuaFieldResolverAdapter::fail(NativeLuaFieldResolverFrame& frame) noexcept {
    frame.phase = NativeLuaFieldResolverFrame::Phase::failed;
    frame.binding_->failed_ = true;
}

std::uint32_t __fastcall NativeLuaFieldResolverAdapter::resolve_number(std::int32_t value) {
    auto& frame = active();
    auto& binding = *frame.binding_;
    frame.phase = NativeLuaFieldResolverFrame::Phase::entered;
    frame.active_site = 0x006ad080;
    try {
        // Native ECX contains the FULL word; only 006AD080 truncates to16 bits.
        // Read the current source cell once at this invocation, then call its
        // real explicit-context signature. No mismatched function-pointer cast.
        frame.loaded_number = binding.slots_.handle_to_object;
        if (!frame.loaded_number)
            throw std::logic_error("raw number resolver source callback is not installed");
        void* const result = frame.loaded_number(static_cast<std::uint32_t>(value), binding.tables_);
        frame.result_bits = reinterpret_cast<std::uint32_t>(result);
        frame.phase = NativeLuaFieldResolverFrame::Phase::complete;
        return frame.result_bits; // Original EAX pointer bits, plain RET.
    } catch (...) { fail(frame); throw; }
}

std::uint32_t __fastcall NativeLuaFieldResolverAdapter::resolve_table(NativeLuaObjectStorage* input) {
    auto& frame = active();
    auto& binding = *frame.binding_;
    frame.phase = NativeLuaFieldResolverFrame::Phase::entered;
    frame.active_site = 0x00888aa0;
    try {
        frame.loaded_table = binding.slots_.context_value;
        if (frame.loaded_table != &object_from_lua_table_00888aa0)
            throw std::logic_error("raw table resolver admits only the installed 00888AA0 source target");
        if (!input || !input->owner_00 || !input->owner_00->state_04)
            throw std::invalid_argument("raw table resolver requires the actual live LuaObject");
        frame.active_site = 0x00888ac3;
        native_lua_get_by_name_00b67800(*input, &frame.temporary, binding.ptr_literal_);
        // B662D0 loads CURRENT owner+4/index+8 then tail-calls lua_touserdata.
        frame.active_site = 0x00888ad2;
        void* const result = lua_touserdata(frame.temporary.owner_00->state_04, frame.temporary.index_08);
        const auto result_bits = reinterpret_cast<std::uint32_t>(result); // ESI before B67700.
        frame.result_bits = result_bits;
        frame.active_site = 0x00888ae5;
        destroy_native_lua_object_00b67700(frame.temporary);
        frame.active_site = 0x00888afb;
        frame.result_bits = result_bits;
        frame.phase = NativeLuaFieldResolverFrame::Phase::complete;
        return result_bits;
    } catch (...) { fail(frame); throw; }
}
} // namespace bsp

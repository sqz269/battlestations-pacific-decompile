#include "bsp/native_sampler_parameter_write.hpp"
#include "bsp/native_lua_string_field.hpp"

#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using Operation = NativeSamplerParameterWriteOperation;

// A second source C++ failure during the native unwind obligation terminates;
// this does not implement original FH3 or make Lua longjmp into C++ exceptions.
void destroy_unwind_globals(Operation& operation) noexcept {
    operation.native_site = 0x00cbc7d0;
    destroy_native_lua_object_00b67700(operation.globals);
    operation.globals_obligation = false;
}

__declspec(noinline) void __cdecl parameter_body(
    void* receiver, NativeSamplerParameterWriteContext* context,
    const void* const volatile* original_key_slot,
    const void* const volatile* original_value_slot) {
    Operation& operation = context->operation;
    if (operation.phase != Operation::Phase::fresh)
        throw std::logic_error("sampler parameter write is one-shot");
    operation.phase = Operation::Phase::running;
    try {
        operation.native_site = 0x00b1b84f;
        auto* const owner = reinterpret_cast<NativeLuaStateStorage*>(
            static_cast<char*>(receiver) + 4);
        auto* const object = native_lua_globals_00b67980(*owner, &operation.globals);
        operation.globals_obligation = true;

        // Native B1B854 reads VALUE before B1B858 reads KEY, both after the
        // globals constructor. Keep the original public slots, not copies.
        const void* const value = *original_value_slot;
        const void* const key = *original_key_slot;
        operation.cleanup_armed = true;
        operation.native_site = 0x00b1b868;
        set_native_lua_string_field_00b674c0(
            object, context->actual_empty_0108ff2c, key, value);

        operation.cleanup_armed = false;
        operation.native_site = 0x00b1b878;
        destroy_native_lua_object_00b67700(operation.globals);
        operation.globals_obligation = false;
        operation.phase = Operation::Phase::complete;
    } catch (...) {
        if (operation.cleanup_armed) {
            operation.cleanup_armed = false;
            destroy_unwind_globals(operation);
        }
        operation.phase = Operation::Phase::failed;
        throw;
    }
}
} // namespace

NativeSamplerParameterWriteOperation::~NativeSamplerParameterWriteOperation() {
    if (phase == Phase::running || phase == Phase::failed || globals_obligation)
        std::terminate();
}

void NativeSamplerParameterWriteOperation::acknowledge_diagnostic_cleanup() noexcept {
    if (phase != Phase::failed || globals_obligation || cleanup_armed)
        std::terminate();
    phase = Phase::diagnostic_retired;
}

__declspec(naked) void __fastcall set_native_sampler_parameter_00b1b830(
    void*, NativeSamplerParameterWriteContext&, const void*, const void*) {
    __asm {
        lea eax, [esp + 8]
        push eax
        lea eax, [esp + 8]
        push eax
        push edx
        push ecx
        call parameter_body
        add esp, 16
        ret 8
    }
}
} // namespace bsp

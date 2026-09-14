#include "bsp/native_sampler_parameter_record.hpp"
#include "bsp/native_lua_field_setters.hpp"
#include "bsp/native_lua_numeric_element.hpp"

#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native sampler record parameters require MSVC Win32.
#endif

namespace bsp {
namespace {
using Operation = NativeSamplerParameterRecordOperation;
static_assert(offsetof(Operation, globals) == offsetof(Operation, result) + 0x14);

// Each original argument is FLD32/FSTP32 immediately before its numeric
// write. Keep this real x87 operation and the next component's late read.
__declspec(noinline) void write_component(NativeLuaObjectStorage* result,
    const void* component, std::int32_t index) {
    float value;
    __asm {
        mov eax, component
        fld dword ptr [eax]
        fstp dword ptr [value]
    }
    set_native_lua_numeric_element_00b66580(result, nullptr, index, value);
}

// Only states0/1/3 are published by the normal body. The original map also
// contains state2, but there is no corresponding normal source transition.
// A second C++ failure during unwind terminates; this is not native FH3.
void destroy_armed_local(Operation& operation) noexcept {
    const auto state = operation.native_state;
    operation.native_state = -1;
    if (state == 0 || state == 1) {
        operation.native_site = state == 0 ? 0x00cbc7f0 : 0x00cbc7f8;
        destroy_native_lua_object_00b67700(operation.globals);
        operation.globals_obligation = false;
    } else if (state == 3) {
        operation.native_site = 0x00cbc800;
        destroy_native_lua_object_00b67700(operation.result);
        operation.result_obligation = false;
    } else if (state != -1) {
        std::terminate();
    }
}

__declspec(noinline) void __cdecl record_body(void* receiver, Operation* frame,
    const NativeString* const volatile* original_key_slot,
    const void* const volatile* original_record_slot) {
    Operation& operation = *frame;
    if (operation.phase != Operation::Phase::fresh)
        throw std::logic_error("sampler record parameter write is one-shot");
    operation.phase = Operation::Phase::running;
    try {
        auto* const owner = reinterpret_cast<NativeLuaStateStorage*>(
            static_cast<char*>(receiver) + 4);
        operation.native_site = 0x00b1b8b4;
        auto* globals = native_lua_globals_00b67980(*owner, &operation.globals);
        operation.globals_obligation = true;

        // B1B8B9: this public slot is captured once, after construction.
        const NativeString* const key = *original_key_slot;
        operation.native_state = 0;
        operation.native_site = 0x00b1b8c8;
        native_lua_set_new_table_00b67580(*globals, *key);

        operation.native_state = -1;
        operation.native_site = 0x00b1b8d9;
        destroy_native_lua_object_00b67700(operation.globals);
        operation.globals_obligation = false;

        operation.native_site = 0x00b1b8e5;
        globals = native_lua_globals_00b67980(*owner, &operation.globals);
        operation.globals_obligation = true;
        operation.native_state = 1;
        operation.native_site = 0x00b1b8fa;
        native_lua_get_by_string_00b68100(*globals, &operation.result, *key);
        operation.result_obligation = true;

        // B1B903: result-only unwind is armed before the globals destructor.
        operation.native_state = 3;
        operation.native_site = 0x00b1b908;
        destroy_native_lua_object_00b67700(operation.globals);
        operation.globals_obligation = false;

        // B1B90D: capture the original record pointer only after destruction.
        const auto* const record = static_cast<const unsigned char*>(*original_record_slot);
        operation.native_site = 0x00b1b91d;
        write_component(&operation.result, record, 1);
        operation.native_site = 0x00b1b92f;
        write_component(&operation.result, record + 4, 2);
        operation.native_site = 0x00b1b941;
        write_component(&operation.result, record + 8, 3);
        operation.native_site = 0x00b1b953;
        write_component(&operation.result, record + 12, 4);

        operation.native_state = -1;
        operation.native_site = 0x00b1b964;
        destroy_native_lua_object_00b67700(operation.result);
        operation.result_obligation = false;
        operation.phase = Operation::Phase::complete;
    } catch (...) {
        destroy_armed_local(operation);
        operation.phase = Operation::Phase::failed;
        throw;
    }
}
} // namespace

NativeSamplerParameterRecordOperation::~NativeSamplerParameterRecordOperation() {
    if (phase == Phase::running || phase == Phase::failed || result_obligation ||
        globals_obligation || native_state != -1)
        std::terminate();
}

void NativeSamplerParameterRecordOperation::acknowledge_diagnostic_cleanup() noexcept {
    if (phase != Phase::failed || result_obligation || globals_obligation || native_state != -1)
        std::terminate();
    phase = Phase::diagnostic_retired;
}

__declspec(naked) void __fastcall set_native_sampler_parameter_record_00b1b890(
    void*, NativeSamplerParameterRecordOperation&, const NativeString*, const void*) {
    __asm {
        lea eax, [esp + 8]
        push eax
        lea eax, [esp + 8]
        push eax
        push edx
        push ecx
        call record_body
        add esp, 16
        ret 8
    }
}
} // namespace bsp

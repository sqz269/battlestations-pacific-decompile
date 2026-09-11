#pragma once

#include <cstdint>

namespace bsp {
struct NativeCrtPointerDecodeSupportContext;

using NativeCrtDecodeTlsGetValue = void* (__stdcall*)(std::uint32_t);

// Four stable borrowed reference bindings. The import reference is the actual
// current TlsGetValue IAT word corresponding to CE20BC, not a stored function
// copy, thunk address or generic replaceable provider. Current TLS indices,
// getter values and PTD/decoder storage remain owned by their real CRT/OS.
// No TLS creation, publication, pointer encoding or provider fallback is added.
struct NativeCrtPointerDecodeContext {
    const volatile std::uint32_t& getter_tls_index_00e15b00;
    const volatile std::uint32_t& ptd_index_00e15afc;
    NativeCrtDecodeTlsGetValue const volatile& tls_get_value_iat_00ce20bc;
    const NativeCrtPointerDecodeSupportContext& module_gate;
};

// Full C04FDE[110], qualified naked Win32 source ABI. At entry ESP+4 holds
// the original pointer argument itself; ESP+8 holds the added context address.
// After the original ESI save, only an invoked nonnull selected decoder writes
// [ESP+8] (the caller's first argument word), then every path reloads that word
// into EAX. Identity paths never write it. Plain RET; caller cleans both args.
// Callers that observe this write use the actual outgoing stack word; passing
// a normal C++ variable by value does not make that variable the native slot.
void* __cdecl native_crt_decode_pointer_00c04fde(
    void* original_argument, const NativeCrtPointerDecodeContext&);

// The added context argument word and its four bindings stay valid/stable.
// Every selected current TLS getter and DecodePointer entry must be the actual
// established stdcall provider. The second TLS-returned getter is invoked
// without an extra null check. A returned nonnull PTD requires a readable
// 200h-byte prefix; current +1FC may be null (identity, no module fallback).
// Module fallback uses actual GetModuleHandleA/GetProcAddress and the complete
// existing module gate with its owning CRT context. No LastError restoration.
// Additional context loads/new code addresses and the gate's C++ frame do not
// reproduce original volatile-register/fault-site/native SEH continuation ABI.
// No exception translation or cleanup; startup/libm/SSE2 closure is separate.
} // namespace bsp

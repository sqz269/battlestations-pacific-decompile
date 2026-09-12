#pragma once

#include "bsp/native_crt_pointer_decode_support.hpp"

namespace bsp {

// Actual OS TlsGetValue/FlsGetValue signature; the binding below is the real
// current TlsGetValue IAT word, not a generic getter/provider injection point.
using NativeCrtEncodeTlsGetter = void* (__stdcall*)(std::uint32_t);

struct NativeCrtPointerEncodeContext {
    const volatile std::uint32_t& getter_tls_index_00e15b00;
    const volatile std::uint32_t& ptd_index_00e15afc;
    NativeCrtEncodeTlsGetter const volatile& tls_get_value_import_00ce20bc;
    const NativeCrtPointerDecodeSupportContext& module_gate;
};

// Complete C04F67[110]. Original cdecl pointer word at entryESP+4, EAX pointer,
// plain RET. This NEW interface adds a stable context reference at entryESP+8;
// caller cleans both arguments. The first argument remains actual callee
// argument storage: a selected encoder writes its result to that same slot,
// then the entry reloads it; identity paths do not write it. No float adapter
// or return-value-only substitution. Assembly callers may observe that slot.
//
// Context/binding identities and the extra argument word remain valid/stable
// through every reached provider; their referenced native words remain mutable.
// First current E15B00 is pushed BEFORE the real current CE20BC IAT is captured
// into ESI. That captured import is reused for the second current-index query.
// The newly returned raw getter is called unchecked with captured E15AFC. A
// nonnull actual PTD provides its current +1F8 encoder; at least1FCh readable
// bytes are required. A null encoder in an existing PTD means identity, without
// fallback. Other null/missing-index paths use actual GetModuleHandleA on the
// exact KERNEL32.DLL name, full C04EFB gate, and actual GetProcAddress for
// EncodePointer. Selected encoder has actual stdcall void*(void*) ABI.
//
// Caller owns coherent OS TLS/FLS/PTD state, real IAT binding, module/code and
// all reached storage lifetimes. No TLS allocation, private PTD, LoadLibrary,
// generic callback, decoder cache update, validation or LastError restoration.
// Full existing module-gate/owning-CRT source domains apply. Extra context and
// gate bridge are new source plumbing: no native caller/SEH/fault-site/private
// frame equivalence, incidental volatile-register/EFLAGS identity, arbitrary
// provider ABI or runtime proof is claimed.
void* __cdecl native_crt_encode_pointer_00c04f67(
    void* argument, const NativeCrtPointerEncodeContext& context);

struct NativeCrtLibmCallbackRegistrationContext {
    const NativeCrtPointerEncodeContext& encoder;
    volatile std::uint32_t& callback_present_0109e1b8;
    volatile std::uint32_t& encoded_callback_0109ed80;
};

// Complete uncreated original C0F0BB[41] registration body. Original cdecl one
// raw callback word, RET, no semantic result. New interface adds context as the
// second caller-cleaned word. A real callback represents callable code with
// int __cdecl(CameraAxesCrtException*) ABI, mutable32-byte exception record,
// proper nonvolatile/x87 conventions and lifetime through every captured use.
// This entry only publishes its encoded pointer; it never calls the callback.
//
// Compare the current first argument slot; null performs actual DWORD AND0 RMW
// on the flag, leaving encoded value stale and skipping encoder binding/value
// reads. Nonnull rereads the actual argument and invokes the full encoder using
// a real temporary callee argument slot, then publishes encoded EAX BEFORE1 to
// the flag. No pre-clear, lock, decoded-null guard, rollback, callback ownership,
// quiescence or retirement is added. Disabling does not stop captured callbacks.
// Stable context/extra argument identities are required; underlying words may
// change through actual providers. No currently registered body/lifetime or
// whole CRT startup construction is inferred from this source.
void __cdecl register_native_crt_libm_callback_00c0f0bb(
    void* actual_callback, const NativeCrtLibmCallbackRegistrationContext& context);

} // namespace bsp

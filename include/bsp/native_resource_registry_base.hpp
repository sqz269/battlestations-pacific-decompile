#pragma once

#include <cstdint>

namespace bsp {

// Complete B197E0[9]. Native ECX=this, EAX=this, plain RET. The new fastcall
// declaration uses exactly that no-stack-argument shape. Writes only the
// original D5E550 profile DWORD; it does not construct a tree or publication.
void* __fastcall construct_native_resource_registry_base_00b197e0(void* owner);

// Complete B197F0[41]. ECX=this; current flags DWORD is at entry ESP+4;
// EAX=captured this, RET4. The new source ABI adds EDX pointing directly to
// the actual mutable F8D41C publication cell (not a copied pointer value).
// That cell and the reached owner/free storage must remain valid.
//
// Test bit0 of the LOW BYTE of the actual flags slot before either write.
// Clear the borrowed publication, install CE3818 on captured this, then free
// that captured allocation only if the original test was nonzero. Neither
// MOV changes flags. Preserve the returning-free ADD ESP,4 and return the
// captured pointer bits, including after free; do not dereference that result.
//
// This base leaf has no tree destruction, reset call or EH cleanup. Actual
// singleton_lifetime_free supplies its existing Win32/CRT contract. Native
// provider volatile-register/fault identity, original-caller ABI without the
// extra EDX binding, native SEH and game behavior are not established here.
void* __fastcall delete_native_resource_registry_base_00b197f0(
    void* owner, void* volatile* actual_publication_00f8d41c,
    std::uint32_t flags);

} // namespace bsp

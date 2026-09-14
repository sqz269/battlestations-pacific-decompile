#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native spatial-index publication requires MSVC Win32.
#endif

namespace bsp {

// Complete 0042D450[62]. Native ECX actual fresh 16018h-byte allocation,
// EDX unconsumed, EAX self, plain RET. Writes profile CE3CEC and +4, clears
// only the 15F90h-byte grid at +84, then clears count +80 and root +16014.
// The thirty loose slots +8..+7C retain their previous bytes. The profile
// is a native identity DWORD; it is not a rebuilt callable virtual table.
void* __fastcall construct_native_spatial_index_0042d450(
    void* actual_storage, void* unused_edx) noexcept;

// Complete 0042E630[174]. New source ABI with distinct, stable references
// to the actual raw manager (01090AA0) and index (F8A0D8) publication cells.
// Native takes no arguments, returns EAX, plain RET. Uses the existing raw
// manager, allocation, registration and Win32 lock providers. Never pass a
// typed SingletonLifetimeDomain owner or create a second publication domain.
// The first manager's section +10 is captured across both later lookups and
// registration. Guard cleanup starts after Enter and physical +18 increment;
// there is no allocation cleanup state. Publication survives registration
// failure. The slow return reloads publication after releasing the section.
// Native FH3/SEH stack identity and fault delivery are not reproduced.
void* get_native_spatial_index_0042e630(
    void* volatile& actual_manager_publication_01090aa0,
    void* volatile& actual_index_publication_00f8a0d8);

} // namespace bsp

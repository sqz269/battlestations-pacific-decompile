#pragma once

#include <cstdint>

namespace bsp {

// Eight DISTINCT complete derived-plane vtable+5Ch bodies, 482B/172 instructions.
// Original ECX is the actual receiver; EDX is unused; one stacked DWORD; full
// EAX0/1 and RET4. Fixed ordered words precede ONE fresh actual DWORD+C4 read.
// These raw Win32 entries preserve the original bytes, not a class-ID census.
// Admit stable nonnull actual backing >=C8h with a genuinely live, aligned
// uint32 cell at +C4h. Fixed-word paths do not read the receiver; this common
// admission covers the dynamic path. No guards/defaults/caches/constructor or
// ownership policy are added. Invalid/fault/concurrent-lifetime paths unbound.
// Raw profile DATA does not supply a callable Source class or native world.
// Numeric names retain provisional descriptor assignments for 11h/12h/17h.
// Base0074E400 is a separate provider; no larger caller/class/profile/arena,
// private-EH, virtual admission, InitAll, observer or gameplay closure claimed.

std::uint32_t __fastcall native_plane_derived_is_kind_007d77f0(const void*, void*, std::uint32_t);
std::uint32_t __fastcall native_plane_derived_is_kind_009535c0(const void*, void*, std::uint32_t);
std::uint32_t __fastcall native_plane_derived_is_kind_00953530(const void*, void*, std::uint32_t);
std::uint32_t __fastcall native_plane_derived_is_kind_007dda80(const void*, void*, std::uint32_t);
std::uint32_t __fastcall native_plane_derived_is_kind_0074e480(const void*, void*, std::uint32_t);
std::uint32_t __fastcall native_plane_derived_is_kind_0084c9f0(const void*, void*, std::uint32_t);
std::uint32_t __fastcall native_plane_derived_is_kind_0074e4e0(const void*, void*, std::uint32_t);
std::uint32_t __fastcall native_plane_derived_is_kind_009534a0(const void*, void*, std::uint32_t);

} // namespace bsp

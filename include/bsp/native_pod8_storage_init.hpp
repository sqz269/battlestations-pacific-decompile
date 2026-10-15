#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native POD8 storage entries require MSVC Win32.
#endif

namespace bsp {

// Complete raw 00418700..0041870D,14B. Original stack count, RET4/EAX raw
// allocation; no consumed receiver. Load the actual count cell into ECX, zero EDX and
// call the accepted full13390 allocator, including its count0/failure policy.
// Primary created this reviewed14B Ghidra function after worker recovery;
// no Original incoming caller/reference was found. See the batch evidence.
void* __stdcall allocate_native_pod8_storage_00418700(std::uint32_t count);

// Complete raw00420AC0..00420B06,71B. ECX actual10h header plus one stacked
// uint32 count, RET4. Capture count before header writes, preserve proxy0,
// zero4/8/C first. Count0 returns AL0 without allocation; nonzero checks the
// unsigned1FFFFFFF bound, calls actual203A0/13390 providers, then publishes
// first/last and wrapped first+count*8 end. AL1 on normal nonzero return;
// upper EAX retains endpoint bits except low byte, not a DWORD bool result.
// Source EDX is an unused physical-binding placeholder. No free/rollback/
// normalization or null-return recovery is added. Primary created the reviewed
//71B Ghidra function; no Original incoming direct/literal reference was found.
bool __fastcall initialize_native_pod8_storage_00420ac0(
    void* actual_header, void* unused_source_edx, std::uint32_t count);

// Complete00418710..00418734,37B. ECX actual left8B iterator, one stacked
// right8B iterator pointer, RET4/EAX signedSAR3 difference. Capture both
// iterator addresses; validate nonnull left.owner and equality with right.owner
// through actual returning invalid policy. After it returns, reload CURRENT
// left/right position words, subtract moduloDWORD then SAR3. No owner recheck,
// argument-pointer reload, numeric range repair or private iterator is added.
// Source EDX is an unused binding placeholder. No current caller is known.
std::int32_t __fastcall difference_native_pod8_iterator_00418710(
    const void* actual_left, void* unused_source_edx, const void* actual_right);

// Literal Win32 bodies retain actual stack cells, flags, raw access order and
// cleanup; only fixed provider CALL operands change. Current Source allocation,
// owning exception/catch, invalid-handler and C++ frame identities are qualified.
// Every reached backing/control/provider cell must be valid. No original class,
// Native EH/SEH/runtime, installed replacement or gameplay claim is made.
// Evidence: docs/GOAL15H_POD8_STORAGE_INIT.md.

} // namespace bsp

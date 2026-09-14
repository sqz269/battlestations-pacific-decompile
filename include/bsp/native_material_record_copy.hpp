#pragma once

#include "bsp/native_string.hpp"

#include <cstdint>

namespace bsp {

// Complete 00B13070 copy-constructor over an actual 300-byte renderer/material
// record. Original: ECX destination, stacked source, EAX destination, RET4;
// preserves EBX, EBP, ESI and EDI. The source interface keeps ECX/stack/RET4
// and adds the existing raw string-pool context in EDX. Destination must be
// fresh actual storage; both records and their reached raw headers must remain
// live. The prefix uses five separate forward REP MOVSD blocks under the native
// clear-DF calling convention, preserving overlap and alias effects.
void* __fastcall construct_native_material_record_00b13070(
    void* actual_destination, NativeStringRawPoolContext& raw_strings,
    const void* actual_source);

// Complete 00B0D230 leaf: original ECX actual vector, EAX signed record count,
// plain RET. The +4/+8 begin/end DWORDs are subtracted with 32-bit wrap, then
// interpreted signed and divided by the 300-byte record stride. A null begin
// returns zero. No order, extent or pointer validation is performed.
std::int32_t __fastcall get_native_material_record_vector_size_00b0d230(
    const void* actual_vector) noexcept;

// The constructor's extra EDX context and C++ EH frame make these new source
// interfaces, not whole-program binary replacements. Native hardware faults
// and private FH3 frame identity are outside the source compatibility claim.

} // namespace bsp

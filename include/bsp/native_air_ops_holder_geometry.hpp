#pragma once

namespace bsp {

// Complete 006BCC90..006BCCD9 ordinary body. Native ECX=actual holder,
// stack(destination XYZ, source XYZ), EAX=destination, RET8. EDX is unused
// in this Win32 fastcall declaration. No copied holder/pose, lifetime or provider.
// Reads the actual matrix at holder+48h through complete 004142E0 into local
// scratch, then ordered x87 FLD/FSUB/FSTP against FRESH holder+A4/A8/AC.
// Valid ordinary storage is required; output may alias these translation cells.
// Preserves ambient x87 state/rounding and staged input/ordered output effects.
// Masked-exception component comparison admits at least five free x87 slots.
// No original executable binding, private fault/EH or gameplay claim.
void* __fastcall native_air_ops_holder_local_less_t_006bcc90(
    const void* actual_holder, void* unused_edx, void* destination_xyz,
    const void* source_xyz);

} // namespace bsp

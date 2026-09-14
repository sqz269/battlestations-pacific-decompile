#pragma once
#include <cstdint>

namespace bsp {
// Source storage provider for0087B460's resize/end-insertion use. Actual10h
// header: untouched opaque0, begin4, end8, capacity-endC; raw DWORD pointers.
// Preserve existing elements, fill added entries, retain capacity on shrink,
// and publish capacity/end/begin after replacement allocation and old free.
// Uses existing shared checked-DWORD mechanics and allocation/exception domain.
// Valid consistently owned storage is required; allocation callbacks may throw
// but must not structurally mutate the vector. Original STL/FH3 ABI, malformed
// range continuation, private-stack aliases and fault timing are not supplied.
// Native ECX=header, stack(unsigned count, DWORD fill), RET8; new C++ interface.
void resize_native_unit_part_pointers_0087b460(void* actual_header,
    std::uint32_t count, std::uint32_t fill);
} // namespace bsp

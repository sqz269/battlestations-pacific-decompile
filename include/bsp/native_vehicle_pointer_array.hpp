#pragma once

#include <cstdint>

namespace bsp {

// Required dispatch for the CURRENT referent table's slot0. The implementation
// must execute the captured target with ECX=referent, EDX=table, no stack args.
// Numeric original profiles require a reconstructed terminal implementation.
class NativeVehiclePointerArrayCalls {
public:
    virtual ~NativeVehiclePointerArrayCalls() = default;
    virtual void zero_reference(std::uint32_t target, void* referent,
                                std::uint32_t captured_table) = 0;
};

// Complete ordinary bodies 00546630[244] and 005471B0[126]. Original ABI:
// ECX={data,count,capacity}, one signed stack argument, RET4, no stable result.
// Storage is borrowed Win32 DWORD storage. Each nonnull element has an aligned
// reference LONG at +4 and a current method table at +0. Arithmetic wraps in
// DWORDs; signed comparisons and current-header reloads follow the listing.
// Reserve clamps to >=1, retains copies, releases old slots, frees old storage,
// then publishes data and capacity. Resize zeroes growth and releases shrinkage
// from the back, publishing the decremented count before each release.
void reserve_native_vehicle_pointer_array_00546630(
    void* actual_header, std::int32_t requested, NativeVehiclePointerArrayCalls&);
void resize_native_vehicle_pointer_array_005471b0(
    void* actual_header, std::int32_t requested, NativeVehiclePointerArrayCalls&);

// New source ABI; allocation uses the existing singleton source-CRT boundary.
// No native FH3/SEH frame, hardware-fault recovery or cleanup rollback is added.
// A throwing terminal skips the captured slot clear and final publication.
// Callers supply valid storage for every reached native access; negative sizes
// and overflow are not normalized beyond reserve's original minimum clamp.
} // namespace bsp

#pragma once
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace bsp {
// 70BD70's actual 84h owner. These bytes are deliberately uninitialized.
// Grid is a descriptive hypothesis; the enclosing simulation is not recovered.
struct alignas(4) NativeGameGridStorage { std::byte bytes[0x84]; };
static_assert(sizeof(NativeGameGridStorage)==0x84);
static_assert(std::is_trivially_default_constructible_v<NativeGameGridStorage>);

struct NativeGameGridCpuCalls {
    virtual ~NativeGameGridCpuCalls()=default;
    // BF6989 is the game's CRT free, not a six-array ownership abstraction.
    // Source allocations must belong to the matching source CRT domain.
    virtual void free_00bf6989(void* allocation);
};

// Complete 709C90: native ECX=destination38h, stack=source38h, EAX=destination,
// RET4. First two DWORDs are bit copies; the next twelve use sequential x87
// FLD/FSTP32 pairs, including signaling-NaN quieting. Forward overlap is live.
// This explicit two-pointer source interface is not the original binary ABI.
void* copy_native_game_grid_descriptor_00709c90(void* destination,const void* source);

// Complete 709CF0: native ECX=84h owner, RET; incidental EAX is not exposed.
// Dimensions +3C/+40, positions +08, normals +1C. Interior central differences
// spill to binary32 before the native x87 cross-product stack schedule, then
// reuse the existing 42B260 normalization body. Boundary copies are x87 too.
// No dimension, alias, allocation or finite-number checks are added. The caller
// must provide every address the original would access (including edge cases).
void calculate_native_game_grid_normals_00709cf0(void* actual_grid);

// Complete 709760: current nonnull pointers +08,+0C,+10,+14,+18,+1C are freed
// in order; all six fields are zeroed only AFTER every free has returned.
// Callback changes to later pointers are observed. Exceptions retain partial
// effects; no catch, rollback, individual early clearing or replay is added.
void release_native_game_grid_cpu_arrays_00709760(void* actual_grid,NativeGameGridCpuCalls&);
} // namespace bsp

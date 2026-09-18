#pragma once
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include "bsp/gui_text_native_renderer.hpp"
#include "bsp/native_string.hpp"

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
    virtual void* allocate_00bf55be(std::uint32_t bytes);
    virtual void free_00bf65ac(void* allocation);
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

struct NativeGameGridConstants {
    const volatile std::uint32_t& bits_00d7a24c;
    const volatile std::uint32_t& bits_00cfd470;
    const volatile std::uint32_t& bits_00d7a260;
    const volatile std::uint32_t& bits_00ce5380;
    const volatile std::uint32_t& bits_00ce397c;
    const volatile std::uint32_t& bits_00cfd46c;
    const volatile std::uint32_t& bits_00f87574;
    const volatile std::uint32_t& bits_00f87578;
    const volatile std::uint32_t& bits_00f8757c;
    const volatile double& value_00ce3d28;
    const volatile double& value_00d7a3a0;
    const char* literal_00cfd4e4; // actual eleven bytes, "gunvc.mvfm" plus NUL
};
// The constructor leaves these two fresh descriptor words unwritten, then
// 709C90 copies them to +5C/+60 before six observable allocator calls. Supply
// their preimage explicitly; zero is not an inferred native default.
struct NativeGameGridDescriptorPreimage { std::uint32_t word20,word24; };
struct NativeGameGridContext {
    GuiTextNativeRendererServices& graphics;
    NativeStringStorage& strings;
    NativeGameGridCpuCalls& cpu;
    NativeGameGridConstants constants;
};
struct NativeGameGridOperation final {
    enum class Phase { fresh,running,complete,failed,diagnostic_retired };
    Phase phase{Phase::fresh};
    void* owner{};
    NativeGameGridContext* context{};
    std::uint32_t native_site{};
    NativeString format_name;
    bool name_cleanup_active{};
    GuiNativeDeclarationAcquired declaration;
    NativeStreamCloneAcquired vertex,index;
    NativeGameGridOperation()=default;
    ~NativeGameGridOperation();
    NativeGameGridOperation(const NativeGameGridOperation&)=delete;
    NativeGameGridOperation& operator=(const NativeGameGridOperation&)=delete;
    // Only after the caller has resolved every retained owner/map/factory
    // record. This acknowledgement performs no resource release or rollback.
    void acknowledge_diagnostic_cleanup() noexcept;
};

// Full 70B330 normal body, ECX owner, stacked38h descriptor, RET4. Six actual
// CPU allocations, original x87/SSE geometry, existing declaration cache and
// raw renderer stream factories/maps/ownership. Native rectangular-grid quirks
// are retained. Failed source operations preserve partial effects and cannot
// be replayed; native FH3/SEH/hardware-fault identity is not reproduced.
void initialize_native_game_grid_0070b330(void* actual_grid,const void* descriptor,
    NativeGameGridContext&,NativeGameGridOperation&);
// Full 70BD70 normal body, ECX84h allocation, stacked angle, EAXsame, RET4.
// Source angle representation is explicit; x87 FSIN/FCOS and spills preserved.
void* construct_native_game_grid_0070bd70(void* allocation,float angle,
    const NativeGameGridDescriptorPreimage&,NativeGameGridContext&,NativeGameGridOperation&);
// Full 70B220 and70B280. Stamp original profile, release CPU arrays, release
// current vertex then index creator references and clear fields AFTER calls.
// Scalar deletion frees the allocation iff the LOW flags byte has bit0 set.
void destroy_native_game_grid_0070b220(void* actual_grid,NativeGameGridContext&);
void* delete_native_game_grid_0070b280(void* actual_grid,std::uint8_t flags,NativeGameGridContext&);
} // namespace bsp

#include "bsp/native_frame_target_owner.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <exception>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native frame target owner requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);

void* address(const void* storage, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(storage) + offset);
}
__forceinline std::uint32_t word(const void* storage) noexcept {
    std::uint32_t result;
    __asm { mov eax, storage }
    __asm { mov eax, dword ptr [eax] }
    __asm { mov result, eax }
    return result;
}
__forceinline void put(void* storage, std::uint32_t value) noexcept {
    __asm { mov eax, storage }
    __asm { mov edx, value }
    __asm { mov dword ptr [eax], edx }
}
__forceinline void put_byte(void* storage, std::uint8_t value) noexcept {
    __asm { mov eax, storage }
    __asm { mov dl, value }
    __asm { mov byte ptr [eax], dl }
}
void* pointer(std::uint32_t value) noexcept { return reinterpret_cast<void*>(value); }
NativeFrameTargetVectorStorage& records(void* owner) noexcept {
    return *static_cast<NativeFrameTargetVectorStorage*>(address(owner, 0x1c));
}

std::uint32_t surface_slot(void* actual_owner, std::uint32_t offset,
    NativeFrameTargetOwnerContext& context) noexcept {
    const auto current_profile = word(actual_owner);
    __assume(current_profile == 0x00d619a0);
    return word(address(const_cast<const std::uint32_t*>(
        context.actual_surface_profile_00d619a0), offset));
}
void release_surface_at_zero(void* captured, NativeFrameTargetOwnerContext& context) {
    const auto invoker = surface_slot(captured, 0, context);
    __assume(invoker == 0x00bd30e0);
    // Full concrete BD30E0: no second decrement, reload the CURRENT +4 slot.
    const auto terminal = surface_slot(captured, 4, context);
    __assume(terminal == 0x00b3f5b0);
    delete_native_surface_00b3f5b0(*static_cast<NativeSurfaceOwnerStorage*>(captured),
        1, context.actual_surface_context);
}
void release_intrusive_field(void* owner, std::uint32_t offset,
    NativeFrameTargetOwnerContext& context) {
    if (auto* const captured = pointer(word(address(owner, offset)))) {
        if (InterlockedDecrement(static_cast<volatile LONG*>(address(captured, 4))) == 0)
            release_surface_at_zero(captured, context);
        put(address(owner, offset), 0);
    }
}
using ComRelease = ULONG (__stdcall*)(void*);
void release_current_com_field(void* owner, std::uint32_t offset) {
    if (auto* const current_com = pointer(word(address(owner, offset)))) {
        const auto release = reinterpret_cast<ComRelease>(
            word(address(pointer(word(current_com)), 8)));
        release(current_com);
        put(address(owner, offset), 0);
    }
}

int terminate_cpp_cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_owner(void* owner, int state) noexcept {
    // FH3 map DF5204: state1 -> B1FB90(this+1C), state0 -> BD30F0(this).
    // A second C++ exception must terminate during search, before an extra
    // nested cleanup can run. Ordinary COM/surface exceptions reach this map.
    __try {
        if (state == 1) destroy_native_frame_target_vector_00b1fb90(records(owner));
        put(owner, 0x00ceb130);
    } __except (terminate_cpp_cleanup_exception(GetExceptionCode())) {
        __assume(0);
    }
}
struct OwnerCleanup {
    void* owner;
    int state{1};
    ~OwnerCleanup() noexcept { if (state >= 0) unwind_owner(owner, state); }
};

void assign_surface(void* field, NativeSurfaceOwnerStorage* incoming,
    NativeFrameTargetOwnerContext& context) {
    auto* const outgoing = pointer(word(field));
    if (outgoing != incoming) {
        put(field, reinterpret_cast<std::uint32_t>(incoming));
        if (incoming) InterlockedIncrement(static_cast<volatile LONG*>(address(incoming, 4)));
        if (outgoing &&
            InterlockedDecrement(static_cast<volatile LONG*>(address(outgoing, 4))) == 0)
            release_surface_at_zero(outgoing, context);
        // Native assignment has no post-terminal clear or rollback.
    }
}
} // namespace

NativeFrameTargetOwnerStorage* construct_native_frame_target_owner_00b1fbb0(
    void* actual_storage) noexcept {
    auto* const owner = ::new (actual_storage) NativeFrameTargetOwnerStorage;
    put(owner, 0x00ceb130);
    put(address(owner, 4), 1);
    put(owner, 0x00d5e600);
    put(address(owner, 0x18), 0);
    put(address(owner, 0x1c), 0);
    put(address(owner, 0x20), 0);
    put(address(owner, 0x24), 0);
    put(address(owner, 0x38), 0);
    put_byte(address(owner, 0x3c), 0);
    put(address(owner, 8), 0);
    put(address(owner, 0x28), 0);
    put(address(owner, 0xc), 0);
    put(address(owner, 0x2c), 0);
    put(address(owner, 0x10), 0);
    put(address(owner, 0x30), 0);
    put(address(owner, 0x14), 0);
    put(address(owner, 0x34), 0);
    return owner;
}

void destroy_native_frame_target_owner_00b1fc00(
    NativeFrameTargetOwnerStorage& owner, NativeFrameTargetOwnerContext& context) {
    put(&owner, 0x00d5e600);
    OwnerCleanup cleanup{&owner};
    for (std::uint32_t slot = 0; slot != 4; ++slot) {
        release_intrusive_field(&owner, 8 + slot * 4, context);
        release_current_com_field(&owner, 0x28 + slot * 4);
    }
    release_intrusive_field(&owner, 0x18, context);
    release_current_com_field(&owner, 0x38);
    cleanup.state = 0;
    resize_native_frame_target_vector_00b1f9f0(records(&owner), 0);
    singleton_lifetime_free(pointer(word(address(&owner, 0x1c))));
    cleanup.state = -1;
    put(&owner, 0x00ceb130); // Full BD30F0, reached at B1FCD7 after vector free.
}

NativeFrameTargetOwnerStorage* delete_native_frame_target_owner_00b1fcf0(
    NativeFrameTargetOwnerStorage& owner, std::uint32_t flags,
    NativeFrameTargetOwnerContext& context) {
    auto* const original_address = &owner;
    destroy_native_frame_target_owner_00b1fc00(owner, context);
    if ((flags & 1u) != 0) singleton_lifetime_free(original_address);
    return original_address;
}

void set_native_frame_target_color_00b1fab0(NativeFrameTargetOwnerStorage& owner,
    std::uint32_t slot, NativeSurfaceOwnerStorage* incoming,
    NativeFrameTargetOwnerContext& context) {
    assign_surface(address(&owner, 8u + slot * 4u), incoming, context);
}

void set_native_frame_target_depth_00b1fb00(NativeFrameTargetOwnerStorage& owner,
    NativeSurfaceOwnerStorage* incoming, NativeFrameTargetOwnerContext& context) {
    assign_surface(address(&owner, 0x18), incoming, context);
}

void set_native_frame_target_srgb_byte_00b1f700(
    NativeFrameTargetOwnerStorage& owner, std::uint8_t value) noexcept {
    put_byte(address(&owner, 0x3c), value);
}

} // namespace bsp

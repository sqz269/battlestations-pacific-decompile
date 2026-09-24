#include "bsp/system_fog_owner.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/native_ref_counted.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <array>
#include <cstring>
#include <new>
#include <stdexcept>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error System fog ownership requires MSVC Win32 native pointer and LONG widths.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(long) == 4);
static_assert(std::is_standard_layout_v<SystemFogOwner>);
static_assert(std::is_trivially_destructible_v<SystemFogOwner>);
static_assert(sizeof(SystemFogOwner) == kSystemFogOwnerNativeBytes);
static_assert(offsetof(SystemFogOwner, references_04) == 4);
static_assert(offsetof(SystemFogOwner, fields_08) == 8);
static_assert(sizeof(SystemFogState) == 0x8c);
static_assert(offsetof(SystemFogState, color_08) == 0x00);
static_assert(offsetof(SystemFogState, underwater_color_18) == 0x10);
static_assert(offsetof(SystemFogState, directional_colors_28) == 0x20);
static_assert(offsetof(SystemFogState, scalar_68) == 0x60);
static_assert(offsetof(SystemFogState, scalar_90) == 0x88);

namespace {
void store_bits(float& target, std::uint32_t bits) noexcept {
    std::memcpy(&target, &bits, sizeof(bits));
}

void copy_four_words_forward(void* destination, const void* source) noexcept {
    // Keep individual MOV read/store pairs, including propagating overlap and
    // signaling-NaN payloads. memcpy of the full vector would change overlap.
    __asm {
        mov ecx, destination
        mov eax, source
        mov edx, dword ptr [eax]
        mov dword ptr [ecx], edx
        mov edx, dword ptr [eax + 4]
        mov dword ptr [ecx + 4], edx
        mov edx, dword ptr [eax + 8]
        mov dword ptr [ecx + 8], edx
        mov edx, dword ptr [eax + 12]
        mov dword ptr [ecx + 12], edx
    }
}

void set_counted_slot(const SystemFogState*& slot, SystemFogOwner* next) noexcept {
    const auto* old_fields = slot;
    const auto* next_fields = next ? &next->fields_08 : nullptr;
    if (old_fields == next_fields) return;
    slot = next_fields; // native publishes BEFORE InterlockedIncrement
    if (next) retain_system_fog_owner(*next);
    if (old_fields) release_system_fog_owner(*system_fog_owner_from_state(old_fields));
}
// ECX actual94h; EDX stable source metadata containing nine borrowed cells.
// Only source register plumbing differs; native scratch/store/read order stays.
__declspec(naked) SystemFogOwner* __fastcall construct_raw_fog_kernel(
    void*, const volatile std::uint32_t* const*) {
    __asm {
        push esi
        push ebx
        mov esi, edx
        sub esp, 10h
        xorps xmm0, xmm0
        mov eax, ecx
        mov ebx, [esi]
        movss xmm1, dword ptr [ebx]
        mov ebx, [esi + 4]
        movss xmm2, dword ptr [ebx]
        movss dword ptr [esp], xmm0
        mov ecx, [esp]
        movss dword ptr [esp + 4], xmm0
        mov edx, [esp + 4]
        movss dword ptr [esp + 8], xmm0
        movss dword ptr [esp + 0ch], xmm0
        mov [eax + 8], ecx
        mov ecx, [esp + 8]
        movss dword ptr [esp], xmm0
        movss dword ptr [esp + 4], xmm0
        movss dword ptr [esp + 8], xmm0
        mov [eax + 0ch], edx
        mov edx, [esp + 0ch]
        movss dword ptr [esp + 0ch], xmm0
        mov ebx, [esi + 8]
        movss xmm0, dword ptr [ebx]
        movss dword ptr [eax + 68h], xmm0
        movss dword ptr [eax + 78h], xmm0
        mov ebx, [esi + 0ch]
        movss xmm0, dword ptr [ebx]
        movss dword ptr [eax + 7ch], xmm0
        mov ebx, [esi + 10h]
        movss xmm0, dword ptr [ebx]
        movss dword ptr [eax + 80h], xmm0
        mov ebx, [esi + 14h]
        movss xmm0, dword ptr [ebx]
        mov [eax + 10h], ecx
        mov ecx, [esp]
        movss dword ptr [eax + 84h], xmm0
        mov ebx, [esi + 18h]
        movss xmm0, dword ptr [ebx]
        mov [eax + 14h], edx
        mov edx, [esp + 4]
        mov [eax + 18h], ecx
        mov ecx, [esp + 8]
        mov [eax + 1ch], edx
        mov edx, [esp + 0ch]
        movss dword ptr [eax + 88h], xmm0
        mov ebx, [esi + 1ch]
        movss xmm0, dword ptr [ebx]
        mov dword ptr [eax], 0ceb130h
        movss dword ptr [eax + 8ch], xmm0
        mov ebx, [esi + 20h]
        movss xmm0, dword ptr [ebx]
        mov [eax + 20h], ecx
        mov dword ptr [eax + 4], 1
        mov dword ptr [eax], 0d63180h
        mov [eax + 24h], edx
        movss dword ptr [eax + 6ch], xmm1
        movss dword ptr [eax + 70h], xmm2
        movss dword ptr [eax + 74h], xmm1
        movss dword ptr [eax + 90h], xmm0
        add esp, 10h
        pop ebx
        pop esi
        ret
    }
}
const volatile std::uint32_t* fog_table(std::uint32_t profile,
    NativeSystemFogStorageContext& context) {
    if (profile != kSystemFogOwnerVtable || !context.actual_profile_00d63180)
        throw std::invalid_argument("raw fog requires its actual current D63180 table");
    return context.actual_profile_00d63180;
}
class RawFogDelete final : public NativeRefCountedDeleteCalls {
public:
    explicit RawFogDelete(NativeSystemFogStorageContext& context) : context_(context) {}
    void delete_vslot04(void* actual, std::uint32_t profile, std::uint32_t flags) override {
        if (fog_table(profile, context_)[1] != 0x00b84f70u || flags != 1)
            throw std::invalid_argument("raw fog requires genuine fresh current4 B84F70");
        (void)delete_system_fog_owner_00b84f70(static_cast<SystemFogOwner*>(actual), flags);
    }
private:
    NativeSystemFogStorageContext& context_;
};
} // namespace

SystemFogOwner* initialize_system_fog_owner_00b84e50(void* storage) noexcept {
    // Legacy callers retain their installed-constant domain and SAME owner.
    static const std::uint32_t words[]{0x43480000, 0x45dac000, 0x3ecccccd,
        0xc2f00000, 0x44480000, 0x3f7851ec, 0xc3480000, 0xc30c0000, 0x3f7eb852};
    const NativeSystemFogConstants constants{words[0], words[1], words[2],
        words[3], words[4], words[5], words[6], words[7], words[8]};
    return initialize_system_fog_owner_00b84e50(storage, constants);
}
SystemFogOwner* initialize_system_fog_owner_00b84e50(
    void* storage, const NativeSystemFogConstants& constants) noexcept {
    static_assert(std::is_trivially_copyable_v<SystemFogOwner>);
    std::array<unsigned char, sizeof(SystemFogOwner)> preimage;
    std::memcpy(preimage.data(), storage, preimage.size());
    auto* owner = ::new (storage) SystemFogOwner;
    std::memcpy(owner, preimage.data(), preimage.size());
    // Capture only metadata addresses, never any native constant values. Use
    // a real pointer array rather than assuming reference-member object layout.
    const volatile std::uint32_t* cells[]{&constants.scalar_6c_74_00ce386c,
        &constants.scalar_70_00d63188, &constants.scalar_68_78_00ce7804,
        &constants.scalar_7c_00ce77f8, &constants.scalar_80_00ce3950,
        &constants.scalar_84_00ce77e8, &constants.scalar_88_00ce77e4,
        &constants.scalar_8c_00ce77e0, &constants.scalar_90_00ce77dc};
    return construct_raw_fog_kernel(owner, cells);
}

void invoke_native_system_fog_zero(SystemFogOwner* captured,
    NativeSystemFogStorageContext& context) {
    if (!captured || captured->references_04 != 0)
        throw std::logic_error("raw fog terminal requires SAME live actual04 observed zero");
    if (fog_table(captured->native_vtable_00, context)[0] != 0x00bd30e0u)
        throw std::invalid_argument("raw fog requires genuine current0 BD30E0");
    RawFogDelete calls(context);
    invoke_native_ref_counted_delete_00bd30e0(captured, calls);
}
void set_native_camera_fog_storage_00b71940(void* actual_camera,
    const volatile std::uint32_t& incoming_argument, NativeSystemFogStorageContext& context) {
    const auto incoming = incoming_argument;
    auto& cell = *reinterpret_cast<volatile std::uint32_t*>(
        reinterpret_cast<std::uintptr_t>(actual_camera) + 0x184u);
    const auto old = cell;
    if (old == incoming) return;
    cell = incoming;
    if (incoming) context.increment_00ce221c(
        &reinterpret_cast<SystemFogOwner*>(incoming)->references_04);
    if (old) {
        auto* const captured = reinterpret_cast<SystemFogOwner*>(old);
        if (context.decrement_00ce2220(&captured->references_04) == 0)
            invoke_native_system_fog_zero(captured, context);
    }
}

SystemFogOwner* allocate_system_fog_owner() {
    void* storage = singleton_lifetime_allocate({SingletonAllocationKind::object,
        kSystemFogOwnerNativeBytes, sizeof(SystemFogOwner)});
    return initialize_system_fog_owner_00b84e50(storage);
}

SystemFogOwner* system_fog_owner_from_state(const SystemFogState* fields) noexcept {
    if (!fields) return nullptr;
    return reinterpret_cast<SystemFogOwner*>(reinterpret_cast<std::uintptr_t>(fields)
        - offsetof(SystemFogOwner, fields_08));
}

void retain_system_fog_owner(SystemFogOwner& owner) noexcept {
    (void)::InterlockedIncrement(&owner.references_04);
}

void release_system_fog_owner(SystemFogOwner& owner) noexcept {
    if (::InterlockedDecrement(&owner.references_04) == 0) {
        invoke_system_fog_deleting_destructor_00bd30e0(&owner);
    }
}

void invoke_system_fog_deleting_destructor_00bd30e0(SystemFogOwner* owner) noexcept {
    if (owner) {
        // Concrete D63180[0] = BD30E0, D63180[1] = B84F70. This is the
        // actual known leaf target, not a placeholder subclass callback.
        (void)delete_system_fog_owner_00b84f70(owner, 1);
    }
}

SystemFogOwner* delete_system_fog_owner_00b84f70(
    SystemFogOwner* owner, std::uint32_t flags) noexcept {
    auto* const original_address = owner;
    owner->native_vtable_00 = kSystemFogOwnerVtable;
    owner->native_vtable_00 = kSystemFogOwnerBaseVtable; // bounded 00BD30F0
    if ((flags & 1u) != 0) {
        owner->~SystemFogOwner();
        singleton_lifetime_free(owner);
    }
    // Native omitted listing bytes00B84F8B are ADD ESP,4, then this return.
    return original_address;
}

void set_system_fog_camera_owner_00b71940(
    const SystemFogState*& slot, SystemFogOwner* next) noexcept {
    set_counted_slot(slot, next);
}

void set_system_fog_world_owner_00bbdf20(
    const SystemFogState*& slot, SystemFogOwner* next) noexcept {
    set_counted_slot(slot, next);
}

void initialize_system_fog_camera_slot_00b71ae3(const SystemFogState*& slot) noexcept {
    slot = nullptr;
}

void clear_system_fog_camera_slot_00b71f68(const SystemFogState*& slot) noexcept {
    if (const auto* fields = slot) {
        release_system_fog_owner(*system_fog_owner_from_state(fields));
        slot = nullptr; // after virtual destruction/free returns, not before
    }
}

void set_system_fog_camera_owner_00b71940(SystemFogSlotRef slot, SystemFogOwner* next) noexcept {
    auto* const previous = slot.owner();
    if (previous == next) return;
    slot.store_owner(next);
    if (next) retain_system_fog_owner(*next);
    if (previous) release_system_fog_owner(*previous);
}
void initialize_system_fog_camera_slot_00b71ae3(SystemFogSlotRef slot) noexcept {
    slot.clear();
}
void clear_system_fog_camera_slot_00b71f68(SystemFogSlotRef slot) noexcept {
    if (auto* const previous = slot.owner()) {
        release_system_fog_owner(*previous);
        slot.clear(); // after the actual zero-reference destruction/free
    }
}

void set_system_fog_color_00b84c40(SystemFogOwner& owner, const void* source) noexcept {
    copy_four_words_forward(owner.fields_08.color_08.data(), source);
}

void set_system_fog_underwater_color_00b84c70(
    SystemFogOwner& owner, const void* source) noexcept {
    copy_four_words_forward(owner.fields_08.underwater_color_18.data(), source);
}

void set_system_fog_directional_color_00b84fa0(
    SystemFogOwner& owner, const void* source, std::uint32_t index) noexcept {
    const auto offset = std::uint32_t{0x28} + (index << 4);
    auto* destination = reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(&owner) + offset);
    copy_four_words_forward(destination, source);
}

void set_system_fog_scalar_68_00b84d00(SystemFogOwner& o, std::uint32_t b) noexcept { store_bits(o.fields_08.scalar_68, b); }
void set_system_fog_scalar_6c_00b84d10(SystemFogOwner& o, std::uint32_t b) noexcept { store_bits(o.fields_08.scalar_6c, b); }
void set_system_fog_scalar_70_00b84d20(SystemFogOwner& o, std::uint32_t b) noexcept { store_bits(o.fields_08.scalar_70, b); }
void set_system_fog_scalar_74_00b84d30(SystemFogOwner& o, std::uint32_t b) noexcept { store_bits(o.fields_08.scalar_74, b); }
void set_system_fog_scalar_78_00b84d40(SystemFogOwner& o, std::uint32_t b) noexcept { store_bits(o.fields_08.scalar_78, b); }
void set_system_fog_scalar_7c_00b84d50(SystemFogOwner& o, std::uint32_t b) noexcept { store_bits(o.fields_08.scalar_7c, b); }
void set_system_fog_scalar_80_00b84d60(SystemFogOwner& o, std::uint32_t b) noexcept { store_bits(o.fields_08.scalar_80, b); }
void set_system_fog_scalar_84_00b84d80(SystemFogOwner& o, std::uint32_t b) noexcept { store_bits(o.fields_08.scalar_84, b); }
void set_system_fog_scalar_88_00b84dc0(SystemFogOwner& o, std::uint32_t b) noexcept { store_bits(o.fields_08.scalar_88, b); }
void set_system_fog_scalar_8c_00b84de0(SystemFogOwner& o, std::uint32_t b) noexcept { store_bits(o.fields_08.scalar_8c, b); }
void set_system_fog_scalar_90_00b84e00(SystemFogOwner& o, std::uint32_t b) noexcept { store_bits(o.fields_08.scalar_90, b); }

} // namespace bsp

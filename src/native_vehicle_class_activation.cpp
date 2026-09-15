#include "bsp/native_vehicle_class_activation.hpp"

#include "bsp/gun_aiming.hpp"
#include "bsp/native_vehicle_pointer_array.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native vehicle platform arc cleanup requires MSVC Win32 x87 assembly.
#endif

namespace bsp {
namespace {

using Word = std::uint32_t;

void* pointer(Word value) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}

const void* pointer_const(Word value) noexcept {
    return reinterpret_cast<const void*>(static_cast<std::uintptr_t>(value));
}

constexpr std::size_t kPlatformDeviceListSentinel = 0x20;
constexpr std::size_t kVehiclePlatformArray = 0x94;
constexpr std::size_t kVehiclePlatformCount = 0x98;
constexpr std::size_t kClassActivationLatch = 0x44;
constexpr std::size_t kListNodeNext = 0x00;
constexpr std::size_t kListNodeValue = 0x08;
constexpr std::size_t kDeviceClassId = 0x6c;

// 00D08B88 is loaded as a double by 007F6EB9. Its exact little-endian bytes are
// 00 00 00 A0 46 DF 81 3F.
const double kArcPruneThreshold = 0.008726646192371845;

static_assert(sizeof(GunFiringArc) == 0x14);

Word read_volatile_word(const void* base, std::size_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(
        static_cast<const std::uint8_t*>(base) + offset);
}

std::uint8_t read_volatile_byte(const void* base, std::size_t offset) noexcept {
    return *reinterpret_cast<const volatile std::uint8_t*>(
        static_cast<const std::uint8_t*>(base) + offset);
}

std::int32_t read_volatile_count(const void* base, std::size_t offset) noexcept {
    return *reinterpret_cast<const volatile std::int32_t*>(
        static_cast<const std::uint8_t*>(base) + offset);
}

void invalid(const SingletonLifetimeCallbacks& callbacks) {
    callbacks.invalid_parameter(callbacks.context);
}

// Exact ordinary tail007F6EA7..007F6F70. The threshold remains resident on the
// x87 stack across the full pruning loop. Flagged records skip all FP loads;
// FCOMI/JBE retains equal and unordered spans. The five DWORD moves and every
// current data/count reload are kept in listing order. The final transition
// scan remains raw assembly, so its reached reads cannot be optimized away.
__declspec(naked) void __fastcall prune_and_scan_platform_arcs_007f6ea7(void*) {
    __asm {
        push ebx
        push esi
        push edi
        mov esi, ecx

        mov eax, dword ptr [esi + 40h] // 007f6ea7
        mov edx, dword ptr [esi + 3ch] // 007f6eaa
        lea ecx, [eax + eax*4] // 007f6ead
        mov eax, edx // 007f6eb0
        lea ecx, [eax + ecx*4] // 007f6eb2
        cmp edx, ecx // 007f6eb5
        jz final_scan // 007f6eb7
        fld qword ptr [kArcPruneThreshold] // 007f6eb9
        or edi, 0ffffffffh // 007f6ebf

    inspect_arc:
        cmp byte ptr [edx], 0 // 007f6ec2
        jnz keep_arc // 007f6ec5
        fld dword ptr [edx + 8] // 007f6ec7
        fsub dword ptr [edx + 4] // 007f6eca
        fxch st(1) // 007f6ecd
        fcomi st(0), st(1) // 007f6ecf
        fstp st(1) // 007f6ed1
        jbe keep_arc // 007f6ed3
        mov ecx, dword ptr [esi + 40h] // 007f6ed5
        mov ebx, dword ptr [esi + 3ch] // 007f6ed8
        lea ecx, [ecx + ecx*4] // 007f6edb
        lea ecx, [ebx + ecx*4 - 14h] // 007f6ede
        cmp edx, ecx // 007f6ee2
        mov eax, edx // 007f6ee4
        jz decrement_count // 007f6ee6

    copy_next_arc:
        mov ecx, dword ptr [eax + 14h] // 007f6ee8
        mov dword ptr [eax], ecx // 007f6eeb
        mov ecx, dword ptr [eax + 18h] // 007f6eed
        mov dword ptr [eax + 4], ecx // 007f6ef0
        mov ecx, dword ptr [eax + 1ch] // 007f6ef3
        mov dword ptr [eax + 8], ecx // 007f6ef6
        mov ecx, dword ptr [eax + 20h] // 007f6ef9
        mov dword ptr [eax + 0ch], ecx // 007f6efc
        mov ecx, dword ptr [eax + 24h] // 007f6eff
        mov dword ptr [eax + 10h], ecx // 007f6f02
        mov ecx, dword ptr [esi + 40h] // 007f6f05
        mov ebx, dword ptr [esi + 3ch] // 007f6f08
        lea ecx, [ecx + ecx*4] // 007f6f0b
        add eax, 14h // 007f6f0e
        lea ecx, [ebx + ecx*4 - 14h] // 007f6f11
        cmp eax, ecx // 007f6f15
        jnz copy_next_arc // 007f6f17

    decrement_count:
        add dword ptr [esi + 40h], edi // 007f6f19
        jmp compare_end // 007f6f1c

    keep_arc:
        add edx, 14h // 007f6f1e

    compare_end:
        mov eax, dword ptr [esi + 40h] // 007f6f21
        mov ecx, dword ptr [esi + 3ch] // 007f6f24
        lea eax, [eax + eax*4] // 007f6f27
        lea eax, [ecx + eax*4] // 007f6f2a
        cmp edx, eax // 007f6f2d
        jnz inspect_arc // 007f6f2f
        fstp st(0) // 007f6f31

    final_scan:
        mov eax, dword ptr [esi + 3ch] // 007f6f33
        mov ecx, dword ptr [esi + 40h] // 007f6f36
        mov dl, byte ptr [eax] // 007f6f39
        mov edi, dword ptr [esi + 3ch] // 007f6f3b
        lea ecx, [ecx + ecx*4] // 007f6f3e
        add eax, 14h // 007f6f41
        lea ecx, [edi + ecx*4] // 007f6f44
        and dl, 1 // 007f6f47
        cmp eax, ecx // 007f6f4a
        jz done // 007f6f4c
        mov ecx, dword ptr [esi + 40h] // 007f6f4e
        lea ecx, [ecx + ecx*4] // 007f6f51
        mov esi, edi // 007f6f54
        lea esi, [esi + ecx*4] // 007f6f56
        lea esp, [esp] // 007f6f59

    scan_transition:
        mov cl, byte ptr [eax] // 007f6f60
        and cl, 1 // 007f6f62
        cmp cl, dl // 007f6f65
        jz advance_scan // 007f6f67
        mov dl, cl // 007f6f69

    advance_scan:
        add eax, 14h // 007f6f6b
        cmp eax, esi // 007f6f6e
        jnz scan_transition // 007f6f70

    done:
        pop edi
        pop esi
        pop ebx
        ret
    }
}

} // namespace

void activate_native_vehicle_platform_007f6e50(
    void* actual_platform, NativeVehicleEntryActivationCalls& calls,
    const SingletonLifetimeCallbacks& validation) {
    // platform+1Ch is the list header; +20h is its sentinel pointer. Each node
    // owns a device-class reference at +8h. The producer at009614F0 resolves
    // the authored id through00443090 before inserting that reference.
    const Word list_owner = static_cast<Word>(reinterpret_cast<std::uintptr_t>(actual_platform))
        + 0x1cu;
    const Word captured_sentinel = read_volatile_word(
        actual_platform, kPlatformDeviceListSentinel);
    Word node = read_volatile_word(pointer_const(captured_sentinel), kListNodeNext);
    for (;;) {
        if (list_owner == 0) {
            invalid(validation); // 007F6E6E; may return.
        }
        if (node == captured_sentinel) {
            break;
        }
        if (list_owner == 0) {
            invalid(validation); // 007F6E7D; may return.
        }
        if (node == read_volatile_word(pointer_const(list_owner), 4)) {
            invalid(validation); // 007F6E87; may return and repair the list.
        }
        const void* const device_class = pointer_const(
            read_volatile_word(pointer_const(node), kListNodeValue));
        calls.call_00443490(read_volatile_word(device_class, kDeviceClassId), 0);
        if (node == read_volatile_word(pointer_const(list_owner), 4)) {
            invalid(validation); // 007F6E9E; may return after callback mutation.
        }
        node = read_volatile_word(pointer_const(node), kListNodeNext);
    }
    prune_and_scan_platform_arcs_007f6ea7(actual_platform);
}

void activate_native_vehicle_class_009598d0(void* actual_class,
    std::uint32_t enemy, NativeVehicleClassActivationContext& context,
    NativeDamageableClassModelAcquired& model_acquired) {
    if (read_volatile_byte(actual_class, kClassActivationLatch) != 0) {
        return;
    }

    activate_native_damageable_class_model_00879aa0(
        actual_class, enemy, context.model, model_acquired);

    Word index = 0;
    while (static_cast<std::int32_t>(index)
        < read_volatile_count(actual_class, kVehiclePlatformCount)) {
        auto* const entries = static_cast<std::uint8_t*>(actual_class) + kVehiclePlatformArray;
        if (static_cast<std::int32_t>(index)
            >= read_volatile_count(actual_class, kVehiclePlatformCount)) {
            resize_native_vehicle_pointer_array_005471b0(
                entries, static_cast<std::int32_t>(index + 1u), context.pointer_arrays);
        }
        const auto* data = pointer_const(read_volatile_word(entries));
        const Word candidate = read_volatile_word(
            data, index * static_cast<Word>(sizeof(Word)));
        if (candidate != 0) {
            if (static_cast<std::int32_t>(index)
                >= read_volatile_count(actual_class, kVehiclePlatformCount)) {
                resize_native_vehicle_pointer_array_005471b0(
                    entries, static_cast<std::int32_t>(index + 1u), context.pointer_arrays);
            }
            data = pointer_const(read_volatile_word(entries));
            activate_native_vehicle_platform_007f6e50(
                pointer(read_volatile_word(
                    data, index * static_cast<Word>(sizeof(Word)))),
                context.entries, context.validation);
        }
        index += 1u;
    }
}

} // namespace bsp

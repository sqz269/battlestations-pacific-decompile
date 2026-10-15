#include "bsp/native_zone_leaf_00423640.hpp"
#include "bsp/native_render_batch_keys.hpp"

#include <limits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native zone leaf requires MSVC Win32 x87 assembly.
#endif

namespace bsp {
namespace {

std::int32_t truncate_layer(const float* depth) noexcept {
    std::int32_t result;
    __asm {
        mov eax, depth
        cvttss2si eax, dword ptr [eax]
        mov result, eax
    }
    return result;
}

std::int32_t update_owner_params(Cc13X87cContourOwnerNative* owner,
    NativeZoneLeafParams* params, float original_cell,
    const NativeZoneLeafBindings& bindings) {
    const auto unit_length = bindings.unit_length_00880250;
    const auto actual_mode = bindings.actual_0109eea4;
    const double half = 0.5; // exact f64 bytes at 00D7A280
    float rounded_unit;
    std::int32_t result;
    // 004236F0..00423713: preserve the first binary32 spill, the two FSTs,
    // extended division/addition and the shared, mode-dependent ST0 consumer.
    __asm {
        mov ecx, owner
        call unit_length
        fstp dword ptr rounded_unit
        fld dword ptr rounded_unit
        mov eax, params
        fst dword ptr [eax + 14h]
        fst dword ptr [eax + 4]
        fdivr dword ptr original_cell
        fadd qword ptr half
        mov ecx, actual_mode
        call native_crt_truncate_st0_00bf7420
        mov result, eax
    }
    if (result < 1) {
        result = 1;
    } else if (result > 16) {
        result = 16;
    }
    return result;
}

bool owner_accepts_44(Cc13X87cContourOwnerNative* owner) {
    using Predicate = bool (__thiscall*)(Cc13X87cContourOwnerNative*, std::int32_t);
    const auto* table = *reinterpret_cast<const std::uintptr_t* const*>(owner);
    const auto predicate = reinterpret_cast<Predicate>(table[0x5C / 4]);
    return predicate(owner, 0x44); // AL only, as at 004236BD / 0042393B
}

std::int32_t source_extent(void* source, std::uint32_t slot) {
    using Extent = std::int32_t (__thiscall*)(void*);
    const auto* table = *reinterpret_cast<const std::uintptr_t* const*>(source);
    return reinterpret_cast<Extent>(table[slot / 4])(source);
}

void replace_path_name(Cc13X87cContourPathNative& path, const char* name,
    const NativeZoneLeafBindings& bindings) {
    std::uint32_t length = 0;
    while (name[length] != '\0') {
        ++length;
    }
    if (length != path.name_length) {
        if (length == 0) {
            if (path.name_data != nullptr) {
                char* const old = path.name_data;
                const std::uint32_t bytes = path.name_length + 1U;
                void* const pool = bindings.pool_00419cc0();
                bindings.pool_return_00bd1510(pool, old, bytes, 1);
            }
            path.name_data = nullptr;
            path.name_length = 0;
        } else {
            void* const pool = bindings.pool_00419cc0();
            char* const replacement = bindings.pool_allocate_00bd1120(pool, length + 1U, 1);
            if (path.name_data != nullptr) {
                char* const old = path.name_data;
                const std::uint32_t bytes = path.name_length + 1U;
                void* const old_pool = bindings.pool_00419cc0();
                bindings.pool_return_00bd1510(old_pool, old, bytes, 1);
            }
            path.name_data = replacement;
            path.name_length = length;
            replacement[length] = '\0';
        }
    }
    if (path.name_data != nullptr) {
        bindings.copy_00bf7680(path.name_data, name, path.name_length);
    }
}

} // namespace

void generate_native_zone_paths_00423640(Cc13AvoidZoneManagerNative& manager,
    Cc13X87cContourOwnerNative* requested_owner, NativeZoneLeafParams& params,
    const NativeZoneLeafBindings& bindings) {
    const std::int32_t layer = truncate_layer(&params.shore.depth);
    auto* const group = bindings.group_for_layer_004120d0(&manager, layer);
    if (group != nullptr) {
        bindings.remove_owner_0041d150(group, requested_owner);
    }

    // The native reads this chain even when requested_owner is nonnull.
    auto* const game = static_cast<std::uint8_t*>(*bindings.actual_00e188a8);
    auto* const scene = *reinterpret_cast<std::uint8_t**>(game + 0x19CC);
    auto* node = *reinterpret_cast<NativeZoneLeafSceneNode**>(scene + 0x34C);
    const bool scan_all = requested_owner == nullptr;
    auto* owner = requested_owner;
    if (scan_all && node != nullptr && node->owner != nullptr && owner_accepts_44(node->owner)) {
        owner = node->owner; // reload after the predicate, as the native does
    }
    const float original_cell = params.shore.cell_size;
    std::uint32_t sequence = 0;

    for (;;) {
        if (owner != nullptr) {
            params.shore.block = update_owner_params(owner, &params, original_cell, bindings);
            const std::int32_t width = source_extent(owner->grid_source, 0x10);
            const std::int32_t height = source_extent(owner->grid_source, 0x14);
            if (width > 1 && height > 1) {
                const std::uint64_t wide_bytes = static_cast<std::uint64_t>(width) * 4U;
                const std::uint32_t bytes = wide_bytes > std::numeric_limits<std::uint32_t>::max()
                    ? std::numeric_limits<std::uint32_t>::max()
                    : static_cast<std::uint32_t>(wide_bytes);
                auto** const columns = static_cast<std::int8_t**>(bindings.allocate_00bf55be(bytes));
                for (std::int32_t x = 0; x < width; ++x) {
                    columns[x] = static_cast<std::int8_t*>(
                        bindings.allocate_00bf55be(static_cast<std::uint32_t>(height)));
                }
                bindings.shore_mask_00880420(owner, columns, 1, &params.shore);
                for (;;) {
                    auto* path = bindings.trace_00421ce0(owner, columns, 1);
                    if (path == nullptr) {
                        break;
                    }
                    ++sequence; // native DWORD ADD wraps before the signed %d
                    char name[100];
                    bindings.format_00412f80(name, "AvoidZoneG %s %d #%03d", "all", layer,
                        static_cast<std::int32_t>(sequence));
                    replace_path_name(*path, name, bindings);
                    bindings.simplify_0041ea30(path, &params);
                    auto* const head = params.paths.head;
                    auto* const added = bindings.buy_node_0041c0b0(
                        &params.paths, head, head->previous, &path);
                    bindings.grow_list_0041fdd0(&params.paths, 1);
                    head->previous = added;
                    added->previous->next = added;
                }
                for (std::int32_t x = 0; x < width; ++x) {
                    bindings.free_00bf6989(columns[x]);
                }
                bindings.free_00bf6989(columns);
            }
        }

        owner = nullptr;
        if (!scan_all || node == nullptr) {
            break;
        }
        node = node->next;
        if (node == nullptr) {
            break;
        }
        if (node->owner != nullptr && owner_accepts_44(node->owner)) {
            owner = node->owner;
        }
    }
}

} // namespace bsp

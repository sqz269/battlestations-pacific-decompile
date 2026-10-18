#include "bsp/oct18_tail_d.hpp"
#include "bsp/cc13_dyn1.hpp"

#include <cstddef>

namespace bsp::oct18_tail_d {
namespace {
template <class T>
T& field(void* object, std::size_t offset) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(object) + offset);
}
template <class T>
const T& field(const void* object, std::size_t offset) noexcept {
    return *reinterpret_cast<const T*>(static_cast<const std::uint8_t*>(object) + offset);
}
static_assert(sizeof(void*) == 4, "Original object layouts require Win32");
static_assert(sizeof(FontIterator) == 8, "Native iterator is two dwords");
}  // namespace

void* camera_mover_tick_element_0042a8a0(void* mover) noexcept {
    return mover ? static_cast<std::uint8_t*>(mover) + 0x170 : nullptr;
}
void camera_mover_focus_entity_0042a8b0(void*, std::uint32_t, std::uint32_t,
    std::uint32_t, std::uint32_t, std::uint32_t) noexcept {}
void camera_mover_focus_point_0042a8c0(void*, std::uint32_t, std::uint32_t,
    std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t) noexcept {}
void camera_mover_adopt_from_0042a8d0(void*, std::uint32_t) noexcept {}
void* camera_mover_target_0042a900(void*) noexcept { return nullptr; }

void entity_inherit_race_and_activate_0042a910(void* entity, EntityActivate activate) noexcept {
    activate(entity);
}

void movie_force_aa_flak_00465570(void* action, void* movie, MoviePartCalls& calls) {
    for (void* part = field<void*>(movie, 0x48); part; part = field<void*>(part, 0x44)) {
        if (!calls.is_kind_of_slot5c(part, 0x1e)) continue;
        if (!calls.is_kind_of_slot5c(part, 0x20)) continue;
        auto* definition = field<void*>(part, 0x3f4);
        const auto type = field<std::int32_t>(definition, 0x80);
        if (type != 5 && type != 1) continue;
        const float filter = field<float>(action, 0x20);
        // FILD int32 / FUCOMIP compares the exact integer, without narrowing it
        // to float first. This matters for IDs above 2^24. NaNs never match.
        const auto id = field<std::int32_t>(part, 0x38c);
        if (static_cast<double>(id) == static_cast<double>(filter) || filter == 0.0f)
            calls.force_aa_flak_slot1d8(part, 0, 0.0f, 0.0f);
    }
}

void binoculars_observer_released_0051e780(void* observer, void* released) noexcept {
    if (field<void*>(observer, 0x38) == released) field<void*>(observer, 0x38) = nullptr;
}

void unit_library_collect_children_00680da0(void* screen, void* collection, CollectionPushBack push_back) {
    push_back(collection, static_cast<std::uint8_t*>(screen) + 0x6c);
}

void font_destroy_00ad53a0(void* font, FontDestroyCalls& calls) {
    FontIterator iterator{font, field<void*>(field<void*>(font, 4), 0)};
    for (;;) {
        auto* head = field<void*>(font, 4);
        if (!iterator.owner || iterator.owner != font) calls.invalid_parameter_00bf6713();
        if (iterator.node == head) break;
        if (!iterator.owner) calls.invalid_parameter_00bf6713();
        if (iterator.node == field<void*>(iterator.owner, 4)) calls.invalid_parameter_00bf6713();
        auto* payload = field<void*>(iterator.node, 0x10);
        if (payload) {
            calls.free_00bf65ac(payload);
            field<void*>(iterator.node, 0x10) = nullptr;
        }
        calls.iterator_increment_00ad40d0(iterator);
    }
    // Reload the alpha owner after the GFX callback, as the image does.
    constexpr std::size_t references[] = {0x64, 0x68};
    for (const auto offset : references) {
        auto* reference = field<void*>(font, offset);
        if (reference) {
            if (calls.interlocked_decrement(static_cast<std::uint8_t*>(reference) + 4) == 0)
                calls.reference_destroy_slot0(reference);
            field<void*>(font, offset) = nullptr;
        }
    }
    // Each pointer and size is captured before obtaining the singleton, because
    // the native caller pushes ReturnBlock's three arguments before that call.
    constexpr std::size_t pointers[] = {0x44, 0x3c, 0x34, 0x2c, 0x24, 0x10};
    for (const auto offset : pointers) {
        auto* block = field<void*>(font, offset);
        if (block) {
            const auto size = field<std::uint32_t>(font, offset - 4) + 1u;
            auto* pool = calls.string_pool_get_00419cc0();
            calls.string_pool_return_00bd1510(pool, block, size, 1);
        }
    }
    auto* head = field<void*>(font, 4);
    FontIterator result{};
    calls.tree_erase_range_00ad4b60(font, result,
        FontIterator{font, field<void*>(head, 0)}, FontIterator{font, head});
    calls.free_00bf65ac(field<void*>(font, 4));
    field<void*>(font, 4) = nullptr;
    field<std::uint32_t>(font, 8) = 0;
}

std::uint32_t resource_interface_type_descriptor_00b8a270() noexcept { return 0x00e13084u; }

void* terrain_shape_construct_00c58840(void* body, void* shape, const void* descriptor) {
    field<std::uint32_t>(shape, 0) = 0x00d7a04cu;
    field<std::uint32_t>(shape, 0x204) = 0;
    field<void*>(shape, 4) = body;
    field<std::uint32_t>(shape, 8) = field<std::uint32_t>(descriptor, 0x10);
    field<float>(shape, 0x24) = field<float>(descriptor, 0);
    field<float>(shape, 0x28) = field<float>(descriptor, 4);
    field<std::uint32_t>(shape, 0x2c) = field<std::uint32_t>(descriptor, 8);
    field<std::uint32_t>(shape, 0x30) = field<std::uint32_t>(descriptor, 0xc);
    field<std::uint32_t>(shape, 0x208) = 0;
    field<std::uint32_t>(shape, 0x20c) = 0;
    field<std::uint32_t>(shape, 0) = 0x00d7a194u;
    field<std::uint32_t>(shape, 0x210) = field<std::uint32_t>(descriptor, 0x5c);
    field<std::uint32_t>(shape, 0x22c) = field<std::uint32_t>(descriptor, 0x60);
    // Forward word copy matches REP MOVSD even if source/destination overlap.
    for (std::size_t i = 0; i < 12; ++i)
        field<std::uint32_t>(shape, 0x34 + 4 * i) = field<std::uint32_t>(descriptor, 0x14 + 4 * i);
    field<float>(shape, 0x21c) = field<float>(descriptor, 0x4c);
    field<float>(shape, 0x220) = field<float>(descriptor, 0x50);
    field<std::uint32_t>(shape, 0x214) = field<std::uint32_t>(descriptor, 0x44);
    field<std::uint32_t>(shape, 0x218) = field<std::uint32_t>(descriptor, 0x48);
    field<float>(shape, 0x224) = static_cast<float>(1.0 / static_cast<double>(field<float>(descriptor, 0x58)));
    field<float>(shape, 0x228) = field<float>(descriptor, 0x54);
    cc13_dyn1::terrain_shape_bounds_00c58690(shape);
    return shape;
}

}  // namespace bsp::oct18_tail_d

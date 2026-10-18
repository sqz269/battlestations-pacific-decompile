#pragma once

#include <cstdint>

// New C++ interfaces for the twelve ordinary oct18-tD entries. All names are
// hypotheses. docs/OCT18_TD_TAIL.md records original ABIs and proof limits.
// Raw object pointers require writable native Win32 layouts. No host is wired.
namespace bsp::oct18_tail_d {

// 0042A8A0: ECX=mover, EAX=mover+170h or zero, RET.
void* camera_mover_tick_element_0042a8a0(void* mover) noexcept;
// Empty virtual bodies; only the original stack widths are established.
// 0042A8B0 RET 14h; 0042A8C0 RET 1Ch; 0042A8D0 RET 4.
void camera_mover_focus_entity_0042a8b0(void*, std::uint32_t, std::uint32_t,
    std::uint32_t, std::uint32_t, std::uint32_t) noexcept;
void camera_mover_focus_point_0042a8c0(void*, std::uint32_t, std::uint32_t,
    std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t) noexcept;
void camera_mover_adopt_from_0042a8d0(void*, std::uint32_t) noexcept;
// 0042A900: EAX=0, RET; no object access.
void* camera_mover_target_0042a900(void*) noexcept;

// 0042A910: JMP 00923840, retaining ECX and the caller's return address.
// Supply the established 00923840 implementation; no duplicate body or host switch.
using EntityActivate = void (*)(void* entity) noexcept;
void entity_inherit_race_and_activate_0042a910(void* entity, EntityActivate activate) noexcept;

struct MoviePartCalls {
    virtual ~MoviePartCalls() = default;
    virtual bool is_kind_of_slot5c(void* part, std::uint32_t kind) = 0;
    virtual void force_aa_flak_slot1d8(void* part, std::uint32_t zero, float a, float b) = 0;
};
// 00465570: ECX=action, stack movie, RET 4. Linked part list at movie+48h;
// exact signed-integer-vs-float comparison, then the zero wildcard at 00D7A218.
void movie_force_aa_flak_00465570(void* action, void* movie, MoviePartCalls& calls);

// 0051E780: ECX=observer, stack released pointer, RET 4.
void binoculars_observer_released_0051e780(void* observer, void* released) noexcept;
// 00680DA0: ECX=screen, stack collection, RET 4. Bind the existing STL operation.
using CollectionPushBack = void (*)(void* collection, void* argument);
void unit_library_collect_children_00680da0(void* screen, void* collection, CollectionPushBack push_back);
struct FontIterator {
    void* owner;
    void* node;
};
// Required bindings, not no-op stand-ins. These preserve native ownership and
// callback order while leaving STL/CRT bodies outside reconstruction scope.
struct FontDestroyCalls {
    virtual ~FontDestroyCalls() = default;
    virtual void invalid_parameter_00bf6713() = 0;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void iterator_increment_00ad40d0(FontIterator& iterator) = 0;
    virtual std::int32_t interlocked_decrement(void* count_at_plus4) = 0;
    virtual void reference_destroy_slot0(void* reference) = 0;
    virtual void* string_pool_get_00419cc0() = 0;
    virtual void string_pool_return_00bd1510(void* pool, void* block,
        std::uint32_t size, std::uint32_t unused_one) = 0;
    virtual void tree_erase_range_00ad4b60(void* font, FontIterator& result,
        FontIterator first, FontIterator last) = 0;
};
// 00AD53A0: ECX=font, RET. Complete normal path through 00AD5586;
// compiler exception cleanup transport is not recreated by this interface.
void font_destroy_00ad53a0(void* font, FontDestroyCalls& calls);

// 00B8A270: EAX=&00E13084, RET. Return an image-address token rather than
// constructing a second descriptor with a different identity.
std::uint32_t resource_interface_type_descriptor_00b8a270() noexcept;

// 00C58840: ECX=body, stack(shape, descriptor), RET 8, EAX=shape.
// Uses the existing 00C58690 bounds body. Normal construction only; the native
// registration for compiler handler 00CC84BB is not installed in the host.
void* terrain_shape_construct_00c58840(void* body, void* shape, const void* descriptor);

}  // namespace bsp::oct18_tail_d

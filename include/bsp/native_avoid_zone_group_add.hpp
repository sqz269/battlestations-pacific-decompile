#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw avoidance group addition requires MSVC Win32.
#endif

namespace bsp {

// Complete caller lowering for 0041D1E0..0041D310 (305B). Original ECX is the
// raw 14h-byte group; one stacked entity word, RET4, no semantic EAX result.
// Source EDX explicitly borrows the ADDRESS of the real world-pointer cell for
// the genuine 0041CCD0 constructor; this is not an Original input or a replacement
// world. All reached raw entity/path/vtable/parent/world storage must be backed.
//
// Path resolution precedes entity+54h==2. Actual incoming entity-cell storage is
// replaced with the allocation and remains the current unwind cleanup operand.
// Current C++ cleanup is active only across construction through returned-count
// read, then disarmed before append/empty destruction. No outside-ctor rollback,
// null repair or extra allocation guard is provided. Real current heap/cleanup
// and the complete raw constructor are mandatory dependencies.
//
// Extra Source context/frames/current C++ EH/CRT differ from Native FH3/SEH/frame
// identity. No Native virtual drop-in, actual entity-factory or gameplay claim.
// Evidence and dependency gate: docs/GOAL15J_AVOID_ZONE_GROUP_ADD.md.
void __fastcall add_native_avoid_zone_group_entity_0041d1e0(
    void* actual_group, void* const volatile* actual_world_pointer_cell,
    void* actual_entity);

} // namespace bsp

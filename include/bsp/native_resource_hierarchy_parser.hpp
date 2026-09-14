#pragma once
#include "bsp/native_resource_stream_reads.hpp"
#include <cstdint>
namespace bsp {
struct NativeResourceHierarchyParserContext {
    NativeResourceStreamReadContext& reads;
    const volatile std::uint32_t& negative_bound_00ce4adc;
    const volatile std::uint32_t& positive_bound_00ce4970;
};
// Complete B7D160: ECX output24h, EDX sphere10h, EAX output, RET. Ordered
// x87 maxima/minima with original float32 staging and SSE bit copies. Caller
// supplies x87 stack space; no floating-point control/status reset occurs.
void* __fastcall build_native_sphere_bounds_00b7d160(void* output,const void* sphere);
// Complete B7D220: ECX output, stack sphere, EAX output, RET4. Use a private
// temporary then six ordered FLD32/FSTP32 copies. Dummy EDX names no input.
void* __fastcall set_native_sphere_bounds_00b7d220(void* output,void* unused_edx,const void* sphere);
// Complete B87AE0: ECX resource, stack raw record pointer, RET4. Grow current
// resource+1C pointer header by16 at count==capacity, append then count++.
// Borrow the existing B87350 allocation domain; no AddRef or record copy.
void append_native_resource_hierarchy_record_00b87ae0(void* resource,void* record);
// Complete B7EB90: ECX manager, stack node handle, RET4. Allocate from the
// already-bound actual hierarchy pool, initialize84h fields except matrix,
// consume ordered field children, and append to CURRENT manager+24 resource.
// Only current child/name temporaries unwind; unpublished record/field
// allocations are not reclaimed by this native body on failure.
void parse_native_resource_hierarchy_item_00b7eb90(void* manager,void* handle,
    NativeResourceHierarchyParserContext&);
// Complete B7F100: ECX manager, stack container handle, RET4. Item tags invoke
// the field parser; unknown children skip/detach. Release child after normal
// dispatch without seeking a recognized child's unread tail. Repeated calls
// append; no parent graph construction or validation is added.
void parse_native_resource_hierarchy_00b7f100(void* manager,void* handle,
    NativeResourceHierarchyParserContext&);
// Actual hierarchy payload84h: parent0, name4/8, matrixC..48,
// resource-index header4C/50/54, flags58, sphere5C..68, bounds6C..80.
// Slot+84 belongs to the existing88h pool and is never initialized here.
// New source integration: original private-stack/EH aliases, native FH3/SEH,
// unmasked faults and game admission remain outside the validated contract.
}

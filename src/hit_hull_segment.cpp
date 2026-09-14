// Reconstruction of the hit record's producer side: where record+34h gets a
// real index and who builds the part-hit array. See docs/HIT_HULL_SEGMENT.md
// for the evidence behind every rule; the comments here only cite the site.
#include "bsp/hit_hull_segment.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// 00723E90, the segment producer.
// ---------------------------------------------------------------------------

bool segment_hit_writes_record_00723e90(const GeomElementHit* element) noexcept
{
    // 00723F17 MOV ESI,EAX / 00723F19 TEST ESI,ESI / 00723F1B JNZ. A null
    // element takes XOR AL,AL at 00723F1E and returns without a single store.
    return element != nullptr;
}

void apply_segment_hit_00723e90(HitRecordFill& record,
                                const GeomElementHit* element,
                                const void* element_address,
                                const HitQueryPoint& world_hit_point) noexcept
{
    if (!segment_hit_writes_record_00723e90(element)) {
        return;
    }

    record.position = world_hit_point; // 00723F44, 00723F4F, 00723F5A
    record.shape_kind = element->kind;        // 00723F5F, 00723F62
    record.source_record = element_address;   // 00723F68, MOV [EAX+38h],ESI
    record.hull_segment = element->part_index; // 00723F65, 00723F6C
}

// ---------------------------------------------------------------------------
// 00723F80 + 006D2E30, the part-hit array producer.
// ---------------------------------------------------------------------------

HitPartEntry make_part_hit_00723f80(const GeomElementHit& element, float distance) noexcept
{
    HitPartEntry entry{};
    entry.kind = element.kind;            // 00724078, 0072407C
    entry.part_index = element.part_index; // 007240A4, 007240B1
    // entry+8h is MOV dword ptr [ESP+1Ch],0 at 00724044 and has no field here.
    entry.distance = distance; // 0072403E, 0072404C: the out float 00723B70 wrote
    return entry;
}

int grow_part_hit_capacity_006d2e30(int capacity) noexcept
{
    // LEA EAX,[EAX+EAX*1+2] at 006D2E3B. No overflow guard in the native.
    return capacity * 2 + 2;
}

PartHitAppendPlan plan_part_hit_append_006d2e30(int count, int capacity) noexcept
{
    PartHitAppendPlan plan{};
    plan.new_capacity = capacity;
    plan.copy_count = 0;
    plan.write_index = count;
    plan.new_count = count + 1; // ADD dword ptr [ESI+40h],1 at 006D2EC9

    // 006D2E36 CMP [ESI+40h],EAX / 006D2E39 JNZ: the grow path runs only on
    // exact equality, so a count above the capacity appends past the buffer
    // exactly as the native does. That is reproduced, not corrected.
    if (count == capacity) {
        plan.reallocates = true;
        plan.new_capacity = grow_part_hit_capacity_006d2e30(capacity);
        // 006D2E42 SHL EAX,4 then operator_new; the copy loop at 006D2E56
        // moves [ESI+40h] entries, which on this path is the old capacity.
        plan.new_byte_size = static_cast<std::size_t>(plan.new_capacity) * 0x10u;
        plan.copy_count = count;
    }

    return plan;
}

// ---------------------------------------------------------------------------
// The gate the produced index meets in 0092D1F0.
// ---------------------------------------------------------------------------

PartIndexReach part_index_reach_0092d1f0(int part_index, signed char gate_byte,
                                         int health_slot_count) noexcept
{
    PartIndexReach reach{};
    // 0092D210 JL on the signed byte at parts+34Ch+index.
    reach.gate_byte_allows = gate_byte >= 0;
    // The inlined bounds check compares the index as unsigned against
    // (parts+314h - parts+310h) / 4 and falls into 00BF6713 when it fails.
    reach.within_health_vector =
        part_index >= 0 && health_slot_count > 0 && part_index < health_slot_count;
    return reach;
}

} // namespace bsp

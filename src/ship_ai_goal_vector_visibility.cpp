#include "bsp/ship_ai_goal_vector_visibility.hpp"

#include <algorithm>

namespace bsp {

const ReconUnionRecord* recon_union_find_target_009dfbe0(
    const ReconUnionListNode* head, const void* target) noexcept
{
    // 009DFBE0..009DFBE8: the head is read from slot+0E0Ch and an empty list
    // returns null without touching the argument.
    for (const ReconUnionListNode* node = head; node != nullptr; node = node->next) {
        // 009DFBF0 MOV EDX,[EAX+8]; 009DFBF3 CMP [EDX+4],ECX. The record is
        // dereferenced before the comparison, so a node always carries one.
        const ReconUnionRecord* record = node->record;
        if (record != nullptr && record->entity == target) {
            return record;  // 009DFC04 MOV EAX,EDX
        }
    }
    return nullptr;  // 009DFBFF XOR EAX,EAX
}

bool recon_scan_visits_class_00806480(int class_id) noexcept
{
    return std::find(kReconScannedClassIds.begin(), kReconScannedClassIds.end(),
        class_id) != kReconScannedClassIds.end();
}

bool recon_union_contains_009dfbe0(const ReconUnionMemberFacts& facts) noexcept
{
    if (!facts.present) return false;
    // Rule (a): a class the scan never visits never enters any slot, so it can
    // reach neither the four relation triples nor their union.
    if (!facts.scanned_class) return false;
    // Rule (b): the four gate bytes 008074D5 reads.
    if (!recon_unit_gate_passes_008074d5(facts.gate)) return false;
    // Triple 0 is the own relation and the drain never touches it, so an
    // own-side member is in the union whatever its detection level is.
    if (facts.same_side) return true;
    // Triples 1 and 2 are drained by 00807647: level 0 is dropped outright and
    // level 1 is moved into triple 3. Triple 4 unions 0..3, so anything at
    // blip or above survives into it and nothing below does.
    return static_cast<int>(facts.level) >= static_cast<int>(ReconDetectionLevel::blip);
}

}  // namespace bsp

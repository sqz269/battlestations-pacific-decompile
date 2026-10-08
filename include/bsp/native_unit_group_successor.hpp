#pragma once

namespace bsp {
// Complete raw 0070D8D0..0070D97C: ECX=actual aligned508h group,
// [entryESP+4]=excluded entity, EAX=actual entity/null, RET4. The fastcall
// spelling leaves the unused native EDX register explicit; it is not a
// provider. The native entry returns a pointer, not a floating-point value.
//
// A nonnull exclusion distinct from a nonnull leader returns that leader.
// Otherwise type18 returns null without touching count/column. The scan reads
// signed count before zeroing group+500, skips null/dead(+5D)/excluded members,
// and reloads count after every record advance. It stops at the leader; any
// earlier strict-minimum candidate overrides that stopped-at identity.
//
// Preserve the actual candidate/best FLD, FCOMIP, FSTP, JBE sequence, including
// unordered comparison and ambient x87 effects. There is no float copy,
// alternate arithmetic, FP environment change, or added input validation.
// Actual group records and live entity+5D fields must remain readable; two
// free x87 stack slots are required when comparison is reached. Only +500 is
// written. Class/world integration, faults, unmasked exceptions and concurrent
// mutation are outside the verified ordinary component domain.
void* __fastcall native_unit_group_successor_0070d8d0(
    void* actual_group, void* unused_edx, const void* excluded_entity) noexcept;
} // namespace bsp

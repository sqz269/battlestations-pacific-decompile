#pragma once
#include "bsp/native_spatial_attachment.hpp"
#include "bsp/native_unit_part_storage.hpp"

namespace bsp {
struct NativeUnitPartAttachmentAccess {
    const NativeSpatialAttachmentAccess* spatial;
    void* volatile* manager_publication_01090aa0;
    void* volatile* index_publication_00f8a0d8;
    // Required complete actual-unit virtual dispatch. ECX owner, EDX captured
    // CURRENT slot token. Kind is one stack word; AL alone is the predicate.
    // Do not recapture the token, cache the owner or interpret identity DWORDs
    // as callable rebuilt virtual tables. These dispatchers may change owners.
    std::uint8_t (__fastcall* query_kind_5c)(void* owner,
        std::uint32_t captured_target, std::uint32_t kind);
    void* (__fastcall* collision_node_b0)(void* owner,
        std::uint32_t captured_target);
};
// Complete 70F7D0[101]. Native ECX part, EAX parent collision node, RET;
// EDX adds access. Query type1E, reload owner164/parent3C, call B0 twice when
// first result is nonnull, and test the SECOND node's 158 byte. Reload each
// next parent after callbacks. No cycle/null-second-result recovery is added.
void* __fastcall find_native_unit_part_spatial_parent_0070f7d0(void* actual_part,
    const NativeUnitPartAttachmentAccess*);
// Complete 710AD0[174]. Native ECX part, RET; EDX adds access. Any nonzero184
// is inert. Short-circuit live-owner queries1C/1B/36/44; capture owner before
// pose refresh; capture its matrix and canonical index before parent lookup;
// perform the complete node attachment before setting184. Only low flag byte
// is consumed; native unspecified upper stack bytes have no source meaning.
void __fastcall attach_native_unit_part_00710ad0(void* actual_part,
    const NativeUnitPartAttachmentAccess*);

// Extends the existing real constructor/storage/group/collision/name/entry
// composition with actual spatial attachment. Borrow the same globals and
// actual publication/pose/dispatch domains. Remaining selected-set destruction,
// unit-name and selected-root bindings are inherited requirements.
class NativeUnitPartAttachmentBindings : public NativeUnitPartStorageBindings {
public:
    NativeUnitPartAttachmentBindings(NativeUnitPartCollisionGlobals,
        NativeUnitPartCollisionCallbacks, const NativeUnitPartAttachmentAccess&) noexcept;
    void call_00710ad0(void* actual_part) override;
private:
    const NativeUnitPartAttachmentAccess& access_;
};
} // namespace bsp

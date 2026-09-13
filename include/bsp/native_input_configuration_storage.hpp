#pragma once
#include "bsp/native_input_configuration_cleanup.hpp"

namespace bsp {
// Shared stateless source storage contracts, extracted from the existing raw
// class-configuration implementation. All headers are actual10h checked DWORD
// vectors {opaque,begin,end,capacity}; no shadow vector or header is allocated.
// Existing allocation/free and returning CRT services are preserved.
void clear_input_checked_word_storage(void* actual_header);
void* assign_input_checked_word_storage(void* actual_destination, const void* actual_source);

// Full checked erase boundary: native6977F0 ECX header; stack output8h,
// first-owner, first-position, last-owner, last-position; EAX output, RET14h.
// Validate only nonnull/equal iterator owners (not owner==receiver), retain the
// captured positions, then move the current suffix and write output+4 before0.
void* erase_input_checked_word_storage(void* actual_header, void* actual_output,
    const void* first_owner, void* first_position,
    const void* last_owner, void* last_position);

// Concrete borrowing-free adapter for the cleanup schedule. This owns no data
// or lifetime and forwards both required calls to the same extracted helpers
// used by raw A917E0 class configuration.
class NativeInputConfigurationStorage final : public NativeInputConfigurationStorageCalls {
public:
    void* call_00697bd0(void*, const void*) override;
    void* call_006977f0(void*, void*, const void*, void*, const void*, void*) override;
};

// Source contracts, not ports or original ABI/CRT/EH replacements. Distinct
// owning vectors require valid separate allocations; same-header assignment
// is a no-op. Erase requires first<=last<=current end after returning validation.
// The existing memmove_s copy has the same values/endpoints on these ranges as the
// original forward DWORD loops; malformed ranges, null placement destinations,
// independently owning overlapping vectors and arbitrary stack aliases are not
// claimed equivalent. Storage mutation leaves opaque header0 and capacity alone;
// an explicitly aliased output iterator still writes its two addressed words.
} // namespace bsp

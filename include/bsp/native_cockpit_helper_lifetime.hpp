#pragma once
#include "bsp/native_node_destruction.hpp"
#include <cstddef>

namespace bsp {
// Already-constructed actual 24h storage, with existing typed field lifetimes.
// Binding does not implement B3C800, retain, initialize, or clear any word.
struct NativeCockpitHelperStorage {
    std::uint32_t profile_00;
    std::atomic<std::int32_t> references_04;
    std::uint32_t attached_08, camera_0c, preserved_10, preserved_14;
    std::uint32_t retained_18, preserved_1c, preserved_20;
};
static_assert(sizeof(NativeCockpitHelperStorage) == 0x24);
static_assert(offsetof(NativeCockpitHelperStorage, references_04) == 4);
static_assert(offsetof(NativeCockpitHelperStorage, retained_18) == 0x18);

class NativeCockpitHelperReference;
// Exactly one authoritative host owner view per actual allocation is required.
// This contract adds no competing native-node identity registry.
class NativeCockpitHelperOwner final {
public:
    NativeCockpitHelperOwner(NativeCockpitHelperStorage&, NativeNodeDestructionRuntime&,
        const volatile std::uint32_t* actual_two_word_table_00d61854);
    NativeCockpitHelperOwner(const NativeCockpitHelperOwner&) = delete;
    NativeCockpitHelperOwner& operator=(const NativeCockpitHelperOwner&) = delete;
    enum class Phase { live, destroying, dead };
    NativeCockpitHelperStorage& storage;
    NativeNodeDestructionRuntime& nodes;
    const volatile std::uint32_t* const table;
    Phase phase{Phase::live};
private:
    friend class NativeCockpitHelperReference;
    bool reference_bound_{};
};
// B3C5C0 ECX owner, RET. Full normal branches and C++ exception projection of
// state0 -> base-only cleanup. Unknown nonnull members fail explicitly.
// +18 requires an existing binding whose zero callback resolves CURRENT native
// profile/slot0 after decrement. No concrete nonzero +18 producer is established.
void destroy_native_cockpit_helper_00b3c5c0(NativeCockpitHelperOwner&);
// B3C6C0 ECX owner, stack DWORD flags, RET4, EAX captured allocation.
// flags&1 requires actual storage in the existing BF681B/free domain.
void* delete_native_cockpit_helper_00b3c6c0(NativeCockpitHelperOwner&, std::uint32_t flags);

struct NativeCockpitHelperCompanionDisposal {
    void* context;
    void (*retire)(void*, NativeCockpitHelperReference&) noexcept;
};
// Separate persistent companion over actual +04, with no new reference count.
// Once bound, final zero exclusively owns destruction/free. Direct owner calls
// are forbidden by contract until retirement. Owner/runtime/table/disposal must
// outlive this path. Terminal noexcept may terminate on unsupported providers;
// this is not FH3/SEH, a drop-in ABI replacement, or game validation.
class NativeCockpitHelperReference final : public RenderCommandReference {
public:
    NativeCockpitHelperReference(NativeCockpitHelperOwner&, NativeCockpitHelperCompanionDisposal);
    ~NativeCockpitHelperReference() override;
    void release_zero_references() noexcept override;
private:
    NativeCockpitHelperOwner& owner_;
    NativeCockpitHelperCompanionDisposal disposal_;
    bool retired_{};
};
}

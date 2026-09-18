#pragma once
#include "bsp/native_language_catalog_storage.hpp"
#include <optional>

namespace bsp {
// These explicit source operations borrow the real 0Ch catalog header and its
// pool cells. They do not introduce a second process catalog or native FH3 ABI.
struct NativeLanguageCatalogLifetimeOperation final {
    using Phase=NativeLanguageCatalogStorageOperation::Phase;
    Phase phase{Phase::fresh};
    void* owner{};
    void* current_row{};
    void* released_backing{};
    std::uint32_t initialized_rows{},destroyed_rows{},native_site{};
    std::optional<NativeLanguageCatalogStorageOperation> child;
    ~NativeLanguageCatalogLifetimeOperation();
    // Caller must resolve retained allocations/rows before acknowledging.
    void acknowledge_diagnostic_cleanup() noexcept;
    NativeLanguageCatalogLifetimeOperation()=default;
    NativeLanguageCatalogLifetimeOperation(const NativeLanguageCatalogLifetimeOperation&)=delete;
    NativeLanguageCatalogLifetimeOperation& operator=(const NativeLanguageCatalogLifetimeOperation&)=delete;
};
// 8D5AA0: ECX actual vector, stack signed count, RET4. Signed capacity/count
// comparisons, wrapped slot arithmetic. Grow zeroes each nonzero computed slot.
// Shrink decrements CURRENT count BEFORE destroying the current last row, then
// rechecks it. Publishes requested count last; no negative-count policy added.
void resize_native_language_catalog_008d5aa0(void*,std::int32_t,
    NativeStringRawPoolContext&,NativeLanguageCatalogAllocationCalls&,
    NativeLanguageCatalogLifetimeOperation&);
// CDEEA0: no inputs, RET. Resize actual F88974 catalog to zero, reload its
// backing pointer, then BF6989. Pointer/capacity are deliberately left dead.
void shutdown_native_language_catalog_00cdeea0(void*,NativeStringRawPoolContext&,
    NativeLanguageCatalogAllocationCalls&,NativeLanguageCatalogLifetimeOperation&);
struct NativeLanguageCatalogRegistrationCalls {
    virtual ~NativeLanguageCatalogRegistrationCalls()=default;
    virtual int register_shutdown_00bf6ff5(void (*)());
};
// CD2DA0: no native inputs; atexit(CDEEA0), preserve EAX, RET. The composition
// owner supplies a stable source thunk bound to its actual catalog lifetime.
// Registration alone performs no catalog construction or header stores.
int register_native_language_catalog_shutdown_00cd2da0(
    NativeLanguageCatalogRegistrationCalls&,void (*stable_shutdown)());
} // namespace bsp

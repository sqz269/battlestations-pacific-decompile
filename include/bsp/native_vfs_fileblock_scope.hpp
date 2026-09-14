#pragma once

#include <cstdint>
#include <memory>

namespace bsp {
class NativeStringStorage;
struct SingletonLifetimeCallbacks;
struct NativePakRegistryBlockContext;
class NativePakRegistryBlockInvocation;

// Receives the actual captured observer/current virtual entry at the reached
// native call. Unknown implementations must preserve the invocation and all
// borrowed storage if they escape; no default no-op or profile substitution.
class NativeVfsFileBlockObserverDispatch {
public:
    virtual ~NativeVfsFileBlockObserverDispatch() = default;
    virtual void invoke_enter(std::uintptr_t entry, void* actual_observer,
        const void* actual_name, NativePakRegistryBlockInvocation&) = 0;
    virtual void invoke_leave(std::uintptr_t entry, void* actual_observer,
        const void* actual_name, NativePakRegistryBlockInvocation&) = 0;
};

// Concrete D64190 +8/+C path: current targets BB5770/BB5910 call the existing
// complete PakRegistry callbacks. Unsupported reached targets throw. This does
// not add production observer wiring or retain dependency-internal stack locals.
class NativePakRegistryFileBlockObserverDispatch final : public NativeVfsFileBlockObserverDispatch {
public:
    explicit NativePakRegistryFileBlockObserverDispatch(NativePakRegistryBlockContext& context)
        : context_(context) {}
    void invoke_enter(std::uintptr_t, void*, const void*, NativePakRegistryBlockInvocation&) override;
    void invoke_leave(std::uintptr_t, void*, const void*, NativePakRegistryBlockInvocation&) override;
private:
    NativePakRegistryBlockContext& context_;
};

struct NativeVfsFileBlockScopeContext {
    NativeStringStorage& strings;
    const SingletonLifetimeCallbacks& invalid_parameters;
    NativeVfsFileBlockObserverDispatch& observers;
    const char* actual_empty_0109cef0;
    const void* actual_clear_bytes_00ce3a0c;
};

enum class NativeVfsFileBlockScopePhase {
    fresh, allocating_gate, growing_gate_count, linking_gate, notifying,
    writing_name, diagnosing, restoring_gate, erasing_gate, complete, failed
};

// One invocation per explicit entry OR exit; this is not a balanced RAII block.
// Publish before dispatch. Retain on failure, together with caller-owned actual
// manager/name/context storage. Owns captured node/observer fields, stable erase
// output and a nested PakRegistry invocation. It neither frees an allocated node
// on growth failure nor rolls back observer/gate/name/depth effects. Active or
// failed destruction terminates. Nested BDD850/BE1740 and resolver cleanup limits
// remain those documented by BO; outer retention does not retain their locals.
class NativeVfsFileBlockScopeAcquired final {
public:
    NativeVfsFileBlockScopeAcquired();
    ~NativeVfsFileBlockScopeAcquired();
    NativeVfsFileBlockScopeAcquired(const NativeVfsFileBlockScopeAcquired&) = delete;
    NativeVfsFileBlockScopeAcquired& operator=(const NativeVfsFileBlockScopeAcquired&) = delete;
    NativeVfsFileBlockScopePhase phase() const noexcept;
    std::uint32_t active_call_site() const noexcept;
    std::uint32_t failure_site() const noexcept;
    const void* allocated_gate_node() const noexcept;
    const void* actual_erase_output() const noexcept;
    const NativePakRegistryBlockInvocation& observer_invocation() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    friend void enter_native_vfs_fileblock_00be0980(void*, const void*, std::uint8_t,
        NativeVfsFileBlockScopeContext&, NativeVfsFileBlockScopeAcquired&);
    friend void leave_native_vfs_fileblock_00bdc9b0(void*, const void*,
        NativeVfsFileBlockScopeContext&, NativeVfsFileBlockScopeAcquired&);
};

// Complete ordinary BE0980..BE0A28[169]. ECX captured manager, stack(name,gate
// low byte), RET8; no stable result. Allocate before grow/link, AND current gate,
// optionally notify current observer +8, then increment depth and copy name.
void enter_native_vfs_fileblock_00be0980(void* actual_manager, const void* actual_name,
    std::uint8_t gate, NativeVfsFileBlockScopeContext&, NativeVfsFileBlockScopeAcquired&);

// Complete ordinary BDC9B0..BDCA74[197]. ECX captured manager, stack name, RET4.
// Empty name routes current manager name to observer +C. Notify before clear and
// decrement; restore last gate, reread last, erase. No parent-name restoration.
// All three returning BF6713 branches occur after the earlier native effects.
void leave_native_vfs_fileblock_00bdc9b0(void* actual_manager, const void* actual_name,
    NativeVfsFileBlockScopeContext&, NativeVfsFileBlockScopeAcquired&);

// Explicit-service MSVC Win32 source APIs, not native ABI/FH3/SEH replacements.
} // namespace bsp

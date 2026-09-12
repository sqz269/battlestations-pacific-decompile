#pragma once

#include "bsp/sound_lifetime_access.hpp"
#include "bsp/xlive_manager_runtime.hpp"

namespace bsp {

struct XLiveManagerOwner;
class XLiveOwnerAllocation;

// Additional native fields. Supply their preimage; this is not a constructor.
// All fields already present in online/flags/pump/signin remain canonical there.
struct XLiveManagerOwnerStorage {
    std::uint32_t vtable_00;
    std::uint8_t byte_86;
    std::uint8_t byte_87;
    std::uint32_t float_bits_88;
    void* ipc_3ac;
    std::uint8_t byte_3e9;
};

// A4C030/A4BDE0 are the actual local IPC allocation/thread/pipe services, not
// XLive DLL exports. Success must provide the real handle. No substitute is
// supplied here. The owner allocation is supplied by the application caller.
struct XLiveManagerOwnerHost {
    virtual ~XLiveManagerOwnerHost() = default;
    virtual std::int32_t create_ipc_00a4c030(void*& output) = 0;
    virtual void destroy_ipc_00a4bde0(void* handle) = 0;
    virtual void free_owner_storage(XLiveManagerOwner*) noexcept = 0;
};

// One F8ABE8 slot, shared with the application's actual current-manager lookup.
// Derive the current pump context from published_owner()->context. Do not
// publish an independent cached context pointer beside this owner slot.
class XLiveManagerLifetimeAccess final {
public:
    XLiveManagerLifetimeAccess(SingletonLifetimeDomain&,
        XLiveManagerOwner* volatile& published_00f8abe8) noexcept;
    XLiveManagerLifetimeAccess(SoundLifetimeAccess,
        XLiveManagerOwner* volatile& published_00f8abe8) noexcept;
    // Retained semantic-domain API. Raw callers use manager_view_00415350.
    ConcreteSingletonLifetimeManager& manager_00415350();
    SoundLifetimeManagerView manager_view_00415350() const;
    SoundLifetimeAccess lifetime_access() const noexcept { return access_; }
    // Reload publication, then derive its exact registered identity. Raw
    // owners require XLiveOwnerAllocation; semantic owners retain &owner.
    void* published_registration_identity() const;
    XLiveManagerOwner* published_owner() const noexcept;
    void publish(XLiveManagerOwner*) noexcept;
private:
    SingletonLifetimeDomain* semantic_domain_{};
    SoundLifetimeAccess access_;
    XLiveManagerOwner* volatile& published_;
};

// Borrowed projections preserve fields the native constructor never writes.
// context must be the same XLiveManagerRuntime::context() used by the pump;
// signin must be the very storage passed to that runtime. Keep every owner at
// a stable address while registered or while SDK operations remain pending.
// Native-written pointer slots are overwritten without freeing their preimage.
struct XLiveManagerOwner {
    XLiveSystemPumpContext& context;
    XLiveSigninStorage& signin;
    XLiveManagerOwnerStorage& storage;
    XLiveManagerLifetimeAccess& lifetime;
    XLiveManagerOwnerHost& host;
    // Set only by XLiveOwnerAllocation. No independently published slot and
    // no copy of any native field; legacy semantic aggregates omit this.
    XLiveOwnerAllocation* allocation_identity{};
};

// ECX=output slot, RET. Null slot -> E_INVALIDARG; otherwise call actual IPC
// create and store success output or null according to signed HRESULT.
std::int32_t initialize_xlive_ipc_slot_00a4c250(void** slot, XLiveManagerOwnerHost&);
// ECX=handle, RET/tail call. Null and -1 are ignored; slot is not cleared.
void close_xlive_ipc_00a4c280(void* handle, XLiveManagerOwnerHost&);

// ECX=owner, RET. Capture the first manager's section, then re-get the manager
// and reload the published slot for register/unregister. Exception cleanup
// releases the captured section FIRST, then restores root vtable CE3818.
XLiveManagerOwner* construct_xlive_manager_base_00a3f530(XLiveManagerOwner&);
void destroy_xlive_manager_base_00a3f5d0(XLiveManagerOwner&);

// ECX=embedded vector +360, RET. Free retained storage, clear all three slots.
void destroy_xlive_achievement_ids_00a3f840(OnlineSystemState&) noexcept;

// ECX=owner, stack callback20/callback24, RET8. Complete native normal body and
// recovered C++ unwind: vector/base cleanup only if derived initialization
// throws. Uses the real recovered reset and pump. In particular +12C/+14C are
// untouched until the pump returns, then zeroed without freeing +14C.
XLiveManagerOwner* construct_xlive_manager_00a40df0(XLiveManagerOwner&,
    const void* callback_20, const void* callback_24);
// ECX=owner, RET. Close IPC, free storage, destroy ID vector, unregister base.
// Native does not close listener/SDK/Winsock or free the achievement batch.
void destroy_xlive_manager_00a3f9d0(XLiveManagerOwner&);

// ECX=owner, flags stack, RET4. Only flag bit0 frees the supplied owner storage;
// These legacy projection overloads return &owner, even after freeing it.
// XLiveOwnerAllocation overloads return the allocation identity. Exceptions propagate
// before free. The borrowed field owners are never implicitly destroyed here.
XLiveManagerOwner* delete_xlive_manager_00a3fdc0(XLiveManagerOwner&, std::uint8_t flags);
XLiveManagerOwner* delete_xlive_manager_base_00a3f670(XLiveManagerOwner&, std::uint8_t flags);

} // namespace bsp

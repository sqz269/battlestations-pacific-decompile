#pragma once
#include <cstdint>
#include <exception>
#include <memory>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Icon delayed request requires MSVC Win32.
#endif
namespace bsp {
struct NativeStringRawPoolContext;
struct NativeFileStoreFactoryContext;
struct NativeFileStoreProviderLifetimeContext;
struct NativeFileStoreRequestContext;
class NativeFileStoreRequestAcquired;
class NativeCrtStricmpCallees;
class NativeGuiIconDelayedRequestStorageFrame;
struct NativeGuiIconDelayedRequestStorageContext;

// Opaque address-only view of one genuine Runtime-owned FileStore call.
// No writable publication accessor or Native ownership is exported. Actual
// Request/Resolver records and dispatch belong to this distinct stable node.
class NativeGuiIconDelayedRequestFileInvocation {
public:
    enum class Phase { fresh, entered, normal_returned, unknown_source_escape };
    virtual ~NativeGuiIconDelayedRequestFileInvocation() = default;
    NativeGuiIconDelayedRequestFileInvocation(const NativeGuiIconDelayedRequestFileInvocation&) = delete;
    NativeGuiIconDelayedRequestFileInvocation& operator=(const NativeGuiIconDelayedRequestFileInvocation&) = delete;
    Phase phase() const noexcept { return phase_; }
    std::uint32_t active_native_site() const noexcept { return site_; }
protected:
    NativeGuiIconDelayedRequestFileInvocation(NativeFileStoreFactoryContext& f,
        NativeFileStoreProviderLifetimeContext& p, NativeFileStoreRequestContext& r,
        NativeFileStoreRequestAcquired& a) noexcept
        : factory_(f), provider_(p), request_(r), acquired_(a) {}
    // Source metadata admission only. Real Runtime serializes this node with
    // its other request/pump/retirement entries; no Native payload is read.
    virtual void validate_source_entry() = 0;
private:
    friend void request_native_gui_icon_delayed_loads_storage_00ab5f00(void*,
        volatile std::uint32_t (&)[3], NativeGuiIconDelayedRequestStorageFrame&,
        NativeGuiIconDelayedRequestStorageContext&);
    NativeFileStoreFactoryContext& factory_;
    NativeFileStoreProviderLifetimeContext& provider_;
    NativeFileStoreRequestContext& request_;
    NativeFileStoreRequestAcquired& acquired_;
    Phase phase_{Phase::fresh};
    std::uint32_t site_{};
    std::exception_ptr failure_;
};
using PrepareNativeGuiIconDelayedRequestFileCall =
    NativeGuiIconDelayedRequestFileInvocation& (*)(void* actual_runtime_provider);

struct NativeGuiIconDelayedRequestStorageContext {
    NativeStringRawPoolContext& strings;
    const volatile std::uint32_t& feature_word_0109eea4;
    const volatile std::int32_t& locale_changed_0109de1c;
    NativeCrtStricmpCallees& stricmp_callees;
    void* file_provider;
    // Cold fresh Runtime-owned call only. No Native work, header copy, private
    // char-path record reuse or caller-provided accepted verdict.
    PrepareNativeGuiIconDelayedRequestFileCall prepare_file_call;
};

class NativeGuiIconDelayedRequestStorageFrame final {
public:
    enum class Phase { fresh, entered, normal_returned, unknown_source_escape };
    struct Observation {
        Phase phase{Phase::fresh};
        void* actual_singleton{};
        const volatile std::uint32_t* argument_home_identity{};
        std::uint32_t active_native_site{}, escaped_native_site{};
        NativeGuiIconDelayedRequestFileInvocation* file_call{};
        void* factory{};
        void* provider{};
        bool duplicate{}, request_returned{}, request_result{}, append_returned{};
        bool temporary_returned{}, parameter_returned{};
        std::exception_ptr failure;
    };
    // Metadata allocation only; user-provided native storage constructor
    // seeds no temporary/argument/append/state/preserve Native words.
    NativeGuiIconDelayedRequestStorageFrame();
    ~NativeGuiIconDelayedRequestStorageFrame();
    NativeGuiIconDelayedRequestStorageFrame(const NativeGuiIconDelayedRequestStorageFrame&) = delete;
    NativeGuiIconDelayedRequestStorageFrame& operator=(const NativeGuiIconDelayedRequestStorageFrame&) = delete;
    const Observation& observation() const noexcept { return observation_; }
private:
    friend void request_native_gui_icon_delayed_loads_storage_00ab5f00(void*,
        volatile std::uint32_t (&)[3], NativeGuiIconDelayedRequestStorageFrame&,
        NativeGuiIconDelayedRequestStorageContext&);
    struct Storage;
    std::unique_ptr<Storage> storage_;
    Observation observation_;
};

// Whole NORMAL [00AB5F00,00AB6070), inclusive end606F,368B/124starts.
// Original ECX singleton10/[LEN,DATA,opaque owner]/RET0C becomes this ordinary
// Source ABI. Incoming actual three DWORD HOMEs remain mutable/addressable.
// Queue10/entries0C, all subobject/allocator/string/physical storage, captured
// receiver lifetime and actual CRT/VFS/callback/worker custody are caller proof.
// No semantic queue, owner retain, profile/count/phase birth or accepted bool.
// The name is consumed on every normal path, including duplicate/rejection.
// UNKNOWN Source escapes retain all remaining temp/parameter/queue/request debt;
// no guessed Native FH3 cleanup, rollback/retry or disposal permission. Original
// ABI/FLAGS/fault/FH3/SEH/NoThrow/Origin/whole graph/runtime/activation stay held.
void request_native_gui_icon_delayed_loads_storage_00ab5f00(void* actual_singleton10,
    volatile std::uint32_t (&actual_argument_homes)[3],
    NativeGuiIconDelayedRequestStorageFrame&, NativeGuiIconDelayedRequestStorageContext&);
} // namespace bsp

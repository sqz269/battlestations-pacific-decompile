#pragma once
#include <cstdint>
#include <exception>
#include <memory>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Icon delivery requires MSVC Win32.
#endif
namespace bsp {
class NativeStringStorage;
class NativeAdoptedSubstreamDispatch;
struct SingletonLifetimeCallbacks;
struct NativeFileStoreFactoryContext;
struct NativeFileStoreProviderLifetimeContext;
struct NativeTextureCacheContext;
struct NativeGuiIconTextureApplyStorageContext;
class NativeGuiIconDelayedLoads;
class NativeGuiIconDelayedDeliveryStorageFrame;
struct NativeGuiIconDelayedDeliveryStorageContext;

// Address-only view of one genuine Runtime-owned footer invocation. Its own
// provider context/operation must not borrow a request, cancel or legacy record.
class NativeGuiIconDelayedDeliveryFileInvocation {
public:
    enum class Phase { fresh, entered, normal_returned, unknown_source_escape };
    virtual ~NativeGuiIconDelayedDeliveryFileInvocation() = default;
    NativeGuiIconDelayedDeliveryFileInvocation(const NativeGuiIconDelayedDeliveryFileInvocation&) = delete;
    NativeGuiIconDelayedDeliveryFileInvocation& operator=(const NativeGuiIconDelayedDeliveryFileInvocation&) = delete;
    Phase phase() const noexcept { return phase_; }
    std::uint32_t active_native_site() const noexcept { return site_; }
protected:
    NativeGuiIconDelayedDeliveryFileInvocation(NativeFileStoreFactoryContext& f,
        NativeFileStoreProviderLifetimeContext& p, NativeStringStorage& s,
        NativeAdoptedSubstreamDispatch& d, const SingletonLifetimeCallbacks& v) noexcept
        : factory_(f), provider_(p), strings_(s), streams_(d), invalid_(v) {}
    virtual void validate_source_entry() = 0; // Source history only; no Native reads.
private:
    friend void deliver_native_gui_icon_delayed_storage_00ab51b0(void*, volatile std::uint32_t (&)[2],
        NativeGuiIconDelayedDeliveryStorageFrame&, NativeGuiIconDelayedDeliveryStorageContext&);
    NativeFileStoreFactoryContext& factory_;
    NativeFileStoreProviderLifetimeContext& provider_;
    NativeStringStorage& strings_;
    NativeAdoptedSubstreamDispatch& streams_;
    const SingletonLifetimeCallbacks& invalid_;
    std::uint32_t remove_home_; // Ordinary Native HOME, no constructor seed.
    Phase phase_{Phase::fresh};
    std::uint32_t site_{};
    std::exception_ptr failure_;
};
using PrepareNativeGuiIconDelayedDeliveryFileCall =
    NativeGuiIconDelayedDeliveryFileInvocation& (*)(void* actual_runtime_provider);

struct NativeGuiIconDelayedDeliveryStorageContext {
    NativeGuiIconTextureApplyStorageContext& apply;
    NativeTextureCacheContext& cache;
    // Mandatory genuine process cell for exact resize/BF7680; no default or
    // mapped numeric-VA substitute. Values are sampled by the reached callees.
    const volatile std::uint32_t& feature_word_0109eea4;
    void* file_provider;
    PrepareNativeGuiIconDelayedDeliveryFileCall prepare_file_call;
};
class NativeGuiIconDelayedDeliveryStorageFrame final {
public:
    enum class Phase { fresh, entered, normal_returned, unknown_source_escape };
    struct Observation {
        Phase phase{Phase::fresh};
        void* actual_singleton{};
        volatile std::uint32_t* argument_homes{};
        const void* second_name{};
        std::uint32_t active_native_site{}, escaped_native_site{};
        NativeGuiIconDelayedDeliveryFileInvocation* file_call{};
        void* factory{};void* provider{};
        bool decrement_entered{},decrement_returned{},zero_entered{},zero_returned{};
        long decrement_result{};
        std::uint32_t zero_profile{},zero_target{};
        std::exception_ptr failure;
    };
    NativeGuiIconDelayedDeliveryStorageFrame(); // No Native home seeds.
    ~NativeGuiIconDelayedDeliveryStorageFrame(); // Metadata after caller-proven last use only.
    NativeGuiIconDelayedDeliveryStorageFrame(const NativeGuiIconDelayedDeliveryStorageFrame&)=delete;
    NativeGuiIconDelayedDeliveryStorageFrame& operator=(const NativeGuiIconDelayedDeliveryStorageFrame&)=delete;
    const Observation& observation() const noexcept { return observation_; }
private:
    friend void deliver_native_gui_icon_delayed_storage_00ab51b0(void*, volatile std::uint32_t (&)[2],
        NativeGuiIconDelayedDeliveryStorageFrame&, NativeGuiIconDelayedDeliveryStorageContext&);
    struct Storage;std::unique_ptr<Storage> storage_;Observation observation_;
};
struct NativeGuiIconDelayedCompletionStorageContext {
    NativeGuiIconDelayedLoads& delayed;
    NativeGuiIconDelayedDeliveryStorageContext& delivery;
};
class NativeGuiIconDelayedCompletionStorageFrame final {
public:
    enum class Phase { fresh, entered, normal_returned, unknown_source_escape };
    struct Observation {
        Phase phase{Phase::fresh};
        volatile std::uint32_t* callback_homes{};
        void* actual_singleton{};
        std::uint32_t active_native_site{},escaped_native_site{};
        bool getter_entered{},getter_returned{},delivery_entered{},delivery_returned{};
        std::exception_ptr failure;
    };
    NativeGuiIconDelayedCompletionStorageFrame(); // No Native home seeds.
    ~NativeGuiIconDelayedCompletionStorageFrame();
    NativeGuiIconDelayedCompletionStorageFrame(const NativeGuiIconDelayedCompletionStorageFrame&)=delete;
    NativeGuiIconDelayedCompletionStorageFrame& operator=(const NativeGuiIconDelayedCompletionStorageFrame&)=delete;
    const Observation& observation() const noexcept { return observation_; }
private:
    friend void complete_native_gui_icon_file_loaded_storage_00ab58d0(volatile std::uint32_t (&)[2],
        NativeGuiIconDelayedCompletionStorageFrame&, NativeGuiIconDelayedCompletionStorageContext&);
    struct Storage;std::unique_ptr<Storage> storage_;Observation observation_;
};

// Whole NORMAL AB51B0..5358 inclusive (425B/155starts), exclusive5359.
// ECX singleton/two name8-header stack pointers/RET8 -> ordinary Source API.
// Actual homes are borrowed and read at reached epochs, first only after the
// footer factory getter. Owner08 remains an unretained opaque actual Icon138.
void deliver_native_gui_icon_delayed_storage_00ab51b0(void* actual_singleton10,
    volatile std::uint32_t (&actual_argument_homes)[2], NativeGuiIconDelayedDeliveryStorageFrame&,
    NativeGuiIconDelayedDeliveryStorageContext&);
// Whole NORMAL AB58D0..58E8 inclusive (25B/8starts), exclusive58E9. Capture
// second then first; stage BOTH before the actual fresh private service getter.
// No stream/file/Icon argument, semantic queue, numeric code-pointer call or
// registered_identity promotion. Full graph/library/current-thread/OS/owner
// custody remains caller proof. Unknown Source escape retains backing/debt;
// no guessed Native cleanup, replay, ABI/FH3/SEH/fault/Origin/runtime theorem.
void complete_native_gui_icon_file_loaded_storage_00ab58d0(volatile std::uint32_t (&actual_callback_homes)[2],
    NativeGuiIconDelayedCompletionStorageFrame&, NativeGuiIconDelayedCompletionStorageContext&);
} // namespace bsp

#pragma once

#include "bsp/movie_factory_followup.hpp"
#include "bsp/sound_lifetime_access.hpp"

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Game movie factory composition requires MSVC Win32.
#endif

namespace bsp {
class BinkRuntimeLibrary;
struct BinkFactoryImports;
}

namespace bsp::game {
class GameNativeReadOnlyData;

// Required matched-heap service. Root binds the actual existing
// singleton_lifetime_allocate({object, bytes, bytes}) / singleton_lifetime_free
// pair. There is no default allocator, private lifetime domain or null provider.
// allocate returns raw DWORD-aligned storage, without a C++ metadata wrapper.
struct GameMovieFactoryAllocation {
    virtual ~GameMovieFactoryAllocation() = default;
    virtual void* allocate_00bf681b(std::size_t bytes) = 0;
    virtual void free_00bf65ac(void* captured_allocation) noexcept = 0;
};

struct GameMovieFactoryObservation {
    enum class Phase {
        fresh, allocating, constructing, ready, null_result, retiring,
        retired, destroyed_retained, failed
    };
    Phase phase{Phase::fresh};
    void* captured_allocation{}; // Identity only after raw_free_returned.
    std::uint32_t native_site{};
    std::uint32_t deletion_flags{};
    bool allocation_returned{}, registration_entered{}, publication_written{};
    bool registration_returned{}, sound_entered{}, sound_returned{};
    bool retirement_entered{}, unregister_returned{}, publication_cleared{};
    bool raw_free_entered{}, raw_free_returned{};
};

// Stable source composition metadata, not the native factory representation.
// The native allocation is exactly the separate 4-byte MovieFactoryPrefix.
// Root supplies SAME actual01090AA0 and actualF8AEFC cells, validated original
// read-only token storage, the actual matched heap and one retained fully bound
// thirteen-import Bink owner. All dependencies outlive this owner and raw drain.
class GameMovieFactoryOwner final : private MovieFactoryBootstrapHost {
public:
    GameMovieFactoryOwner(void* volatile& actual_manager_01090aa0,
        void* volatile& actual_factory_00f8aefc, BinkRuntimeLibrary&,
        GameNativeReadOnlyData&, GameMovieFactoryAllocation&);
    // Metadata only: no native deletion, free, retry, drain or terminate call.
    // Precondition: explicit flags-1 retirement completed, or construction was
    // never entered/null. Failed/overlapping operations and flag0 storage require
    // root to retain the WHOLE aggregate/providers until OS exit instead.
    ~GameMovieFactoryOwner() noexcept override;
    GameMovieFactoryOwner(const GameMovieFactoryOwner&) = delete;
    GameMovieFactoryOwner& operator=(const GameMovieFactoryOwner&) = delete;
    GameMovieFactoryOwner(GameMovieFactoryOwner&&) = delete;
    GameMovieFactoryOwner& operator=(GameMovieFactoryOwner&&) = delete;

    // One explicit Startup invocation at0073DF9B. Allocator exceptions precede
    // acquired cleanup. The existing original null branch skips constructor
    // effects; the real root allocator is not replaced by a null-success path.
    // C++ constructor/register/sound exceptions free ONLY captured raw storage,
    // latch terminal failure and rethrow. Reached publication/registry effects
    // remain, possibly dangling: root MUST prevent ordinary shared drain.
    MovieFactoryPrefix* construct_in_startup_0073df9b();

    // Root dispatches genuine current profiles CFEA78/CFEA60 to these entries.
    // Receiver must be this owner's actual allocation; current F8AEFC may differ.
    // No current-publication shutdown dispatch is supplied here. Calls must be
    // serialized after construction, once only; no reentrant delete/retry.
    // Normal unregister uses CURRENT publication, clears it, then flags bit0
    // frees captured incoming storage. Flag0 keeps storage and grants no retry.
    void* delete_factory_00737950(void* actual_captured_receiver, std::uint32_t flags);
    void* delete_base_factory_00737030(void* actual_captured_receiver, std::uint32_t flags);

    const GameMovieFactoryObservation& observation() const noexcept { return observation_; }
    bool requires_process_retention() const noexcept;
    // SAME borrowed library table, never a copy/private table or module token.
    const BinkImports& decoder_imports() const noexcept { return decoder_imports_; }

private:
    void register_movie_factory_007366b0(MovieFactoryPrefix&) override;
    void set_bink_direct_sound_system_00a4d356(std::uint32_t argument) override;
    void* delete_captured(void*, std::uint32_t flags, std::uint32_t native_site);
    void free_captured() noexcept;

    SoundLifetimeAccess lifetime_;
    void* volatile& publication_;
    const BinkImports& decoder_imports_;
    const BinkFactoryImports& factory_imports_;
    GameMovieFactoryAllocation& allocation_;
    GameMovieFactoryObservation observation_{};
};
} // namespace bsp::game

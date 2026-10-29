#include "bsp/game_movie_factory_owner.hpp"
#include "bsp/bink_runtime_library.hpp"
#include "bsp/game_native_readonly_data.hpp"

#include <cstring>
#include <new>
#include <stdexcept>

namespace bsp::game {
namespace {
constexpr std::uint32_t base_profile = 0x00ce3818u;
constexpr std::uint32_t registration_profile = 0x00cfea60u;
constexpr std::uint32_t factory_profile = 0x00cfea78u;

void require_original_table_word(GameNativeReadOnlyData& original,
    std::uint32_t address, std::uint32_t expected) {
    std::uint32_t word;
    std::memcpy(&word, original.data_at(address, sizeof word), sizeof word);
    if (word != expected)
        throw std::invalid_argument("Movie factory original-image table identity mismatch");
}
} // namespace

GameMovieFactoryOwner::GameMovieFactoryOwner(void* volatile& actual_manager_01090aa0,
    void* volatile& actual_factory_00f8aefc, BinkRuntimeLibrary& library,
    GameNativeReadOnlyData& original, GameMovieFactoryAllocation& allocation)
    : lifetime_(actual_manager_01090aa0), publication_(actual_factory_00f8aefc),
      decoder_imports_(library.imports()), factory_imports_(library.factory_imports()),
      allocation_(allocation) {
    static_assert(sizeof(MovieFactoryPrefix) == 4);
    static_assert(sizeof(void*) == 4);
    // These are original image WORDS, not host vtable/function-pointer casts.
    // Metadata preparation performs no allocation, publication or Bink call.
    require_original_table_word(original, base_profile, 0x00412440u);
    require_original_table_word(original, registration_profile, 0x00737030u);
    require_original_table_word(original, factory_profile, 0x00737950u);
    require_original_table_word(original, factory_profile + 4u, 0x00737730u);
}

GameMovieFactoryOwner::~GameMovieFactoryOwner() noexcept = default;

bool GameMovieFactoryOwner::requires_process_retention() const noexcept {
    using Phase = GameMovieFactoryObservation::Phase;
    return observation_.phase == Phase::allocating || observation_.phase == Phase::constructing ||
        observation_.phase == Phase::retiring || observation_.phase == Phase::failed ||
        observation_.phase == Phase::destroyed_retained;
}

void GameMovieFactoryOwner::free_captured() noexcept {
    observation_.raw_free_entered = true;
    allocation_.free_00bf65ac(observation_.captured_allocation);
    observation_.raw_free_returned = true;
}

MovieFactoryPrefix* GameMovieFactoryOwner::construct_in_startup_0073df9b() {
    using Phase = GameMovieFactoryObservation::Phase;
    if (observation_.phase != Phase::fresh) {
        observation_.phase = Phase::failed;
        throw std::logic_error("Movie factory construction cannot be replayed");
    }
    observation_.phase = Phase::allocating;
    observation_.native_site = 0x0073df9du;
    void* captured;
    try {
        captured = allocation_.allocate_00bf681b(4);
    } catch (...) {
        // Caller state21 is not armed during the allocation call.
        observation_.phase = Phase::failed;
        throw;
    }
    observation_.captured_allocation = captured;
    observation_.allocation_returned = true;
    if (!captured) {
        observation_.native_site = 0x0073dfd4u;
        observation_.phase = Phase::null_result;
        return nullptr;
    }
    observation_.phase = Phase::constructing;
    try {
        // Trivial default initialization starts lifetime without zeroing bytes.
        auto* const owner = ::new (captured) MovieFactoryPrefix;
        observation_.native_site = 0x0073dfbcu;
        construct_movie_factory_00737710(*owner, *this);
        observation_.native_site = 0x0073dfd4u;
        observation_.phase = Phase::ready;
        return owner;
    } catch (...) {
        // 00DB61C8 ->00C86BEC: captured raw free only. No publication load,
        // unregister, clear, virtual destructor, sound reset or rollback.
        observation_.phase = Phase::failed;
        free_captured();
        throw;
    }
}

void GameMovieFactoryOwner::register_movie_factory_007366b0(MovieFactoryPrefix& owner) {
    observation_.registration_entered = true;
    observation_.native_site = 0x007366d8u;
    owner.vtable_00 = registration_profile;
    try {
        {
            CapturedSoundLifetimeSection captured_section(lifetime_);
            observation_.native_site = 0x00736706u;
            publication_ = &owner;
            observation_.publication_written = true;
            observation_.native_site = 0x0073670cu;
            auto manager = lifetime_.get_manager_00415350();
            observation_.native_site = 0x0073671au;
            // Resolve the manager in the COMPLETED preceding statement, THEN
            // evaluate this actual volatile cell as the registration argument.
            manager.register_object(publication_);
        }
        observation_.registration_returned = true;
    } catch (...) {
        // Helper state1 guard unwinds first; state0 action only resets the
        // incoming profile. F8AEFC/manager effects remain exactly as reached.
        owner.vtable_00 = base_profile;
        throw;
    }
}

void GameMovieFactoryOwner::set_bink_direct_sound_system_00a4d356(std::uint32_t argument) {
    observation_.sound_entered = true;
    observation_.native_site = 0x0073dfcdu;
    (void)factory_imports_.set_sound_system(factory_imports_.open_direct_sound, argument);
    observation_.sound_returned = true;
}

void* GameMovieFactoryOwner::delete_captured(void* actual_receiver,
    std::uint32_t flags, std::uint32_t native_site) {
    using Phase = GameMovieFactoryObservation::Phase;
    if (observation_.phase != Phase::ready ||
        actual_receiver != observation_.captured_allocation || !actual_receiver) {
        observation_.phase = Phase::failed;
        throw std::logic_error("Movie factory deletion requires its live captured receiver once");
    }
    observation_.phase = Phase::retiring;
    observation_.retirement_entered = true;
    observation_.deletion_flags = flags;
    observation_.native_site = native_site;
    auto* const captured = static_cast<MovieFactoryPrefix*>(actual_receiver);
    try {
        captured->vtable_00 = registration_profile;
        {
            CapturedSoundLifetimeSection captured_section(lifetime_);
            observation_.native_site = 0x007367a6u;
            auto manager = lifetime_.get_manager_00415350();
            observation_.native_site = 0x007367b4u;
            manager.unregister_object(publication_);
            observation_.unregister_returned = true;
            publication_ = nullptr;
            observation_.publication_cleared = true;
        }
        captured->vtable_00 = base_profile;
        if ((flags & 1u) != 0) {
            observation_.native_site = native_site == 0x00737950u ? 0x00737960u : 0x00737040u;
            free_captured();
            observation_.phase = Phase::retired;
        } else {
            observation_.phase = Phase::destroyed_retained;
        }
        return captured;
    } catch (...) {
        // 00736750 state1 leaves the captured guard; state0 (00C85FA0)
        // then resets the incoming base profile. It does not clear or free.
        captured->vtable_00 = base_profile;
        observation_.phase = Phase::failed;
        throw;
    }
}

void* GameMovieFactoryOwner::delete_factory_00737950(void* receiver, std::uint32_t flags) {
    return delete_captured(receiver, flags, 0x00737950u);
}
void* GameMovieFactoryOwner::delete_base_factory_00737030(void* receiver, std::uint32_t flags) {
    return delete_captured(receiver, flags, 0x00737030u);
}
} // namespace bsp::game

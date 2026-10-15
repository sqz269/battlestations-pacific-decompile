#include "bsp/movie_factory_followup.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <new>

namespace bsp {

static_assert(sizeof(NativeMovieDecoder) == 0x38);
static_assert(offsetof(NativeMovieDecoder, state) == 0x10);

MovieFactoryPrefix& construct_movie_factory_00737710(
    MovieFactoryPrefix& owner, MovieFactoryBootstrapHost& host) {
    host.register_movie_factory_007366b0(owner);
    owner.vtable_00 = 0x00cfea78u;
    host.set_bink_direct_sound_system_00a4d356(0);
    return owner;
}

MovieDecoderState* create_movie_decoder_00737730(std::uint32_t unused_stack_slot) {
    (void)unused_stack_slot;
    void* storage = singleton_lifetime_allocate({SingletonAllocationKind::object,
        0x38, sizeof(NativeMovieDecoder)});
    if (storage == nullptr) {
        return nullptr;
    }
    // Begin the object's C++ lifetime; the established constructor sets its fields.
    auto* decoder = ::new (storage) NativeMovieDecoder;
    construct_movie_decoder_00a4c660(*decoder); // hidden complete-object flag 1
    // Native reads the file vbtable's signed +4 entry (10h for this constructor).
    // The established constructor always returns its nonnull input owner.
    return &decoder->state;
}

} // namespace bsp

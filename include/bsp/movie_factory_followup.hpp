#pragma once

#include "bsp/movie_decoder.hpp"

#include <cstdint>

namespace bsp {

// The only prefix read or written by 00737710. The complete factory-object
// allocation size and the callable contents of its vtable are not projected.
struct MovieFactoryPrefix {
    std::uint32_t vtable_00;
};

struct MovieFactoryBootstrapHost {
    virtual ~MovieFactoryBootstrapHost() = default;
    // Required existing helper 007366B0: install base table 00CFEA60, publish
    // this exact owner at 00F8AEFC, then look up the shared lifetime manager
    // again and register the reloaded publication under the captured section
    // guard. No replacement body.
    virtual void register_movie_factory_007366b0(MovieFactoryPrefix&) = 0;
    // Required Bink boundary: BinkSetSoundSystem(BinkOpenDirectSound, argument).
    // 00A4D356 jumps through IAT 00CE23D0. The callback at native 00A4D35C
    // jumps through IAT 00CE23D4 (_BinkOpenDirectSound@4). Ignore the result.
    virtual void set_bink_direct_sound_system_00a4d356(std::uint32_t argument) = 0;
};

// Complete 00737710 body: original ECX=owner, EAX=owner, plain RET; no stack
// arguments. The recorded vtable word is an evidence ID, not callable code.
MovieFactoryPrefix& construct_movie_factory_00737710(
    MovieFactoryPrefix&, MovieFactoryBootstrapHost&);

// Complete 00737730 body. Original ECX is unused, one DWORD stack slot is
// ignored (RET4), EAX is the shared decoder base or null. This typed API uses
// the existing 00BF681B host CRT allocation service and complete-object
// 00A4C660(flag=1) reconstruction. It returns allocation+10h, never allocation.
// Native SEH metadata and drop-in register/stack ABI are not supplied.
MovieDecoderState* create_movie_decoder_00737730(std::uint32_t unused_stack_slot);

} // namespace bsp

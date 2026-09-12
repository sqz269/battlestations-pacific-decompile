#pragma once

#include "bsp/sound_system_owner.hpp"

namespace bsp {
struct SoundSystemShutdownContext;

// Exact four words returned by global01090AB0 vtable+14 at00A887F5. Their
// interpretation belongs to the time service; the constructor only copies them.
class SoundStartupClockHost {
public:
    virtual ~SoundStartupClockHost() = default;
    virtual std::array<std::uint32_t, 4> sample_time_14() = 0;
};

// Ordered normal path and base unwind of 00A88770. Native ECX=178h owner,
// stack=(disabled byte,
// two unused words), EAX=this, RET0C. Inputs must satisfy the fresh-construction
// preconditions of sound_system_owner.hpp. auxiliary_owner receives ownership of
// the separately allocated global00F8BBE8 object and must outlive the registered
// lifetime entry. shutdown must bind this same lifetime/canonical state and its
// services must outlive the call. After A81480 completes, exceptions destroy
// only the base (A816B0): completed auxiliary registration, live FMOD handles,
// and an already-assigned +54 resource owner survive, as in DEC460's unwind map.
// Keep their storage alive for explicit host recovery; do not run the derived
// destructor on the failed owner. Failure inside A81480 remains its own contract.
// Factories free partial auxiliary/resource storage; the original allocator
// BF681B throws on exhaustion. Native defensive null branches and raw allocation
// identities are not projected by the throwing C++ factories. Original object
// layout and SEH ABI are not supplied. Evidence: docs/SOUND_STARTUP_UNWIND.md.
// The resource loader, clock, Lua owner and library interfaces are real required
// dependencies, with no fallback implementations. Complete teardown is separate.
SoundSystemOwner& construct_sound_system_00a88770(SoundSystemOwner&,
    SoundOwnerLifetimeBindings&, std::unique_ptr<SoundAuxiliaryTreeOwner>& auxiliary_owner,
    bool sound_disabled, std::uint32_t unused_08, std::uint32_t unused_0c,
    SoundStartupClockHost&, SoundResourceLoadHost&, FmodStartupHost&,
    SoundConfigurationFmodHost&, SoundConfigurationLuaOwner&,
    SoundSystemShutdownContext&);

} // namespace bsp

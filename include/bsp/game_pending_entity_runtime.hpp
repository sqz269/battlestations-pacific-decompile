#pragma once

#include "bsp/native_pending_entity_owners.hpp"

namespace bsp::game {

// The single process-lifetime raw owner pair representing F899A8/F899B4.
// This accessor only returns those same bytes: no allocation, native list
// initialization, callback registration, or per-host owner is introduced.
// Zero-filled pre-startup storage is not an initialized sentinel ring. Future
// queue hosts must borrow this pair after the explicit startup calls below.
NativePendingEntityOwners& game_pending_entity_owners() noexcept;

// Explicit process startup composition of the complete R owner wrappers.
// Invoke ONCE in native ascending CRT table order: CCD6A0 at CE2BAC, CD2D80
// at CE3054, destroy CD3910 at CE30C0, then kill CD3940 at CE30C4. Other table
// entries remain the startup integrator's responsibility. These functions do
// not install a second implicit C++ initializer or a repeated-startup guard.
// Each returns the actual std::atexit status unchanged. Failure keeps the
// initialized owner, as in the native wrapper; no callback is manufactured.
int initialize_game_pending_destroy_owner_00cd3910();
int initialize_game_pending_kill_owner_00cd3940();

// Successful registrations use real CRT atexit with fixed noncapturing exit
// functions, which destroy these SAME process-static owner bytes. There is no
// borrowed registration context or nontrivial C++ owner destructor to expire
// first. Relative exit order is kill CDF4B0, then destroy CDF4A0. The caller
// must keep the process quiescent and use intact rings at shutdown; payloads
// remain borrowed. No manual/duplicate teardown or module unload is supported.
// Source lifetime composition only: startup/frame wiring, producers, drains,
// event delivery, original ABI/EH and gameplay are outside this component.
} // namespace bsp::game

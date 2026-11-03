#pragma once

#include "bsp/random_threads.hpp"

#include <stdexcept>

namespace bsp::game {

// Borrow the SAME RandomThreads created by GameStartupHost::random_threads_initialize
// and registered by WinMain before mission construction. This header owns no
// generator, seed, registration or per-mission reset. Startup clears the borrow
// before destroying its owner; consumers must finish before startup shutdown.
inline RandomThreads* g_borrowed_startup_random = nullptr;

inline void publish_startup_random(RandomThreads* owner) noexcept {
    g_borrowed_startup_random = owner;
}

inline RandomThreads& require_startup_random() {
    if (g_borrowed_startup_random == nullptr)
        throw std::logic_error("Gunnery random draw requires the existing startup RandomThreads");
    return *g_borrowed_startup_random;
}

}  // namespace bsp::game

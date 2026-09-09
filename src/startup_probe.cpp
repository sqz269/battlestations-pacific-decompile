#include "bsp/random_threads.hpp"
#include <iostream>

int main() {
    bsp::RandomThreads random;
    auto* fallback = &random.state_00bd2ed0(bsp::RandomStream::primary);
    random.register_current_00bd2fe0();
    auto* primary = &random.state_00bd2ed0(bsp::RandomStream::primary);
    auto* secondary = &random.state_00bd2ed0(bsp::RandomStream::secondary);
    std::cout << "BSP reconstructed random-subsystem startup\n";
    if (primary == fallback || primary == secondary) return 1;
    for (int i = 0; i < 5; ++i)
        std::cout << "draw " << i << ": " << random.next_00bd2fc0(bsp::RandomStream::primary) << '\n';
    random.unregister_current_00bd3050();
    if (&random.state_00bd2ed0(bsp::RandomStream::primary) != fallback) return 2;
    std::cout << "Registration, distinct streams, fallback restoration and shutdown completed.\n"
                 "This probe does not initialize the game engine.\n";
}

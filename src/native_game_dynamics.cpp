#include "bsp/native_game_dynamics.hpp"
namespace bsp {
void set_native_dyn_world_callback_owner_00c31a40(void* world,void* callback_owner) noexcept {
    static_assert(sizeof(void*)==4);
    *reinterpret_cast<void* volatile*>(static_cast<unsigned char*>(world)+0x24)=callback_owner;
}
} // namespace bsp

// Goal 5 chain 743: retained storage for 008EA250's category-list elements.
// Original constructor ECX=record, EAX=record, RET; copy ECX=destination,
// stack source, EAX=destination, RET4. These remain C++ service interfaces.
#include "bsp/game_hosts_powerups.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/main_menu_selection_listener.hpp"
#include "bsp/powerup_config.hpp"
#include "bsp/powerup_seg58_argon.hpp"

#include <cstring>
#include <stdexcept>

namespace bsp::game_powerups {
namespace {
// Borrow the existing process pool; no second manager or string pool is made.
class ModifierStorageBinding final : public powerup_seg58_argon::PowerupModifierStorageHost {
public:
    float& global_f32(std::uint32_t address) override {
        // 008E6AC0 MOV constants. Original CE4970 bytes F9 02 15 50 = 1e10f.
        if (address == 0x00d7a24cu) return one_;
        if (address == 0x00ce4970u) return end_;
        throw std::logic_error("powerup modifier unexpected constant");
    }
    void* native_string_from_int_004260b0(void* output, std::int32_t value) override {
        bsp::native_string_from_int_004260b0(*static_cast<NativeString*>(output), value,
            game::game_native_string_process().strings());
        return output;
    }
    void native_string_resize_0041dd40(void* string, std::uint32_t length,
        std::uint8_t preserve) override {
        resize_native_string_header_0041dd40(string,
            game::game_native_string_process().raw_context(), length, preserve != 0);
    }
    void* memcpy_00bf7680(void* dst, const void* src, std::uint32_t bytes) override {
        return std::memmove(dst, src, bytes);
    }
private:
    float one_{1.0f};
    float end_{1.0e10f};
};
}

PowerupModifier::PowerupModifier() {
    ModifierStorageBinding storage;
    try {
        powerup_seg58_argon::powerup_modifier_construct_008e6ac0(storage, this);
    } catch (...) {
        name_20.release_to(game::game_native_string_process().strings());
        throw;
    }
}

PowerupModifier::PowerupModifier(const PowerupModifier& source) : cls(source.cls) {
    ModifierStorageBinding storage;
    try {
        powerup_seg58_argon::powerup_modifier_copy_construct_008e72b0(storage, this, &source);
    } catch (...) {
        name_20.release_to(game::game_native_string_process().strings());
        throw;
    }
}

PowerupModifier& PowerupModifier::operator=(const PowerupModifier& source) {
    if (this != &source) {
        name_20.release_to(game::game_native_string_process().strings());
        ModifierStorageBinding storage;
        powerup_seg58_argon::powerup_modifier_copy_construct_008e72b0(storage, this, &source);
        cls = source.cls;
    }
    return *this;
}

PowerupModifier::~PowerupModifier() {
    name_20.release_to(game::game_native_string_process().strings());
}

void PowerupModifier::bind_class(const PowerupClassConfig& source) {
    // bsp.py show 008EA250: 008EA35D..008EA38D resizes +20h, then copies
    // class.name's length bytes. The class pointer remains host metadata.
    name_20.copy_from_00be0a30_fragment(game::game_native_string_process().strings(), source.name_00);
    cls = &source;
}
} // namespace bsp::game_powerups

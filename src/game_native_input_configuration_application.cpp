#include "bsp/game_native_input_configuration_application.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_native_input_settings_application.hpp"
#include "bsp/game_native_lua_services.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_game_settings_owner.hpp"
#include "bsp/native_input_action_owner.hpp"
#include "bsp/native_input_configuration_cleanup.hpp"
#include "bsp/native_input_configuration_load.hpp"
#include "bsp/native_input_configuration_owner.hpp"
#include "bsp/native_input_configuration_parse.hpp"
#include "bsp/native_input_configuration_runtime.hpp"
#include "bsp/native_input_configuration_storage.hpp"
#include "bsp/native_input_keyboard_apply.hpp"
#include "bsp/native_input_keyboard_storage.hpp"
#include "bsp/native_input_settings_defaults.hpp"
#include "bsp/native_lua_service_bindings.hpp"
#include <array>
#include <cstddef>
#include <cstring>
#include <stdexcept>

namespace bsp::game {
namespace {
// Image constants, read from the executable's .rdata (bytes checked with
// `bsp.py ghidra bytes 00d7a200` / `00d7a240`): D7A208 00000080 (-0.0f),
// D7A218 00000000 (0.0f), D7A24C 0000803F (1.0f), D7A250 000000000000F0BF
// (-1.0), D7A260 000080BF (-1.0f).
template<class T> const volatile T& image(const GameNativeReadOnlyData& data, std::uintptr_t address) {
    return *static_cast<const volatile T*>(data.data_at(address, sizeof(T)));
}
// One byte of the raw BCh settings owner published at 00F88980.
const volatile std::uint8_t& settings_byte(NativeGameSettingsStorage& s, std::size_t offset) {
    return *reinterpret_cast<const volatile std::uint8_t*>(s.data() + offset);
}

// [00F8BBFC]. Its one writer is OnInitOnce 004DD71B (MOV [00F8BBFC],006965A0),
// which installs the deadline callback over the native game's clock (game+64Ch)
// and its map at 00E18A7C. This host builds no native game or map, so nothing
// in this process writes the cell and it keeps its zero BSS value; the tick at
// A92C40 then skips the call (`if (callback)`), as the image does before
// 004DD71B. A non-zero identity cannot arrive here, so dispatch is an error.
class UnwrittenPostTickCallback final : public GameInputActionCallbacks {
public:
    void post_tick(std::uint32_t identity) override {
        static_cast<void>(identity);
        throw std::logic_error("[00F8BBFC] has no writer in this process");
    }
};

template<class T> T word(const std::byte* p, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, p + offset, sizeof value);
    return value;
}
// A checked 10h vector header {opaque, begin, end, capacity}: element count.
std::uint32_t header_count(const std::byte* configuration, std::size_t header, std::size_t element) {
    const auto begin = word<std::uintptr_t>(configuration, header + 4);
    const auto end = word<std::uintptr_t>(configuration, header + 8);
    return begin ? static_cast<std::uint32_t>((end - begin) / element) : 0u;
}
} // namespace

struct GameNativeInputConfigurationApplication::Impl {
    enum class Phase { constructed, loading, loaded, failed, retired };
    GameNativeInputConfigurationBindings b;
    const volatile std::uint32_t post_tick_callback_00f8bbfc{0};
    UnwrittenPostTickCallback no_post_tick;
    GameInputActions actions;
    NativeInputConfigurationStorage storage_calls;
    NativeInputConfigurationCleanupContext cleanup;
    NativeInputConfigurationLoadServices loading;
    NativeInputConfigurationParseServices parsing;
    NativeInputKeyboardStorage keyboard_library;
    NativeInputKeyboardApplyContext keyboard_context;
    NativeInputKeyboardApplication keyboard;
    NativeInputSettingsDefaultsServices defaults;
    NativeInputConfigurationRuntimeServices runtime;
    // The native game is operator-new storage; this host's explicit preimage
    // for the bytes 00698680 leaves untouched (4CCh, vector opaque words, Lua
    // constructor preimages, 521h..523h) is zero, as in GameInputSettingsStackPolicy.
    // game+3Ch: 4-byte aligned, as the original span inside the game is.
    alignas(4) std::array<std::byte, 0x524> configuration{};
    Phase phase{Phase::constructed};
    std::uint32_t loads{};

    explicit Impl(GameNativeInputConfigurationBindings bindings)
        : b(bindings),
          actions(b.input, GameInputActionServices{b.crt, b.binding_constants, b.timing,
              b.zero_00d7a218, post_tick_callback_00f8bbfc, no_post_tick}),
          cleanup{b.input.action_context(), b.input.records_context(), storage_calls},
          // [00F88A30] = settings+B0h (XboxCompatibilityMode, 00D15C38).
          loading{cleanup, b.strings, b.lua.bootstrap(), b.lua.binding().files(),
              settings_byte(b.game_settings, 0xb0)},
          // [00F889C1] = settings+41h (swapSticks), [00F889C4] = +44h (swapMapSticks).
          parsing{loading, actions.configuration_context(), b.crt_sse2_conversion,
              settings_byte(b.game_settings, 0x41), settings_byte(b.game_settings, 0x44),
              image<double>(b.data, 0x00d7a250)},
          keyboard_library(b.strings),
          // Stack preimages: explicit source policy zero (not native stack identity).
          keyboard_context{b.input.action_context(), b.input.records_context().bindings,
              b.backend_00f8bbf4, keyboard_library,
              {image<float>(b.data, 0x00d7a218), image<float>(b.data, 0x00d7a24c),
                  image<float>(b.data, 0x00d7a260), image<float>(b.data, 0x00d7a208),
                  image<double>(b.data, 0x00d7a250)},
              0u, 0u, 0u},
          keyboard(keyboard_context),
          defaults{b.input.action_context(), b.input.records_context().bindings,
              b.backend_00f8bbf4, keyboard, 0u, 0u},
          runtime{parsing, actions.tick_context(), b.settings.context(), defaults} {
        // 004DDBE2: LEA ECX,[game+3Ch]; CALL 00698680.
        construct_native_input_configuration_00698680(configuration.data());
    }

    void load() {
        if (phase == Phase::failed || phase == Phase::retired || phase == Phase::loading)
            throw std::logic_error("input configuration is not loadable in its current phase");
        phase = Phase::loading;
        try {
            NativeLuaServiceBindings::Activation activation(b.lua.binding());
            load_native_input_configuration_00698a10(configuration.data(), runtime);
        } catch (...) {
            phase = Phase::failed;
            throw;
        }
        phase = Phase::loaded;
        ++loads;
        const std::byte* const c = configuration.data();
        void* const owner = get_native_input_action_owner_004bec00(b.input.action_context());
        b.log.notef("native input configuration 00698A10 load %u: byte520=%u lua_opened_4c8=%u "
            "rows_4d0=%u words_4e0=%u words_4f0=%u words_500=%u words_510=%u actions=%u",
            loads, static_cast<unsigned>(word<std::uint8_t>(c, 0x520)),
            static_cast<unsigned>(word<std::uint8_t>(c, 0x4c8)),
            header_count(c, 0x4d0, 0x10), header_count(c, 0x4e0, 4), header_count(c, 0x4f0, 4),
            header_count(c, 0x500, 4), header_count(c, 0x510, 4),
            owner ? word<std::uint32_t>(static_cast<const std::byte*>(owner), 8) : 0u);
    }
};

GameNativeInputConfigurationApplication::GameNativeInputConfigurationApplication(
    GameNativeInputConfigurationBindings bindings)
    : impl_(std::make_unique<Impl>(bindings)) {}

GameNativeInputConfigurationApplication::~GameNativeInputConfigurationApplication() = default;

bool GameNativeInputConfigurationApplication::load_if_unloaded() {
    // 004DD739 CMP byte [ESI+55Ch],0; JNZ 004DD74A; 004DD742 LEA ECX,[ESI+3Ch].
    if (loaded_520()) return false;
    impl_->load();
    return true;
}

void GameNativeInputConfigurationApplication::load() { impl_->load(); }

void GameNativeInputConfigurationApplication::retire() {
    auto& p = *impl_;
    if (p.phase == Impl::Phase::retired) return;
    if (p.phase == Impl::Phase::failed || p.phase == Impl::Phase::loading) {
        // An interrupted load leaves partial native state; keep the span as is.
        p.b.log.note("native input configuration retained: interrupted 00698A10 load");
        p.phase = Impl::Phase::retired;
        return;
    }
    {
        NativeLuaServiceBindings::Activation activation(p.b.lua.binding());
        destroy_native_input_configuration_004dceb0(p.configuration.data());
    }
    p.phase = Impl::Phase::retired;
    p.b.log.notef("native input configuration 004DCEB0 retired after %u load(s)", p.loads);
}

bool GameNativeInputConfigurationApplication::loaded_520() const noexcept {
    return word<std::uint8_t>(impl_->configuration.data(), 0x520) != 0;
}

std::uint32_t GameNativeInputConfigurationApplication::loads() const noexcept { return impl_->loads; }
} // namespace bsp::game

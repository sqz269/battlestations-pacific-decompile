#include "bsp/game_native_input_configuration_application.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_native_input_settings_application.hpp"
#include "bsp/game_native_lua_services.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_string_process.hpp"
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
#include "bsp/native_soldier_loop_lengths.hpp"
#include "bsp/settings_seg29_cc13.hpp"
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

// 00444BE0 is the same string/float map specialization for keyboard
// sensitivity and SoldierClass LoopLengths: key at node+0Ch, float+14h,
// color/nil+18h/+19h. Use the complete existing raw-tree body on the actual
// keyboard map, rather than the storage provider's abbreviated insert path.
class KeyboardRawFloatLibrary final : public NativeInputKeyboardStorage {
public:
    explicit KeyboardRawFloatLibrary(NativeStringStorage& strings)
        : NativeInputKeyboardStorage(strings),
          raw_strings_(game_native_string_process().raw_context()) {
        // Other keyboard operations and cleanup still borrow this bridge.
        // Admit identical publication/gate/manager cells, not matching values.
        const auto* const bridge = dynamic_cast<ActualNativeStringPoolStorage*>(&strings);
        if (!bridge || !bridge->uses_raw_pool_cells(raw_strings_))
            throw std::logic_error("keyboard float map requires the canonical raw string-pool domain");
    }
    float* multiplier_00444be0(void* tree, const void* key) override {
        // Empty callbacks select the body's native CRT invalid-parameter
        // boundary; no alternate allocation or tree-validation service.
        return get_or_insert_native_soldier_loop_length_00444be0(
            tree, key, raw_strings_, {});
    }
private:
    NativeStringRawPoolContext& raw_strings_;
};
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
    KeyboardRawFloatLibrary keyboard_library;
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

    void set_device_flags(std::uint8_t flag_4c9, std::uint8_t flag_4ca, std::uint8_t flag_4cb,
        std::uint8_t flag_4cc) {
        if (phase == Phase::failed || phase == Phase::retired || phase == Phase::loading)
            throw std::logic_error("input configuration device flags set in an unusable phase");
        // 00699B80's two calls on the same ECX: 00698730 (cleanup context as the
        // load prefix uses) then the whole 00698A10 through load().
        struct Hooks final : cc13_seg29::InputConfigurationHooks {
            explicit Hooks(Impl& owner) : p(owner) {}
            void clear_00698730(void* c) override {
                if (c != p.configuration.data()) throw std::logic_error("00698730 on a foreign configuration");
                clear_native_input_configuration_00698730(c, p.cleanup);
            }
            void load_00698a10(void* c) override {
                if (c != p.configuration.data()) throw std::logic_error("00698A10 on a foreign configuration");
                p.load();
            }
            Impl& p;
        } hooks(*this);
        const std::uint32_t before = loads;
        cc13_seg29::input_configuration_set_device_flags_00699b80(configuration.data(),
            flag_4c9, flag_4ca, flag_4cb, flag_4cc, hooks);
        b.log.notef("native input configuration 00699B80 device flags 4C9..4CC=%u/%u/%u/%u reload=%d",
            static_cast<unsigned>(flag_4c9), static_cast<unsigned>(flag_4ca),
            static_cast<unsigned>(flag_4cb), static_cast<unsigned>(flag_4cc), loads != before ? 1 : 0);
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

GameInputRuntime& GameNativeInputConfigurationApplication::input_runtime() noexcept {
    return impl_->b.input;
}

void GameNativeInputConfigurationApplication::update_actions(float seconds) {
    auto& p = *impl_;
    if (p.phase != Impl::Phase::loaded)
        throw std::logic_error("native action frame requires the loaded input configuration");
    p.actions.update(seconds);
}

void GameNativeInputConfigurationApplication::set_device_flags_004b46e0(bool swap_sticks_41,
    bool invert_camera_y_42, bool invert_plane_y_43, bool swap_map_sticks_44) {
    // 004B46E0 (CG_adjustor_thunk_004b46e0): ADD ECX,3Ch; JMP 00699B80, from
    // game to its embedded configuration, which here is the owned span.
    // 008D5B9C..008D5BAF push settings bytes +44h,+43h,+42h,+41h, so +41h is
    // the first argument (configuration +4C9h) and +44h the last (+4CCh).
    impl_->set_device_flags(swap_sticks_41 ? 1u : 0u, invert_camera_y_42 ? 1u : 0u,
        invert_plane_y_43 ? 1u : 0u, swap_map_sticks_44 ? 1u : 0u);
}

bool GameNativeInputConfigurationApplication::retire() {
    auto& p = *impl_;
    if (p.phase == Impl::Phase::retired) return true;
    if (p.phase == Impl::Phase::failed || p.phase == Impl::Phase::loading) {
        // An interrupted load leaves partial native state (an opened Lua owner
        // at +0, partly filled vectors, actions on the shared owner). Neither run
        // 004DCEB0 over it nor let the owner free the span: report retention so
        // the application exits before the shared drain, as the sibling
        // interrupted owners (input settings, settings loader) do.
        p.b.log.note("native input configuration retained: interrupted 00698A10 load");
        return false;
    }
    {
        NativeLuaServiceBindings::Activation activation(p.b.lua.binding());
        destroy_native_input_configuration_004dceb0(p.configuration.data());
    }
    p.phase = Impl::Phase::retired;
    p.b.log.notef("native input configuration 004DCEB0 retired after %u load(s)", p.loads);
    return true;
}

bool GameNativeInputConfigurationApplication::loaded_520() const noexcept {
    return word<std::uint8_t>(impl_->configuration.data(), 0x520) != 0;
}

std::uint32_t GameNativeInputConfigurationApplication::loads() const noexcept { return impl_->loads; }
} // namespace bsp::game

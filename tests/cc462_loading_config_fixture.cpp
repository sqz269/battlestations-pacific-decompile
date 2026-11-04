// Source ownership/publication fixture; no game execution or native ABI claim.
#include "bsp/game_loading_config.hpp"
#include "bsp/frontend_state_machine.hpp"
#include "bsp/native_singleton_destruction.hpp"
#include "bsp/native_singleton_vector_leaves.hpp"

#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
std::string text(const bsp::frontend_seg19_21_cc13::NativeStr& value) {
    return value.length ? std::string(value.data, value.length) : std::string{};
}
void check(const bsp::game::GameLoadingConfig& owner,
    const std::vector<std::string>& expected, const char* name, bool side) {
    for (const auto* record : {&owner.pending(), &owner.current()}) {
        const auto count = record->begin ? record->end - record->begin : 0;
        require(static_cast<std::size_t>(count) == expected.size(), "wrong published text count");
        for (std::size_t i = 0; i < expected.size(); ++i)
            require(text(record->begin[i]) == expected[i], "wrong published text bytes");
        require(text(record->name) == name, "wrong published mission title");
        require(record->second_side_enabled == static_cast<std::uint8_t>(side), "wrong side flag");
    }
    if (!expected.empty()) {
        require(owner.current().begin != owner.pending().begin, "configs share vector backing");
        require(owner.current().begin[0].data != owner.pending().begin[0].data,
            "configs share pooled string backing");
    }
    if (*name)
        require(owner.current().name.data != owner.pending().name.data, "configs share title backing");
}

// One concrete risk in chain545: normal teardown unregisters twice, whereas
// canonical shutdown pops the registered secondary before calling its deleter.
// Exercise both on the same raw manager and live projected screen.
void check_loading_lifetime() {
    using namespace bsp;
    using namespace bsp::frontend_seg19_21_cc13;
    void* volatile manager{};
    LoadingScreen* published{};
    LoadingScreen screen;
    FrontEndScreenTable table;
    LoadingScreenLifetimeContext context{SoundLifetimeAccess(manager), published};
    struct Teardown final : LoadingScreenDestructionHost {
        LoadingScreen& screen;
        LoadingScreenLifetimeContext& context;
        FrontEndScreenTable& table;
        int stage{};
        Teardown(LoadingScreen& s, LoadingScreenLifetimeContext& c, FrontEndScreenTable& t)
            : screen(s), context(c), table(t) {}
        void remove_loading_page() override {
            require(stage++ == 0 && context.singleton == &screen, "page removal order/publication");
        }
        void mark_loading_screen_destroyed() override {
            require(stage++ == 1, "game flag write order");
        }
        void destroy_loading_frontend_base() override {
            require(stage++ == 2 && context.singleton == nullptr, "base destruction before singleton clear");
            unregister_front_end_screen_004f71a0(table, screen.base);
        }
    } teardown(screen, context, table);
    const auto live_registrations = [&]() {
        void** slots{};
        std::memcpy(&slots, static_cast<const unsigned char*>(manager) + 4, sizeof slots);
        const auto count = count_native_singleton_slots_00bcf910(manager, nullptr);
        unsigned live{};
        for (std::uint32_t i = 0; i != count; ++i) {
            if (slots[i] != nullptr) {
                require(slots[i] == &screen.lifetime, "registered a shadow instead of the live member");
                ++live;
            }
        }
        return live;
    };
    screen.base = {true, true};
    screen.progress = 0.375f;
    screen.progress_units = 48;
    screen.field_38 = screen.field_3c = screen.field_40 = screen.field_44 = 99;
    require(loading_screen_construct_0057c1c0(screen, context) == &screen,
        "constructor did not return the live screen");
    require(published == &screen && live_registrations() == 1, "constructor publication/registration");
    require(!screen.base.wanted && !screen.base.active && screen.native_vtable == 0x00CEF264 &&
        screen.lifetime.native_vtable == 0x00CEF28C, "constructor base/leaf identities");
    require(screen.progress == 0.375f && screen.progress_units == 48, "constructor reset Init-owned progress");
    require(screen.field_38 == 0 && screen.field_3c == 0 && screen.field_40 == 1 && screen.field_44 == 0,
        "constructor scalar fields");
    const auto id = loading_screen_get_screen_id_0057c230();
    register_front_end_screen_004f71d0(table, screen.base, id);
    context.lifetime.get_manager_00415350().unregister_object(&screen.lifetime);
    loading_screen_destruct_0057c2e0(screen, context, teardown);
    require(teardown.stage == 3 && published == nullptr && live_registrations() == 0 &&
        table.slots[id] == nullptr && screen.lifetime.native_vtable == 0x00CE3818,
        "normal loading screen retirement");

    teardown.stage = 0;
    loading_screen_construct_0057c1c0(screen, context);
    register_front_end_screen_004f71d0(table, screen.base, id);
    LoadingScreenDeletionBinding binding{&teardown, &screen.lifetime,
        [](void* value, void* hook, std::uint32_t flags) {
            auto& target = *static_cast<Teardown*>(value);
            require(hook == &target.screen.lifetime && flags == 1, "drain receiver/flags");
            loading_screen_destruct_0057c2e0(target.screen, target.context, target);
        }};
    NativeSingletonDeletionBindings bindings{};
    bindings.loading_screen = &binding;
    destroy_native_singleton_manager_00bd0400(manager, bindings);
    require(teardown.stage == 3 && published == nullptr && table.slots[id] == nullptr &&
        count_native_singleton_slots_00bcf910(manager, nullptr) == 0, "canonical loading screen drain");
    singleton_lifetime_free(manager);
    manager = nullptr;
}
} // namespace

int main() {
    try {
        bsp::game::GameLoadingConfig owner(bsp::game::game_native_string_process());
        // A source edit after publication must not affect either retained copy.
        std::vector<std::string> source{"mp.loading_24", "mp.loading_05"};
        std::string title = "mission.first";
        owner.publish(source, title, true);
        source[0] = "changed";
        title = "changed";
        check(owner, {"mp.loading_24", "mp.loading_05"}, "mission.first", true);
        // Exercise retained backing, replacement growth and clear on the same
        // owners rather than only checking an initially empty destination.
        owner.publish({"short"}, "second", false);
        check(owner, {"short"}, "second", false);
        owner.publish({"a", "b", "c"}, "third", true);
        check(owner, {"a", "b", "c"}, "third", true);
        owner.publish({}, {}, false);
        check(owner, {}, "", false);
        check_loading_lifetime();
        std::puts("chain462 loading config: retained, independent copies and replacement passed");
        std::puts("chain545 loading lifetime: live member, normal retirement and canonical raw drain passed");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "chain462 loading config: %s\n", error.what());
        return 1;
    }
}

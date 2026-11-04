// Source ownership/publication fixture; no game execution or native ABI claim.
#include "bsp/game_loading_config.hpp"

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
        std::puts("chain462 loading config: retained, independent copies and replacement passed");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "chain462 loading config: %s\n", error.what());
        return 1;
    }
}

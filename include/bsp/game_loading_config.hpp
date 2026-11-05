#pragma once

#include "bsp/frontend_seg19_21_cc13.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_render_service_vector_cleanup.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_vector_assignment.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/seg10_cc13.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace bsp::game {

// The image's actual checked-vector/native-string/byte layout. The unidentified
// +0 word is not consumed by either publisher or the accepted vector services.
// Evidence: bsp.py show 0057CFF0, 0057D060, 0057D0D0 and 0058BDF0 (BE9A..BEDE).
struct GameLoadingConfigRecord {
    void* opaque{};
    frontend_seg19_21_cc13::NativeStr* begin{};
    frontend_seg19_21_cc13::NativeStr* end{};
    frontend_seg19_21_cc13::NativeStr* capacity_end{};
    frontend_seg19_21_cc13::NativeStr name{};
    std::uint8_t second_side_enabled{};
};
static_assert(sizeof(void*) == 4);
static_assert(offsetof(GameLoadingConfigRecord, begin) == 4);
static_assert(offsetof(GameLoadingConfigRecord, name) == 0x10);
static_assert(offsetof(GameLoadingConfigRecord, second_side_enabled) == 0x18);

// Retained Source authorities for Original E08780/E0879C. These are genuine
// mutable config owners, not fixed-address mappings or original binary ABI.
// Both publishers deep-copy through the application's real pooled-string
// domain. They outlive the borrowed source headers and the mission-start call.
class GameLoadingConfig final {
public:
    explicit GameLoadingConfig(GameNativeStringProcess& strings) noexcept
        : strings_(strings.raw_context()) {}
    GameLoadingConfig(const GameLoadingConfig&) = delete;
    GameLoadingConfig& operator=(const GameLoadingConfig&) = delete;
    ~GameLoadingConfig() {
        if (temporary_present_) release(temporary_);
        release(current_);
        release(pending_);
    }

    void publish(const std::vector<std::string>& text, std::string_view mission_name,
        bool second_side_enabled) {
        // The selected mission's actual side-loading strings and title supply
        // every byte. Only read-only native headers are borrowed; no string
        // ownership is transferred out of the mission record.
        std::vector<frontend_seg19_21_cc13::NativeStr> entries;
        entries.reserve(text.size());
        for (const auto& value : text) {
            entries.push_back({static_cast<std::uint32_t>(value.size()),
                const_cast<char*>(value.c_str())});
        }
        GameLoadingConfigRecord source;
        if (!entries.empty()) {
            source.begin = entries.data();
            source.end = entries.data() + entries.size();
            source.capacity_end = source.end;
        }
        source.name = {static_cast<std::uint32_t>(mission_name.size()),
            const_cast<char*>(mission_name.data())};
        source.second_side_enabled = static_cast<std::uint8_t>(second_side_enabled);
        Host host(*this);
        // bsp.py show 0058BDF0, BE9A..BEDE/BFEF: the mission-start
        // temporary owns its vector/title before 0057D060 deep-copies it;
        // 004D2A80 releases it at the later finish_start hook. A caller
        // that republishes without that hook must retire its previous
        // temporary before replacing it, without draining either global.
        if (temporary_present_) {
            release(temporary_);
            temporary_present_ = false;
        }
        temporary_ = {};
        temporary_present_ = true;
        try {
            assign_native_string_header_vector_00506c80(&temporary_, &source, strings_);
            resize_native_string_header_0041dd40(&temporary_.name, strings_,
                source.name.length, true);
            if (source.name.length != 0)
                std::memcpy(temporary_.name.data, source.name.data, source.name.length);
            temporary_.second_side_enabled = source.second_side_enabled;
            frontend_seg19_21_cc13::loading_screen_set_mission_text_0057d060(&temporary_, host);
        } catch (...) {
            release(temporary_);
            temporary_present_ = false;
            throw;
        }
    }

    // MissionStartBinding's actual 0058BFEF hook consumes only the owning
    // temporary. Pending/current remain available to the loading screen.
    void finish_start();

    const GameLoadingConfigRecord& current() const noexcept { return current_; }
    const GameLoadingConfigRecord& pending() const noexcept { return pending_; }

private:
    // The virtual adapter borrows the retained owner only during publication.
    // Keeping it out of the owner's destruction path also leaves OFF builds
    // without a config-service vtable or indirect calls into the new bodies.
    class Host final : public frontend_seg19_21_cc13::LoadingScreenConfigHost {
    public:
        explicit Host(GameLoadingConfig& owner) noexcept : owner_(owner) {}
    private:
        void* global(std::uint32_t address) override {
            switch (address) {
            case 0x00e08780: return &owner_.current_;
            case 0x00e08790: return &owner_.current_.name;
            case 0x00e08798: return &owner_.current_.second_side_enabled;
            case 0x00e0879c: return &owner_.pending_;
            case 0x00e087ac: return &owner_.pending_.name;
            case 0x00e087b4: return &owner_.pending_.second_side_enabled;
            default: throw std::logic_error("unowned loading-config publication");
            }
        }
        void native_string_resize_0041dd40(frontend_seg19_21_cc13::NativeStr* s,
            std::uint32_t length, std::int32_t preserve) override {
            resize_native_string_header_0041dd40(s, owner_.strings_, length, preserve != 0);
        }
        void string_vector_assign_00506c80(void* destination, const void* source) override {
            assign_native_string_header_vector_00506c80(destination, source, owner_.strings_);
        }
        GameLoadingConfig& owner_;
    };
    class ReleaseHost final : public cc13_seg10::LoadingConfigReleaseHost {
    public:
        explicit ReleaseHost(NativeStringRawPoolContext& strings) noexcept : strings_(strings) {}
    private:
        void crt_free_00bf65ac(void* memory) override { singleton_lifetime_free(memory); }
        cc13_seg10::Object sized_pool_singleton_00419cc0(
            void*, std::uint32_t, std::int32_t) override {
            return reinterpret_cast<cc13_seg10::Object>(native_string_pool_get_or_create_00419cc0(
                strings_.actual_published_01090aa8, strings_.actual_manager_publication_01090aa0));
        }
        void sized_pool_return_block_00bd1510(cc13_seg10::Object pool, void* data,
            std::uint32_t bytes, std::int32_t) override {
            return_native_string_pool_00bd1510(reinterpret_cast<NativeStringPoolStorage*>(pool),
                data, bytes, strings_.actual_small_returns_disabled_01090aa4);
        }
        void native_string_range_destroy_00432050(cc13_seg10::Object first,
            cc13_seg10::Object last, cc13_seg10::Object, cc13_seg10::Object) override {
            destroy_native_string_header_range_00432050(first, last, strings_);
        }
        NativeStringRawPoolContext& strings_;
    };
    void release(GameLoadingConfigRecord& config) {
        // bsp.py show 004D2A80: title return precedes vector destruction;
        // the body itself clears +4/+8/+C. Call once per owned record.
        ReleaseHost host(strings_);
        cc13_seg10::game_clear_pending_mission_name_004d2a80(
            host, reinterpret_cast<cc13_seg10::Object>(&config));
    }

    NativeStringRawPoolContext& strings_;
    GameLoadingConfigRecord temporary_;
    bool temporary_present_{false};
    GameLoadingConfigRecord current_;
    GameLoadingConfigRecord pending_;
};

} // namespace bsp::game

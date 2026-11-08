// Focused container/mission-load regression, linked against the built bsp_core.
// Console probe only: no game entry point, window, renderer or original DLL.
#include "bsp/mission_load_path.hpp"
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
class Host final : public bsp::SetPendingSceneHost {
public:
    bsp::NativeSceneRecordList* records{};
    unsigned headers{}, awards{}, blocks{};
    std::vector<std::string> destroyed;
    void destroy_scene_record(bsp::SceneRecord& record) override {
        // 004BF930 calls the payload before unlinking its current node.
        require(records && !records->empty() && &records->front() == &record,
            "payload callback must observe the current list head");
        destroyed.push_back(record.scene_path);
    }
    void enter_file_block(const std::string&) override { ++blocks; }
    void leave_file_block() override { --blocks; }
    bool scene_file_exists(const std::string&) override { return true; }
    void load_scene_header_pass(const std::string& path, const std::string&,
        bsp::SceneRecord& record) override {
        require(record.scene_path == path, "path precedes header load");
        ++headers;
        record.mission_id = static_cast<std::int32_t>(headers);
    }
    void notify_award_tracker() override { ++awards; }
};
}

int main() {
    try {
        bsp::MissionLoadPathState state;
        auto& records = state.records;
        Host host;
        host.records = &records;
        require(records.empty() && !records.image().head && !records.image().tail,
            "constructor must start with the image's empty list");
        const auto first = bsp::run_set_pending_scene_004e2770(
            records, "first.scn", "", {}, host);
        auto* const retained = &records.front();
        require(first.record_created && first.records_destroyed == 0 && records.size() == 1,
            "first mission must append its actual record");
        require(!bsp::run_build_scene_record_004e1d70(records, "first.scn",
            bsp::SceneRecordBuildArm::ReuseExisting, "", host), "duplicate must not append");
        require(bsp::run_build_scene_record_004e1d70(records, "second.scn",
            bsp::SceneRecordBuildArm::Create, "", host), "second record append failed");
        const auto& image = records.image();
        require(image.count == 2 && image.head->record == retained &&
            !image.head->previous && image.head->next == image.tail &&
            image.tail->previous == image.head && !image.tail->next,
            "native append must retain payload identity and both links");
        require(records[1].scene_path == "second.scn" &&
            bsp::run_select_scene_record_004c6890(records, 2, {}).mission_id_byte == 2,
            "selection must read the linked list, including index==count tail behavior");
        const auto replacement = bsp::run_set_pending_scene_004e2770(
            records, "replacement.scn", "", {}, host);
        require(replacement.records_destroyed == 2 && records.size() == 1 &&
            records.front().scene_path == "replacement.scn" &&
            host.destroyed == std::vector<std::string>{"first.scn", "second.scn"},
            "replacement must destroy and unlink each previous payload in order");
        records.clear(&host);
        require(records.empty() && !image.head && !image.tail &&
            host.destroyed.size() == 3 && host.headers == 3 && host.awards == 3 && !host.blocks,
            "final clear must leave the canonical list empty without duplicate work");
        std::puts("SceneRecordList: append, retained identity, selection and replacement/clear passed");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 1;
    }
}

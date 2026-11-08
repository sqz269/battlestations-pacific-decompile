// Headless source fixture: original PE constants are mapped read-only; no
// original code or game loop runs. Exercise production command delivery and
// the ordinary motion tick, not a replacement task-install callback.
#include "bsp/game_hosts_units_internal.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_data_bootstrap.hpp"
#include "bsp/game_startup_random.hpp"
#include "bsp/game_native_dyn_process.hpp"
#include "bsp/legacy_crt_math.hpp"
#include <float.h>
#include <cerrno>
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace {
void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
}

int main(int argc, char** argv) {
    try {
        using namespace bsp::game;
        _control87(_PC_24, _MCW_PC);
        constexpr GameNativeDataSpan spans[] = {
            {GameNativeReadOnlyData::begin_address, GameNativeReadOnlyData::byte_count}
        };
        if (argc >= 2 && std::string(argv[1]) != "--child") {
            wchar_t executable[32768];
            require(GetModuleFileNameW(nullptr, executable, std::size(executable)) != 0,
                "missing probe executable path");
            GameNativeDataBootstrapChild child(executable,
                L"--child \"" + std::filesystem::path(argv[1]).wstring() + L"\"",
                spans, std::size(spans));
            child.reserve_and_resume();
            child.wait_for_mapping(30000);
            require(WaitForSingleObject(child.process_handle(), 30000) == WAIT_OBJECT_0,
                "probe child timed out");
            DWORD result;
            require(GetExitCodeProcess(child.process_handle(), &result) != 0,
                "missing probe result");
            return static_cast<int>(result);
        }
        require(argc >= 3, "expected original PE path or bootstrap child");
        auto reservation = accept_native_data_handoff(spans, std::size(spans));
        GameNativeReadOnlyData data(argv[2], spans, std::size(spans), std::move(reservation));
        GameHostLog log;
        require(log.open("local/c965-review1-host.log"), "cannot open host log");
        GameSingletonHost singletons(log);
        static const volatile std::uint32_t matherr_bypass = 0;
        static const bsp::LegacyCrtMathRuntime crt{&matherr_bypass, &_errno};
        bsp::bind_legacy_crt_math_runtime(crt);
        require(game_native_dyn_process(application_camera_axes_crt()).initialize_once_00cc8950() == 0,
            "source Dyn dispatch startup failed");
        bsp::RandomThreads random;
        random.register_current_00bd2fe0();
        publish_startup_random(&random);
        struct RandomBorrow {
            bsp::RandomThreads& owner;
            ~RandomBorrow() {
                publish_startup_random(nullptr);
                owner.unregister_current_00bd3050();
            }
        } random_borrow{random};
        GameVfsHost vfs(log, singletons, data, argv[2]);
        GameMissionLuaHost mission(log, vfs);
        GameUnitsHost units(log, mission);
        std::vector<GameSceneEntityRecord> entities(2);
        for (std::size_t i = 0; i < entities.size(); ++i) {
            auto& entity = entities[i];
            entity.created = true;
            entity.name = "survivor-fixture-" + std::to_string(i);
            entity.type_id = 1;
            entity.world[0] = entity.world[5] = entity.world[10] = entity.world[15] = 1.0f;
            entity.world[12] = i == 0 ? 420.0f : 0.0f;
            entity.world[13] = 75.0f;
            entity.world[14] = i == 0 ? -20.0f : 0.0f;
        }
        units.create_units(entities);
        std::vector<GameUnitSlot*> members;
        for (std::size_t i = 0; i < entities.size(); ++i) {
            // No installed class tables are mounted in this source fixture.
            // Seed health through the production maximum/repair writers.
            units.gunnery()->set_unit_max_health_036c(i, 100.0f);
            units.gunnery()->repair_unit_to_fraction(i, 1.0f);
            require(!units.gunnery()->unit_dead(i), "fixture plane is not alive");
            auto* slot = static_cast<GameUnitSlot*>(const_cast<void*>(units.unit_identity(i)));
            require(slot != nullptr, "fixture plane was not created");
            slot->class_id = 0x0f;
            slot->state->class_id = 0x0f;
            slot->motion_dispatch = kUnitMotionDispatches[8]; // known 007CE040 plane tick
            slot->state->active = true;
            slot->state->simulate = 0; // live +5Dh; nonzero gates the pilot think
            slot->row.active = true;
            slot->plane_control_mode_900 = 7;
            slot->retreat_task_installed = true;
            slot->retreat_state = bsp::RetreatState::kMoveTo;
            slot->attack_command_class = bsp::kRetreatCommandClass;
            slot->pilot_think_accumulator_70 = 1.0f;
            members.push_back(slot);
        }
        bsp::plane_squadron_registry().clear();
        auto& squadron = bsp::plane_squadron_registry().add("survivor-fixture");
        squadron.squadron_unit = 0;
        squadron.member_units = {0, 1};
        squadron.member_names = {entities[0].name, entities[1].name};
        squadron.member_spawn_index = {0, 1};
        // The fixture's leader is outside, while wing 1 is inside the actual
        // bounds supplied to the canonical 007F31A0 geometry computation.
        const bsp::WorldMapBounds bounds{{-100.0f, -10.0f, 100.0f},
            {300.0f, 10.0f, -100.0f}};
        require(bsp::point_outside_world_map_0071c4f0(bounds, {420.0f, 75.0f, -20.0f}) &&
            !bsp::point_outside_world_map_0071c4f0(bounds, {0.0f, 75.0f, 0.0f}),
            "leader/survivor fixture does not straddle the map edge");
        bool latched = false;
        std::array<float, 3> computed{};
        std::size_t posted = 0;
        bsp::SquadronLeaveMapHost leave;
        leave.effective_game_mode = 8;
        leave.session_mode = 0;
        leave.leave_latched = [&] { return latched; };
        leave.warn_exit_zone = [] {};
        leave.member_count = [&] { return static_cast<std::size_t>(squadron.live_count()); };
        leave.leader = [&] { return squadron.flight_leader(); };
        leave.member = [&](std::size_t i) { return squadron.live_member(i); };
        leave.outside_map = [&](std::size_t i) {
            const auto* p = members[i]->motion.position;
            return bsp::point_outside_world_map_0071c4f0(bounds, {p[0], p[1], p[2]});
        };
        leave.route_member_exit = [](std::size_t) { throw std::runtime_error("in-map survivor routed out"); };
        leave.leaving_on_purpose = [] { return true; };
        leave.latch_leave = [&] { latched = true; };
        leave.survivor_geometry = [&](bsp::SquadronLeaveMapGeometry& geometry) {
            const auto* p = members[0]->motion.position;
            geometry = {bounds.north_west, bounds.south_east, {p[0], p[1], p[2]}};
            return true;
        };
        leave.command_survivors = [&](const std::array<float, 3>& point) {
            computed = point;
            posted = queue_squadron_survivor_moveto(units.commands(), *units.gunnery(), members, 0, point);
        };
        leave.store_squadron_exit_party = [] { throw std::runtime_error("not the final-member arm"); };
        leave.kill_plane = [&] { units.gunnery()->kill_unit_00926d90(0, 4); };
        leave.kill_squadron = [] { throw std::runtime_error("surviving squadron killed"); };
        bsp::squadron_on_plane_left_map_007f31a0(0, leave);
        require(computed == std::array<float, 3>{452.0f, 75.0f, -22.0f},
            "canonical map-centre target is wrong");
        require(posted == 1, "did not post exactly one survivor order");
        require(units.gunnery()->unit_dead(0), "leader was not killed before queued delivery");
        require(members[1]->retreat_task_installed && !members[1]->moveto_task_installed &&
            members[1]->attack_command_class == bsp::kRetreatCommandClass,
            "survivor task changed before queued delivery");
        units.commands().begin_loopback_drain_0076c600();
        require(units.commands().finish_loopback_drain_0076c600() >= 1,
            "survivor command did not traverse the production loopback drain");
        bsp::SceneCommandTarget delivered;
        int mode = 0;
        require(units.commands().current_command_0071be40(1) == bsp::kPilotOrderClassMoveTo &&
            units.commands().active_command_descriptor_0071eb60(1, delivered, mode),
            "survivor director did not receive moveto after leader death");
        require(std::fabs(delivered.position[0] - computed[0]) < 0.002f &&
            std::fabs(delivered.position[1] - computed[1]) < 0.002f &&
            std::fabs(delivered.position[2] - computed[2]) < 0.002f,
            "queued codec did not retain the computed target");
        require(!members[1]->retreat_task_installed &&
            members[1]->retreat_state == bsp::RetreatState::kNone &&
            members[1]->attack_command_class == bsp::kPilotOrderClassMoveTo &&
            members[1]->moveto_point[0] == delivered.position[0] &&
            members[1]->moveto_point[2] == delivered.position[2],
            "delivered target did not retire retreat and reach the survivor's task input");
        for (int i = 0; i != 4 && !members[1]->moveto_task_installed; ++i)
            units.motion_step_00825f20(0.05f);
        require(members[1]->moveto_task_installed && !members[1]->retreat_task_installed &&
            members[1]->moveto_state != GameUnitSlot::MoveToTaskState::kNone &&
            members[1]->moveto_state != GameUnitSlot::MoveToTaskState::kFollow &&
            squadron.flight_leader() == 1 &&
            members[1]->moveto_point[0] == delivered.position[0] &&
            members[1]->moveto_point[2] == delivered.position[2],
            "ordinary bot think did not construct a moveto task for the surviving wing");
        bsp::plane_squadron_registry().clear();
        std::ofstream("local/c965-review1-result.txt")
            << "PASS: leader death -> queued survivor delivery -> decoded target -> retreat retired -> ordinary moveto task constructed\n";
        std::cout << "PASS: leader death -> queued survivor delivery -> decoded target -> retreat retired -> ordinary moveto task constructed\n";
        return 0;
    } catch (const std::exception& error) {
        std::ofstream("local/c965-review1-result.txt") << "FAIL: " << error.what() << '\n';
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}

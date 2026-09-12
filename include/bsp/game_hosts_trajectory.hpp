#pragma once
// bsp_game.exe milestone 2j: --trajectory-csv, one row per unit per fixed step.
//
// Addresses: none. This is executable plumbing, the same kind as --frames and
// --mission-frame-seconds: it writes what the reconstructed motion path already
// computed to a file, so an external comparison against a trace taken from the
// running game can be made. Nothing here is a reconstruction of native code and
// nothing here feeds back into the simulation.
//
// The column order and the spellings are the ones tools/motion_trace_compare.py
// reads. `t` is its only required column; `fwd_speed` is one of the three
// spellings it accepts for the speed channel (`fwd_speed`, `fwd_spd`, `speed`)
// and `forward_speed` is not, which would have zeroed that channel silently.
//
//   step            the fixed simulation step the motion virtual 00825f20 ran
//                   in. Step 0 is the pose before the first step, so a consumer
//                   that aligns on its first sample does not absorb one step's
//                   displacement and rotation into the alignment.
//   t               simulated seconds at the end of that step, 0 for step 0
//   unit            the instance name 0041dd40 set at creation
//   class           the `Type = E ShipClasses : <symbol>` token, which is also
//                   the symbol of the installed VehicleClass row index
//   x, y, z         the world position, pose row 3 (unit+FCh); the same three
//                   floats the keel-point arithmetic reads at 00826897,
//                   008268a1 and 008268af
//   heading         degrees of atan2(row2.x, row2.z)
//   fwd_speed       0092d730 over the body axis and the linear velocity
//   throttle        unit+980h, what the order ring published this step
//   rudder          unit+984h, the ordered rudder the ring published
//   yaw_rate        the angular velocity's component along the hull's own up
//                   axis, dot(w, row1), in rad/s. That is the component
//                   0092e8c0 slews toward the commanded rate; it equals the
//                   world y only while the hull is upright.
//
// The combined file holds every unit, which a reader that treats consecutive
// rows as one trajectory cannot use. One file per unit is therefore written
// alongside it as `<stem>.<unit>.csv`, with the same header and only that
// unit's rows.

#include <cstdio>
#include <map>
#include <string>
#include <vector>

namespace bsp::game {

struct GameUnitRow;
class GameHostLog;

class GameTrajectoryCsv {
public:
    GameTrajectoryCsv() = default;
    ~GameTrajectoryCsv();
    GameTrajectoryCsv(const GameTrajectoryCsv&) = delete;
    GameTrajectoryCsv& operator=(const GameTrajectoryCsv&) = delete;

    // Opens the combined file and writes its header row. A path that cannot be
    // opened is reported once and leaves the writer closed; the run is not
    // failed by it, because the trace is an output of the run rather than a step
    // of it. The per-unit files are opened on the first block of rows, when the
    // unit names are known.
    bool open(const std::string& path, GameHostLog& log);
    bool is_open() const noexcept { return file_ != nullptr; }

    // One block of rows, one per unit, for the fixed step that just ran. Call it
    // once with `step` 0 before the first step to record the initial pose.
    void append_step(unsigned long long step, float simulated_seconds,
        const std::vector<GameUnitRow>& units);

    void close();

    unsigned long long rows() const noexcept { return rows_; }
    std::size_t unit_files() const noexcept { return per_unit_.size(); }
    const std::string& path() const noexcept { return path_; }

private:
    std::FILE* open_unit_file(const std::string& unit_name);

    std::FILE* file_{nullptr};
    std::string path_;
    std::map<std::string, std::FILE*> per_unit_;
    unsigned long long rows_{0};
};

}  // namespace bsp::game

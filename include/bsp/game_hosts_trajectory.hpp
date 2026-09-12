#pragma once
// bsp_game.exe milestone 2j: --trajectory-csv, one row per unit per fixed step.
//
// Addresses: none. This is executable plumbing, the same kind as --frames and
// --mission-frame-seconds: it writes what the reconstructed motion path already
// computed to a file, so an external comparison against a trace taken from the
// running game can be made. Nothing here is a reconstruction of native code and
// nothing here feeds back into the simulation.
//
// The column order is the one tools/motion_trace_compare.py reads, header row
// included:
//
//   step            the fixed simulation step, 1-based, the step 00825f20 ran in
//   t               simulated seconds at the end of that step
//   unit            the instance name 0041dd40 set at creation
//   class           the `Type = E ShipClasses : <symbol>` token, which is also
//                   the installed VehicleClass row index's symbol
//   x, y, z         the world position, pose row 3 (unit+FCh)
//   heading         degrees of atan2(row2.x, row2.z)
//   forward_speed   0092d730 over the body axis and the linear velocity
//   throttle        unit+980h, what the order ring published this step
//   rudder          unit+984h, the ordered rudder the ring published
//   yaw_rate        the body angular velocity's y component, rad/s

#include <cstdio>
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

    // Opens the file and writes the header row. A path that cannot be opened is
    // reported once and leaves the writer closed; the run is not failed by it,
    // because the trace is an output of the run rather than a step of it.
    bool open(const std::string& path, GameHostLog& log);
    bool is_open() const noexcept { return file_ != nullptr; }

    // One block of rows, one per unit, for the fixed step that just ran.
    void append_step(unsigned long long step, float simulated_seconds,
        const std::vector<GameUnitRow>& units);

    void close();

    unsigned long long rows() const noexcept { return rows_; }
    const std::string& path() const noexcept { return path_; }

private:
    std::FILE* file_{nullptr};
    std::string path_;
    unsigned long long rows_{0};
};

}  // namespace bsp::game

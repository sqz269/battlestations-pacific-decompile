// bsp_game.exe milestone 2j: the --trajectory-csv writer. See the header for
// the column contract. Executable plumbing, no native address.

#include "bsp/game_hosts_trajectory.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_units.hpp"

namespace bsp::game {
namespace {

// A unit name comes from the scene and could in principle carry a comma or a
// quote. Quote every text cell and double an embedded quote, which is the whole
// of RFC 4180 that matters here.
void write_text(std::FILE* file, const std::string& text) {
    std::fputc('"', file);
    for (const char c : text) {
        if (c == '"') std::fputc('"', file);
        std::fputc(c, file);
    }
    std::fputc('"', file);
}

}  // namespace

GameTrajectoryCsv::~GameTrajectoryCsv() { close(); }

bool GameTrajectoryCsv::open(const std::string& path, GameHostLog& log) {
    close();
    if (path.empty()) return false;
    path_ = path;
#if defined(_MSC_VER)
    if (fopen_s(&file_, path.c_str(), "wb") != 0) file_ = nullptr;
#else
    file_ = std::fopen(path.c_str(), "wb");
#endif
    if (file_ == nullptr) {
        log.notef("trajectory csv %s could not be opened; no trace is written", path.c_str());
        return false;
    }
    std::fputs("step,t,unit,class,x,y,z,heading,forward_speed,throttle,rudder,yaw_rate\n",
        file_);
    log.notef("trajectory csv %s open: one row per unit per fixed step, columns "
        "step,t,unit,class,x,y,z,heading,forward_speed,throttle,rudder,yaw_rate",
        path.c_str());
    return true;
}

void GameTrajectoryCsv::append_step(unsigned long long step, float simulated_seconds,
    const std::vector<GameUnitRow>& units) {
    if (file_ == nullptr) return;
    for (const GameUnitRow& row : units) {
        std::fprintf(file_, "%llu,%.6f,", step, static_cast<double>(simulated_seconds));
        write_text(file_, row.name);
        std::fputc(',', file_);
        write_text(file_, row.type_symbol);
        std::fprintf(file_, ",%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n",
            static_cast<double>(row.position[0]), static_cast<double>(row.position[1]),
            static_cast<double>(row.position[2]), static_cast<double>(row.heading_degrees),
            static_cast<double>(row.forward_speed), static_cast<double>(row.throttle),
            static_cast<double>(row.ordered_rudder), static_cast<double>(row.yaw_rate));
        ++rows_;
    }
}

void GameTrajectoryCsv::close() {
    if (file_ != nullptr) {
        std::fclose(file_);
        file_ = nullptr;
    }
}

}  // namespace bsp::game

// bsp_game.exe milestone 2j: the --trajectory-csv writer. See the header for
// the column contract. Executable plumbing, no native address.

#include "bsp/game_hosts_trajectory.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_units.hpp"

namespace bsp::game {
namespace {

constexpr const char* kHeader =
    "step,t,unit,class,x,y,z,heading,fwd_speed,throttle,rudder,yaw_rate\n";

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

// `<stem>.<unit>.csv` next to the combined file. Anything a path cannot carry
// becomes an underscore; scene names are plain identifiers in practice, so this
// only guards against an authored name nobody has seen yet.
std::string per_unit_path(const std::string& combined, const std::string& unit) {
    std::string safe;
    safe.reserve(unit.size());
    for (const char c : unit) {
        const bool plain = (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z')
            || (c >= 'a' && c <= 'z') || c == '_' || c == '-';
        safe.push_back(plain ? c : '_');
    }
    if (safe.empty()) safe = "unit";
    const std::size_t dot = combined.find_last_of('.');
    const std::size_t slash = combined.find_last_of("\\/");
    const bool has_extension = dot != std::string::npos
        && (slash == std::string::npos || dot > slash);
    const std::string stem = has_extension ? combined.substr(0, dot) : combined;
    const std::string extension = has_extension ? combined.substr(dot) : std::string(".csv");
    return stem + "." + safe + extension;
}

std::FILE* open_for_write(const std::string& path) {
    std::FILE* file = nullptr;
#if defined(_MSC_VER)
    if (fopen_s(&file, path.c_str(), "wb") != 0) file = nullptr;
#else
    file = std::fopen(path.c_str(), "wb");
#endif
    return file;
}

}  // namespace

GameTrajectoryCsv::~GameTrajectoryCsv() { close(); }

bool GameTrajectoryCsv::open(const std::string& path, GameHostLog& log) {
    close();
    if (path.empty()) return false;
    path_ = path;
    file_ = open_for_write(path);
    if (file_ == nullptr) {
        log.notef("trajectory csv %s could not be opened; no trace is written", path.c_str());
        return false;
    }
    std::fputs(kHeader, file_);
    log.notef("trajectory csv %s open: one row per unit per fixed step, columns "
        "step,t,unit,class,x,y,z,heading,fwd_speed,throttle,rudder,yaw_rate, and one "
        "<stem>.<unit>.csv per unit beside it, because a reader that takes consecutive rows "
        "as one trajectory cannot use the combined file", path.c_str());
    return true;
}

std::FILE* GameTrajectoryCsv::open_unit_file(const std::string& unit_name) {
    auto found = per_unit_.find(unit_name);
    if (found != per_unit_.end()) return found->second;
    std::FILE* file = open_for_write(per_unit_path(path_, unit_name));
    if (file != nullptr) std::fputs(kHeader, file);
    per_unit_.emplace(unit_name, file);
    return file;
}

void GameTrajectoryCsv::append_step(unsigned long long step, float simulated_seconds,
    const std::vector<GameUnitRow>& units) {
    if (file_ == nullptr) return;
    for (const GameUnitRow& row : units) {
        std::FILE* const targets[2] = {file_, open_unit_file(row.name)};
        for (std::FILE* out : targets) {
            if (out == nullptr) continue;
            std::fprintf(out, "%llu,%.6f,", step, static_cast<double>(simulated_seconds));
            write_text(out, row.name);
            std::fputc(',', out);
            write_text(out, row.type_symbol);
            std::fprintf(out, ",%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n",
                static_cast<double>(row.position[0]), static_cast<double>(row.position[1]),
                static_cast<double>(row.position[2]),
                static_cast<double>(row.heading_degrees),
                static_cast<double>(row.forward_speed), static_cast<double>(row.throttle),
                static_cast<double>(row.ordered_rudder),
                static_cast<double>(row.yaw_rate_up_axis));
        }
        ++rows_;
    }
}

void GameTrajectoryCsv::close() {
    if (file_ != nullptr) {
        std::fclose(file_);
        file_ = nullptr;
    }
    for (auto& entry : per_unit_) {
        if (entry.second != nullptr) std::fclose(entry.second);
    }
    per_unit_.clear();
}

}  // namespace bsp::game

#pragma once
#include <cstddef>
#include <string>

namespace bsp::game {
class GameHostLog;
class GameNativeRendererApplication;
class GameVfsHost;
class GameScriptHost;

struct GameNativeFontProbeSummary {
    std::size_t fonts{};
    std::size_t glyph_records{};
    std::size_t unique_images{};
    std::size_t owners_before{};
    std::size_t owners_after{};
    bool resources_verified{};
    bool retired{};
};

// Opt-in diagnostic AFTER genuine renderer/device/VFS/script startup. Uses an
// isolated empty registry, the SAME real native font producer and original
// Fonts root/descriptor/language. No replacement publication, semantic texture
// wrapper, Text construction or font association export is performed.
//
// Returns only after actual image/atomic/glyph checks and successful explicit
// retirement. Failure logs the real retained frame state/exception and keeps
// its heap producer/registry alive through std::_Exit(1), the application's
// existing process-retention policy. It never returns invented success or
// unwinds a failed graph. Serialize with frames/reset and singleton drain.
// Resource execution proves this bounded producer path; raw ABI, language
// reload, active native Text/frontend rendering and gameplay remain unproven.
GameNativeFontProbeSummary probe_actual_native_fonts(GameHostLog&,
    GameNativeRendererApplication&, GameVfsHost&, GameScriptHost&,
    const std::string& language_font_path);
} // namespace bsp::game

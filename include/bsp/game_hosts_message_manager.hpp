#pragma once
// cc15-ob1: the MessageManager [game+21DCh] as bsp_game.exe holds it.
//
// Addresses: 00707480 construct (70h bytes, ConstructWorld 004DE610 allocates it at
// 004DFA1A and calls the constructor at 004DFA39), 007071F0 destruct (vtable slot 0
// 00707520 from DestroyWorld 004D2BB0 at 004D2D04), 007065E0 load Messages.lua,
// 00706160 load or unload one message script, 00706760 / 00706FC0 the per-mission
// "name#side" maps, 00705E00 the record resolver 00976B00 calls, 00702300 the
// lower_bound 0097B5E0 tests keys with.
//
// The typed bodies are bsp::engine_seg11_35_66_cc13 (src/engine_seg11_35_66_cc13.cpp);
// this host supplies their MessageManagerCalls. The record tree +4h is held as
// bsp::MessageRecordStore and the "name#side" string set +64h as a case-insensitive
// std::set; the 70h layout keeps the image's vtable word, tree heads, filters and
// flags. Evidence, seams and uncertainties: docs/CC15_OBJECT_MESSAGEMANAGER.md.
// Names are hypotheses, not recovered symbols.

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace bsp {
struct LanguageEntry;
struct VoiceClipRecord;
}

namespace bsp::game {

class GameHostLog;
class GameVfsHost;

// cc15-msg2: 00705E00's miss arm. A name absent from the record map gets a new record
// "<name>" timed by the narrative screen's TempCharWait / TempWaitBase, as the image
// builds it. OFF: the miss is counted and answers null.
inline constexpr bool kMessageRecordMissCc15Bound = true;
// cc15-msg2: 00704590's language is 008D57A0 on the settings block [00F88980] (the
// catalog row's voice_dir), not the lockit name the host was built with. OFF: the
// lockit name, as before.
inline constexpr bool kMessageVoiceLanguageCc15Bound = true;

// What 007065E0 reads besides its argument: the local party's side kind
// [[game+18CCh+4*[game+18ECh]]+28h] and the scene record block [game+5FCh]+1064h
// (enable byte; 00468BE9 sets it when the scene tree has "unittypes") with its
// id arrays +1068h "allied", +1074h "japanese", +1080h "neutral".
struct MessageManagerLoadInputs {
    std::int32_t local_side_kind{0};
    bool filters_enabled{false};
    std::vector<std::int32_t> ids_1068;
    std::vector<std::int32_t> ids_1074;
    std::vector<std::int32_t> ids_1080;
};

struct MessageManagerSummary {
    unsigned long long constructed{0};
    unsigned long long destructed{0};
    unsigned long long loads{0};               // 00706160 runs with load = 1
    unsigned long long unloads{0};             // 00706160 runs with load = 0
    unsigned long long scripts_opened{0};
    unsigned long long scripts_failed{0};
    std::string script_error;
    unsigned long long records_added{0};       // new record-map nodes from loads
    unsigned long long voice_probe_hits{0};    // [0109CEEC] vtable +8h true
    unsigned long long voice_probe_misses{0};
    unsigned long long sound_banks_not_loaded{0};       // 00A83FD0 seam
    unsigned long long streamed_probes_not_evaluated{0};// [0109CEEC] vtable +0Ch seam
    unsigned long long resolve_hits{0};        // 00705E00 found
    unsigned long long resolve_misses{0};      // 00705E00 miss
    unsigned long long records_created{0};     // misses given a "<name>" record (cc15-msg2)
    float char_wait_7c{0.0f};                  // the timing pair those records used
    float wait_base_80{0.0f};
    unsigned long long key_tests{0};           // 00702300 tests from 0097B5E0
    unsigned long long key_hits{0};
};

class GameMessageManagerHost {
public:
    // `voice_language` stands in for 008D57A0 on [00F88980] unless
    // kMessageVoiceLanguageCc15Bound and the settings are bound (see the doc).
    GameMessageManagerHost(GameHostLog& log, GameVfsHost& vfs, std::string voice_language);
    // The process-wide inputs 008D57A0 reads: the language table [00F88974] (count
    // [00F88978]) and the settings block's index [00F88980]+4h. Both are borrowed and
    // must outlive every manager; the application binds them once after settings load.
    static void bind_voice_language_settings(const std::vector<LanguageEntry>& catalog,
        const int& language_index_04);
    ~GameMessageManagerHost();
    GameMessageManagerHost(const GameMessageManagerHost&) = delete;
    GameMessageManagerHost& operator=(const GameMessageManagerHost&) = delete;

    // 00707480 into the 70h block (the image's operator new(70h) at 004DFA1A).
    // A second mission load destroys the previous manager first, as 004D2BB0 does.
    void construct_00707480();
    // 007071F0, then the block is freed (00707520's delete flag 1).
    void destruct_007071f0();
    bool constructed() const noexcept;

    // 007065E0(load): the filters, the side-kind flag and Scripts/datatables/Messages.lua.
    void load_messages_007065e0(bool load, const MessageManagerLoadInputs& inputs);
    // 00706760 / 00706FC0 on the manager (not called by this host's load walk).
    void load_message_map_00706760(const std::string& name, std::int32_t side);
    void unload_message_map_00706fc0(const std::string& name, std::int32_t side);

    // 00705E00 as 00976B00 calls it: the record for `name`. A miss creates a "<name>"
    // record in the image, whose duration reads the narrative screen
    // [[00E198C4]+A4h]+7Ch/+80h. With kMessageRecordMissCc15Bound the host does the
    // same; without it a miss is counted and answers null.
    const VoiceClipRecord* resolve_record_00705e00(const std::string& name);
    // 0097B5E0's test: 00702300 lower_bound plus the inlined find on the record map.
    bool record_map_contains_00702300(const std::string& key);

    std::size_t record_count() const noexcept;
    const MessageManagerSummary& summary() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace bsp::game

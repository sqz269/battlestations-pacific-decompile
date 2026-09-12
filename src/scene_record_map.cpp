#include "bsp/scene_record_map.hpp"

#include <cctype>
#include <cstring>

// The hidden-entity record map at SceneDatabase+18h. See
// docs/SCENE_RECORD_MAP.md for the evidence behind every rule below.

namespace bsp {
namespace {

// __stricmp (00BF7FBF) on two NUL-terminated byte strings, as the comparator
// reaches it from 00443D3D, 00468170 and 0046B386. The native routine folds
// case through the CRT's locale table; this projection folds ASCII only, which
// is what every authored scene name in the shipped files uses.
int stricmp_ascii(const char* a, const char* b) {
    for (;; ++a, ++b) {
        const unsigned char ca =
            static_cast<unsigned char>(std::tolower(static_cast<unsigned char>(*a)));
        const unsigned char cb =
            static_cast<unsigned char>(std::tolower(static_cast<unsigned char>(*b)));
        if (ca != cb) {
            return ca < cb ? -1 : 1;
        }
        if (ca == 0) {
            return 0;
        }
    }
}

SceneRecordKey key_of(const std::string& s) {
    SceneRecordKey k;
    k.length = static_cast<std::uint32_t>(s.size());
    k.data = s.empty() ? nullptr : s.c_str();
    return k;
}

}  // namespace

// 00443D00, and the same branches inlined in the two tree walks. The length
// tests come first, so a null data pointer is never dereferenced and a
// default-constructed key orders exactly like "".
bool scene_record_key_less(const SceneRecordKey& a, const SceneRecordKey& b) {
    if (a.length == 0) {
        return b.length != 0;  // 00443D0D..00443D1A
    }
    if (b.length == 0) {
        return false;  // 00443D24
    }
    return stricmp_ascii(a.data, b.data) < 0;  // 00443D3D
}

bool scene_record_key_less(const std::string& a, const std::string& b) {
    return scene_record_key_less(key_of(a), key_of(b));
}

bool scene_record_key_equivalent(const std::string& a, const std::string& b) {
    return !scene_record_key_less(a, b) && !scene_record_key_less(b, a);
}

// 00468CD0 plus the end check at 004691D9..00469214 and the mapped read at
// 00469233. The tree is not modelled: the ordering is total over the keys, so
// scanning for the equivalent key gives the same answer the descent does.
const SceneHiddenEntityRecord* scene_record_map_find(const SceneRecordMapView& map,
                                                     const std::string& name) {
    for (const SceneRecordEntry& entry : map.entries) {
        if (scene_record_key_equivalent(entry.key, name)) {
            return entry.record;  // node+14h
        }
    }
    return nullptr;  // node == _Myhead
}

// 00469480: the same find, reduced to 004694BE's SETNZ.
bool scene_record_map_contains(const SceneRecordMapView& map, const std::string& name) {
    return scene_record_map_find(map, name) != nullptr;
}

// The branch at 0046D5E4..0046D7E9 of 0046CF40, in the native order. Each gate
// jumps to the same 0046D7EE tail, so the first failure decides the outcome.
SceneHiddenRecordInsertResult scene_record_map_record_entity(SceneRecordMapHost& host,
                                                             const SceneHiddenRecordGates& gates,
                                                             const SceneHiddenRecordSource& source) {
    SceneHiddenRecordInsertResult result;

    if (!gates.hidden_property) {  // 0046D5F4
        result.outcome = SceneHiddenRecordOutcome::NotHidden;
        return result;
    }
    if (!gates.recording_enabled) {  // 0046D5FA, argument a6 at entry+54h
        result.outcome = SceneHiddenRecordOutcome::RecordingDisabled;
        return result;
    }
    if (!gates.should_generate || !host.should_generate(source)) {  // 0046D655, 0046D65A
        result.outcome = SceneHiddenRecordOutcome::GenerationRefused;
        return result;
    }
    if (gates.registration_pass) {  // 0046D679, the argument at entry+58h
        result.outcome = SceneHiddenRecordOutcome::RegistrationPass;
        return result;
    }

    // 0046D68C. A null result skips the construction at 0046D6A4 and leaves the
    // record pointer zero; the reader still runs its temporaries' cleanup.
    if (host.allocate_record(kSceneHiddenRecordSize) == nullptr) {
        result.outcome = SceneHiddenRecordOutcome::AllocationFailed;
        return result;
    }

    result.record = host.construct_record(source);  // 0046D6F4

    // 0046D71B builds the key from the same buffer construct_record took as its
    // third argument, so the key is the record's name by construction.
    if (!host.insert_record(source.name, result.record)) {  // 0046D75F
        result.outcome = SceneHiddenRecordOutcome::Duplicate;
        result.leaked = true;  // the caller never inspects the insert's bool
        return result;
    }

    result.outcome = SceneHiddenRecordOutcome::Inserted;
    return result;
}

// 0046DD87: the compare is FCOMIP of the constant against the heading, and the
// rotation branch is taken only when the constant is strictly greater.
bool scene_record_heading_applies(float heading) {
    return static_cast<double>(heading) < kSceneHiddenRecordHeadingSentinel;
}

// 0046DD54..0046DD7E. The 16 floats come across by REP MOVSD and only the
// translation row is overwritten; the basis survives unless 00467050 replaces
// it, which this packet does not port.
void scene_record_apply_placement(const float record_frame[16],
                                  const ScenePlacementOverride& placement,
                                  float out_frame[16]) {
    std::memcpy(out_frame, record_frame, sizeof(float) * 16);
    out_frame[12] = placement.position[0];  // +30h, 0046DD62
    out_frame[13] = placement.position[1];  // +34h, 0046DD70
    out_frame[14] = placement.position[2];  // +38h, 0046DD7E
}

}  // namespace bsp

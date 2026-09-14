// Scene deferred references (0046AAB0). See docs/SCENE_DEFERRED_REFS.md.
#include "bsp/scene_deferred_refs.hpp"

#include <cctype>

namespace bsp {
namespace {

// 00438E10 BSP_CString_CompareInsensitive, used at 0046AB03. Only the zero/
// non-zero answer matters here, so this compares for equality and nothing else.
// The shipped scenes need it: `follow` and `cruise` are authored in lower case.
bool equal_insensitive(const std::string& a, const std::string& b) noexcept {
    if (a.size() != b.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        const unsigned char ca = static_cast<unsigned char>(a[i]);
        const unsigned char cb = static_cast<unsigned char>(b[i]);
        if (std::tolower(ca) != std::tolower(cb)) {
            return false;
        }
    }
    return true;
}

}  // namespace

SceneCommandRecord scene_command_record_004690d0(void* owner, const char* command,
                                                 const char* target) {
    // 004690D0 measures each argument with an inline strlen and copies it into a
    // pooled string; a null pointer yields the zero-length string, which is what
    // 0046AAB0 later sees as the empty-string global 00E18560.
    SceneCommandRecord record;
    record.owner = owner;
    record.command = (command != nullptr) ? std::string(command) : std::string();
    record.target = (target != nullptr) ? std::string(target) : std::string();
    return record;
}

const SceneCommandType* scene_command_registry_find_00467170(const SceneCommandRegistry& registry,
                                                             const std::string& name) noexcept {
    // 0046AAE7..0046AB11: walk the list from the head, compare the entry's
    // vtable[4] name against the record's, and stop at the first match. The
    // entry is returned even when its object is null; 0046AB1D tests that after
    // the match, not before.
    for (const SceneCommandType& entry : registry) {
        if (equal_insensitive(entry.name, name)) {
            return &entry;
        }
    }
    return nullptr;
}

SceneCommandResolution resolve_scene_command_0046aab0(const SceneCommandRecord& record,
                                                      const SceneCommandRegistry& registry,
                                                      SceneDeferredReferenceHost& host) {
    SceneCommandResolution resolution;

    const SceneCommandType* command = scene_command_registry_find_00467170(registry, record.command);
    if (command == nullptr) {
        resolution.outcome = SceneCommandOutcome::kUnknownCommandName;
        return resolution;
    }
    resolution.command = command;
    if (command->identity == nullptr) {
        // 0046AB18/0046AB1D: ESI is reloaded from node+8h and a null ends the record.
        resolution.outcome = SceneCommandOutcome::kNullCommandObject;
        return resolution;
    }

    // 0046AB2A tests the target name's LENGTH, not a separate kind field.
    if (record.target.empty()) {
        if (host.command_requires_target(command->identity)) {
            resolution.outcome = SceneCommandOutcome::kCommandRequiresTarget;
            return resolution;
        }
        if (!host.owner_pose_is_current(record.owner)) {
            host.refresh_owner_pose(record.owner);
        }
        resolution.target.kind = 0;
        resolution.target.position_valid = 1;
        resolution.target.object_id = 0;
        resolution.target.object = nullptr;
        host.owner_world_position(record.owner, resolution.target.position);
    } else {
        void* entity = host.find_entity_by_name(record.target);
        if (entity == nullptr) {
            resolution.outcome = SceneCommandOutcome::kTargetNotFound;
            return resolution;
        }
        resolution.target.kind = 1;
        resolution.target.position_valid = 0;
        resolution.target.object = entity;
        resolution.target.object_id = host.entity_object_id(entity);
        // 00F87574..00F8757C is a read-only zero vector: no writer anywhere in
        // the image, and the bytes in the file are zero.
        resolution.target.position[0] = 0.0f;
        resolution.target.position[1] = 0.0f;
        resolution.target.position[2] = 0.0f;
    }

    resolution.target.trailing = 0.0f;  // XORPS XMM0 / MOVSS at 0046ABF8
    resolution.outcome = SceneCommandOutcome::kIssued;
    return resolution;
}

SceneDeferredResolveStats resolve_scene_deferred_references_0046aab0(
    const SceneCommandQueue& queue, const SceneCommandRegistry& registry,
    SceneDeferredReferenceHost& host) {
    SceneDeferredResolveStats stats;

    for (const SceneCommandRecord& record : queue) {
        ++stats.records;
        const SceneCommandResolution resolution =
            resolve_scene_command_0046aab0(record, registry, host);
        switch (resolution.outcome) {
            case SceneCommandOutcome::kIssued:
                ++stats.issued;
                // 0046AC0B: ECX is the record's owner, the pushed object is the
                // command, and the literal 1 is the last argument.
                host.issue_command(record.owner, resolution.command->identity, resolution.target, 1);
                break;
            case SceneCommandOutcome::kUnknownCommandName:
                ++stats.unknown_command;
                break;
            case SceneCommandOutcome::kNullCommandObject:
                ++stats.null_command_object;
                break;
            case SceneCommandOutcome::kCommandRequiresTarget:
                ++stats.requires_target;
                break;
            case SceneCommandOutcome::kTargetNotFound:
                ++stats.target_not_found;
                break;
        }
    }

    // 0046AC2A: ECX = this+14Ch, then the tail JMP to 0046A9F0. It runs on every
    // path, including the empty-queue path taken at 0046AAC6.
    host.clear_queue();
    stats.queue_cleared = true;
    return stats;
}

}  // namespace bsp

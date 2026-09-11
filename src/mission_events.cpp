#include "bsp/mission_events.hpp"

#include <cctype>

namespace bsp {
namespace {

// The 23 channel-dispatch routines in segment 65 that call 00980150, paired with
// the literal each one looks up. Three of them look up more than one name and
// appear once per name.
constexpr WarningChannelDispatch kChannelDispatches[] = {
    {"recon", 0x00980E50U},
    {"kill", 0x009813A0U},
    {"globals.warn_uslost", 0x009813A0U},
    {"exitzone", 0x00982120U},
    {"input", 0x00982540U},
    {"surrender", 0x00982990U},
    {"failure", 0x00982C50U},
    {"failureshipyard", 0x00982C50U},
    {"failureairfield", 0x00982C50U},
    {"leak", 0x009832F0U},
    {"fire", 0x00983780U},
    {"repair", 0x00983C10U},
    {"zone", 0x00983F50U},
    {"command", 0x00984300U},
    {"target", 0x00984800U},
    {"ammoType", 0x00984BA0U},
    {"stock", 0x00984EB0U},
    {"gui", 0x00985250U},
    {"generate", 0x00985590U},
    {"player", 0x00985920U},
    {"musicOver", 0x00985C50U},
    {"chat", 0x00985F30U},
    {"entityKilled", 0x00986480U},
    {"shipLanded", 0x00986820U},
    {"damage", 0x00986B00U},
    {"hpEvent", 0x00986B00U},
    {"hit", 0x00988510U},
};

constexpr std::size_t kChannelDispatchCount =
    sizeof(kChannelDispatches) / sizeof(kChannelDispatches[0]);

bool equal_case_insensitive(const char* left, const char* right) noexcept
{
    if (left == nullptr || right == nullptr) {
        return false;
    }
    while (*left != '\0' && *right != '\0') {
        const int a = std::tolower(static_cast<unsigned char>(*left));
        const int b = std::tolower(static_cast<unsigned char>(*right));
        if (a != b) {
            return false;
        }
        ++left;
        ++right;
    }
    return *left == *right;
}

} // namespace

const WarningChannelDispatch* warning_channel_dispatches() noexcept
{
    return kChannelDispatches;
}

std::size_t warning_channel_dispatch_count() noexcept
{
    return kChannelDispatchCount;
}

const WarningChannelDispatch* find_warning_channel(const char* name) noexcept
{
    for (std::size_t index = 0; index < kChannelDispatchCount; ++index) {
        if (equal_case_insensitive(kChannelDispatches[index].name, name)) {
            return &kChannelDispatches[index];
        }
    }
    return nullptr;
}

bool warning_is_expired(float now, float applied_at, float lifetime) noexcept
{
    // 00987677: FCOMIP applied_at against now - lifetime, then JBE into the
    // retire path. Not-greater-than, so an exact hit retires.
    return !(applied_at > now - lifetime);
}

bool warning_outranks(float candidate_priority, float held_priority) noexcept
{
    // 009876c9: FCOMIP then JBE back to the loop head, so the candidate has to
    // be strictly greater and a tie keeps the entry already held.
    return candidate_priority > held_priority;
}

bool advance_warning_periodic(float& accumulator, float scaled_delta) noexcept
{
    accumulator = accumulator + scaled_delta;
    if (accumulator > kMissionEventTickPeriod) {
        // 009875d1 stores zero rather than subtracting the period.
        accumulator = 0.0f;
        return true;
    }
    return false;
}

WarningDeadlineResult update_warning_deadlines(WarningManagerState& state, float now) noexcept
{
    WarningDeadlineResult result{};
    if (state.air_raid_active && state.air_raid_deadline < now) {
        state.air_raid_active = false;
        result.air_raid_expired = true;
    }
    if (state.collision_active && state.collision_deadline < now) {
        state.collision_active = false;
        result.collision_expired = true;
    }
    return result;
}

bool voice_can_play_005b71d0(const VoiceReadinessInputs& inputs) noexcept
{
    if (inputs.panel_sequence_active) {
        return false;
    }
    if (inputs.voice_disabled) {
        return false;
    }
    // The slot set does not change inside the native loop, so the per-clip
    // iteration reduces to one test as long as there is at least one clip.
    if (inputs.clip_count != 0 && inputs.busy_slots >= kVoicePlaybackSlotCount) {
        return false;
    }
    return !inputs.voice_blocked;
}

float voice_line_attenuation_005bbdc0(float distance) noexcept
{
    return (kVoiceAudibleRange - distance) / kVoiceAudibleRange;
}

bool voice_line_audible(float attenuation) noexcept
{
    // 005bbdc0 requires both clauses: at least the floor and above zero.
    return attenuation >= kVoiceMinimumAttenuation && attenuation > 0.0f;
}

bool warning_within_proximity(float dx, float dy, float dz) noexcept
{
    return dx * dx + dy * dy + dz * dz < kWarningProximityRadiusSquared;
}

bool warning_report_clock_allows(float now) noexcept
{
    return now > kWarningReportGraceSeconds;
}

namespace {

// One pass of 009763e0 over a queue. Returns true when the pass finished the
// call, which the native body does by returning outright.
bool report_walk(
    std::vector<WarningRecord>& queue, WarningManagerHost& host, const WarningRecord& incoming,
    float now, bool applied_queue, WarningReportResult& result)
{
    for (std::size_t index = 0; index < queue.size(); ++index) {
        WarningRecord& entry = queue[index];
        const float lifetime = applied_queue ? entry.applied_lifetime : entry.pending_lifetime;
        if (warning_is_expired(now, entry.applied_at, lifetime)) {
            host.destroy_warning(index, !applied_queue);
            queue.erase(queue.begin() + static_cast<std::ptrdiff_t>(index));
            result.outcome = WarningReportOutcome::RetiredExpired;
            result.touched = index;
            result.touched_applied_queue = applied_queue;
            return true;
        }
        // vtable[5](incoming). The native test is a virtual on the queued entry,
        // so the projection compares the message ids the subclasses carry.
        if (entry.message_id == incoming.message_id && entry.kind == incoming.kind) {
            entry.applied_at = now;
            result.outcome = WarningReportOutcome::SuppressedDuplicate;
            result.touched = index;
            result.touched_applied_queue = applied_queue;
            return true;
        }
    }
    return false;
}

} // namespace

WarningReportResult report_warning_009763e0(
    WarningManagerState& state, WarningManagerHost& host, const WarningRecord& incoming,
    float now)
{
    WarningReportResult result{};
    if (incoming.message_id.empty()) {
        host.report_error(kWarningReportErrorText);
        result.outcome = WarningReportOutcome::RejectedEmptyId;
        return result;
    }

    if (report_walk(state.pending, host, incoming, now, false, result)) {
        return result;
    }
    if (report_walk(state.applied, host, incoming, now, true, result)) {
        return result;
    }

    if (!host.message_id_known(incoming.message_id)) {
        host.report_error(kWarningUnknownIdFormat);
        result.outcome = WarningReportOutcome::RejectedUnknownId;
        return result;
    }

    const std::size_t index = state.pending.size();
    state.pending.push_back(incoming);
    state.pending.back().state = 1;
    host.on_warning_accepted(index);

    if (state.pending[index].positional_speaker != 0 && host.voice_ready(index)) {
        host.apply_warning_00974070(index);
        state.pending[index].state = 2;
        state.pending[index].applied_at = now;
        result.outcome = WarningReportOutcome::QueuedAndApplied;
    } else {
        result.outcome = WarningReportOutcome::Queued;
    }
    result.touched = index;
    return result;
}

bool poll_warning_prompt_0096d540(WarningManagerState& state, WarningManagerHost& host)
{
    if (state.prompt_callback.empty()) {
        return false;
    }
    if (!host.input_action_pressed(kWarningPromptInputAction)) {
        return false;
    }
    host.raise_prompt_hud(kWarningPromptHudSlot);
    host.call_mission_lua(state.prompt_callback);
    state.prompt_callback.clear();
    return true;
}

std::vector<std::string> evaluate_warning_channel_0097b8c0(
    const std::vector<WarningSubscription>& subscriptions)
{
    std::vector<std::string> callbacks;
    for (const WarningSubscription& subscription : subscriptions) {
        if (subscription.condition_holds) {
            callbacks.push_back(subscription.callback);
        }
    }
    return callbacks;
}

bool subscription_input_fires(
    const WarningSubscription& subscription, const std::vector<int>& pressed_actions) noexcept
{
    for (const int action : subscription.input_actions) {
        for (const int pressed : pressed_actions) {
            if (action == pressed) {
                return true;
            }
        }
    }
    return false;
}

WarningManagerTickResult run_warning_manager_update(
    WarningManagerState& state, WarningManagerHost& host, float scaled_delta, float now)
{
    WarningManagerTickResult result{};

    host.pump_input_channel_00982540();

    result.periodic_fired = advance_warning_periodic(state.periodic_accumulator, scaled_delta);
    if (result.periodic_fired) {
        host.scan_proximity_00977990();
    }

    result.prompt_dispatched = poll_warning_prompt_0096d540(state, host);

    result.deadlines = update_warning_deadlines(state, now);
    if (result.deadlines.collision_expired) {
        host.stop_collision_sound();
    }

    return result;
}

} // namespace bsp

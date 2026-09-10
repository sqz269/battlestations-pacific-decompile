#include "bsp/profile_unlock.hpp"

#include <algorithm>
#include <cctype>

namespace bsp {
namespace {

int case_insensitive_compare(std::string_view lhs, std::string_view rhs) noexcept
{
    const std::size_t shared = std::min(lhs.size(), rhs.size());
    for (std::size_t i = 0; i < shared; ++i) {
        const auto a = static_cast<unsigned char>(lhs[i]);
        const auto b = static_cast<unsigned char>(rhs[i]);
        const int la = std::tolower(a);
        const int lb = std::tolower(b);
        if (la != lb) {
            return la < lb ? -1 : 1;
        }
    }
    if (lhs.size() == rhs.size()) {
        return 0;
    }
    return lhs.size() < rhs.size() ? -1 : 1;
}

bool is_separator(char c) noexcept
{
    return kUnlockTokenSeparators.find(c) != std::string_view::npos;
}

} // namespace

bool NativeStringCaseInsensitiveLess::operator()(
    std::string_view lhs, std::string_view rhs) const noexcept
{
    return case_insensitive_compare(lhs, rhs) < 0;
}

std::vector<std::string> parse_unlock_expression_007fc4c0(std::string_view expression)
{
    // `strtok` collapses runs of separators and never emits an empty token, so
    // "IJN04,,JM8_GOLD" and "IJN04 JM8_GOLD" parse the same way.
    std::vector<std::string> tokens;
    std::size_t i = 0;
    while (i < expression.size()) {
        while (i < expression.size() && is_separator(expression[i])) {
            ++i;
        }
        const std::size_t start = i;
        while (i < expression.size() && !is_separator(expression[i])) {
            ++i;
        }
        if (i > start) {
            tokens.emplace_back(expression.substr(start, i - start));
        }
    }
    return tokens;
}

UnlockSource classify_unlock_token_007fc4c0(
    const ProfileUnlockState& profile, std::string_view token)
{
    // 007FC5A1: ECX = [this+64h], the mission-completion map. 0090C560 reports
    // false when the key is absent and otherwise `node->value != 0`.
    const auto completed = profile.mission_completion.find(token);
    if (completed != profile.mission_completion.end() && completed->second != 0) {
        return UnlockSource::kMissionCompleted;
    }

    // 007FC602 and 007FC65B: two 004C7F10 finds, membership only. The native
    // compares the returned iterator against the container's own end node, so
    // a hit is exactly "the key is in the set".
    if (profile.unlocks.find(token) != profile.unlocks.end()) {
        return UnlockSource::kUnlocks;
    }
    if (profile.pending_unlocks.find(token) != profile.pending_unlocks.end()) {
        return UnlockSource::kPendingUnlocks;
    }

    // 007FC6C2: `TEST EAX,EAX; JG` on 005097D0's result, so strictly greater
    // than zero. An absent key returns 0 without inserting.
    const auto counter = profile.named_counters.find(token);
    if (counter != profile.named_counters.end() && counter->second > 0) {
        return UnlockSource::kNamedCounter;
    }

    // 007FC6DD: 007F8890 walks the list at this+D0h and compares length first,
    // then __stricmp.
    const auto content = std::find_if(profile.content_ids.begin(), profile.content_ids.end(),
        [token](const std::string& id) { return case_insensitive_compare(id, token) == 0; });
    if (content != profile.content_ids.end()) {
        return UnlockSource::kDownloadableContent;
    }

    return UnlockSource::kNone;
}

bool is_unlock_expression_satisfied_007fc4c0(
    const ProfileUnlockState& profile, std::string_view expression)
{
    // The native tests the accumulated result at the head of the next
    // iteration (007FC554) and returns it, so the first satisfied token ends
    // the walk. A parse that produces no tokens leaves the result at zero.
    for (const std::string& token : parse_unlock_expression_007fc4c0(expression)) {
        if (classify_unlock_token_007fc4c0(profile, token) != UnlockSource::kNone) {
            return true;
        }
    }
    return false;
}

bool are_unlock_requirements_met_007fc820(
    const ProfileUnlockState& profile, const std::vector<std::string>& requirements)
{
    // 007FC844: first == last returns AL = 1 before any element is read.
    // 007FC85C: the first false element returns AL = 0.
    for (const std::string& requirement : requirements) {
        if (!is_unlock_expression_satisfied_007fc4c0(profile, requirement)) {
            return false;
        }
    }
    return true;
}

} // namespace bsp

#include "bsp/profile_unlock.hpp"
#include "bsp/oct18_tail_b.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstring>
#include <stdexcept>

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

class ProfileContentCalls final : public oct18_tail_b::ProfileContentHost {
public:
    int compare_case_insensitive_00bf7fbf(const char* lhs, const char* rhs) override {
        return _stricmp(lhs, rhs);
    }
    void invalid_iterator_00bf6713() override {
        throw std::logic_error("invalid profile content-list iterator");
    }
};

bool owns_profile_content(const ProfileUnlockState& profile, std::string_view token)
{
    // 007F8890 (bsp.py show): profile+D0h points to a circular list sentinel;
    // nodes hold next/previous/length/data at +0/+4/+8/+Ch. This read-only view
    // borrows the actual owner's IDs, preserving their order and byte lengths.
    // It does not populate content entitlements from installed files: 007FDB20
    // clears this owner's list, and only its real producer can add IDs.
    struct Node {
        Node* next;
        Node* previous;
        std::uint32_t length;
        const char* data;
    };
    struct ProfileView {
        std::array<std::byte, 0xd0> unread;
        Node* head;
    };
    struct StringView {
        std::uint32_t length;
        const char* data;
    };
    static_assert(sizeof(Node) == 0x10 && offsetof(Node, data) == 0xc);
    static_assert(offsetof(ProfileView, head) == 0xd0);
    static_assert(offsetof(StringView, data) == 4);

    std::vector<Node> nodes(profile.content_ids.size());
    Node sentinel{&sentinel, &sentinel, 0, nullptr};
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        const std::string& id = profile.content_ids[i];
        nodes[i] = {i + 1 < nodes.size() ? &nodes[i + 1] : &sentinel,
                    i != 0 ? &nodes[i - 1] : &sentinel,
                    static_cast<std::uint32_t>(id.size()), id.c_str()};
    }
    if (!nodes.empty()) {
        sentinel.next = &nodes.front();
        sentinel.previous = &nodes.back();
    }
    const ProfileView view{{}, &sentinel};
    // Tokens are string_views into a delimited expression; the CRT needs its
    // own terminator, while the native length comparison must retain size().
    const std::string terminated(token);
    const StringView comparison{static_cast<std::uint32_t>(token.size()), terminated.c_str()};
    ProfileContentCalls calls;
    return oct18_tail_b::profile_owns_content_007f8890(&view, &comparison, calls);
}

template<class T> T content_field(const void* storage, std::size_t offset) {
    T result;
    std::memcpy(&result, static_cast<const std::byte*>(storage) + offset, sizeof result);
    return result;
}

} // namespace

namespace oct18_tail_b {
// 007F8890, __thiscall(profile,NativeString* id), RET 4, AL bool.
// Moved intact from leftovers_cc13.cpp into the core profile target. Evidence:
// bsp.py show 007f8890; profile+D0h sentinel, node length+8/data+Ch, __stricmp.
// Length equality precedes the CRT comparison; empty IDs read no data pointer.
bool profile_owns_content_007f8890(const void* profile, const void* id, ProfileContentHost& host) {
    for (void* node = content_field<void*>(content_field<void*>(profile, 0xd0), 0);
         node != content_field<void*>(profile, 0xd0); node = content_field<void*>(node, 0)) {
        const auto length = content_field<std::uint32_t>(id, 0);
        if (content_field<std::uint32_t>(node, 8) == length &&
            (length == 0 || host.compare_case_insensitive_00bf7fbf(
                content_field<const char*>(node, 0xc), content_field<const char*>(id, 4)) == 0)) return true;
        if (node == content_field<void*>(profile, 0xd0)) {
            host.invalid_iterator_00bf6713();
            throw std::logic_error("non-returning CRT provider returned");
        }
    }
    return false;
}
} // namespace oct18_tail_b

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
    if (owns_profile_content(profile, token)) {
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

#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

namespace bsp::game {

// Query the host's EXISTING pass-K mark/pending stores. A mark alone is a
// queued notification; after target_gone_drain_009273a0 removes it from the
// pending list, the observer slot04 has delivered. No notice is consumed here.
inline bool projectile_owner_notice_delivered(std::size_t unit,
    const std::vector<bool>& notified, const std::vector<std::size_t>& pending) noexcept {
    if (unit >= notified.size() || !notified[unit]) return false;
    for (const std::size_t queued : pending)
        if (queued == unit) return false;
    return true;
}

// Semantic view of the attacker observer bound by 006E6BB0/006E1B90 at
// shot-base+98h. Its nullable target at member+14h is the owner+ACh read by
// 004704E0. The projectile and its deferred hit records share this ONE view;
// source/accounting indices are separate and never reconstruct its target.
class GameProjectileObservedOwner final {
public:
    GameProjectileObservedOwner(std::size_t unit, const void* identity,
        std::optional<bool> destroyed_at_bind, bool notice_delivered_at_bind) noexcept
        : unit_(unit), identity_(identity), destroyed_at_bind_(destroyed_at_bind),
          notice_delivered_at_bind_(notice_delivered_at_bind) {}

    std::optional<std::size_t> unit() const noexcept { return unit_; }
    const void* identity() const noexcept { return identity_; }

    // 006E17D0: only the matching notifier clears the member's +14h. Once
    // cleared, retained/deferred copies cannot recreate an owner from source.
    void target_gone(const void* notifier) noexcept {
        if (identity_ != nullptr && notifier == identity_) {
            unit_.reset();
            identity_ = nullptr;
        }
    }

    // The canonical removal path stores unit+5Eh before delivering slot04
    // (009263C0 / 009274CE..00927510). Sample it again before hit dispatch,
    // including host removal sites that publish flags directly. Health zero,
    // Destroy/+5Dh and pending +60h do NOT imply a slot04 notification.
    void refresh(const void* current_identity, std::optional<bool> destroyed,
        bool notice_delivered) noexcept {
        if (!unit_) return;
        if (current_identity != identity_
            || (destroyed_at_bind_ == false && destroyed == true)
            || (!notice_delivered_at_bind_ && notice_delivered))
            target_gone(identity_);
    }

private:
    std::optional<std::size_t> unit_;
    const void* identity_;
    std::optional<bool> destroyed_at_bind_;
    bool notice_delivered_at_bind_;
};

using GameProjectileOwnerRef = std::shared_ptr<GameProjectileObservedOwner>;

} // namespace bsp::game

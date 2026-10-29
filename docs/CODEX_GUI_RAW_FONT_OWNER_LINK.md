# Raw Font owner metadata link

**API-only checkpoint; implementation not yet supplied or built.** Base is
`f7f6dd79d0658a8c40b0a60b0f58547857086ed6`, reviewed plan9953. This packet
owns only raw-default Text owner HPP/CPP and this doc; producer worker owns
the combined Font Host/App half. No source activation, native-body credit,
host call site, CMake/metadata/Ghidra edit, shader invocation, build or game.

## Frozen typed interface

All methods are on `bsp::game::GameNativeRawDefaultTextOwner`; producer identity
is `const GameNativeFontApplication&`. Exact agreed signatures are:

- `begin_raw_font_link(producer, GameNativeRendererApplication&, GameVfsHost&)`.
- `complete_raw_font_link(producer, NativeFontResourceOwners&, const void* image)`.
- `fail_raw_font_link(producer) noexcept`.
- `require_font_retirement(producer, const NativeFontResourceOwners&, const void* image) const`.
- `detach_raw_font_link_after_parent_drain(producer, const NativeFontResourceOwners&, const void* image)`.

The stable fresh owner exists before the producer begins export. Combined Host/
App calls begin BEFORE irreversible export, complete with the SAME actual ready
collection/native registry image and existing renderer/VFS domain, then records
the exact borrower. No lookup, count operation, native getter/string/shader/pool
effect occurs in the handshake. Failure at any admitted stage latches all three
participants, including a rejection before owner.begin; owner fail may remember
that producer if none was recorded but never replace a different producer.
No cancellation, replay, legacy empty factory or parallel borrower is admitted.

## Required body contract

Before raw122, the owner prepares a real d13 ShaderContext over that SAME Font
collection and actual renderer platform-publication cell, with one nonmoving
immutably prepared ShaderFrame. The existing real effect/cache/program/VFS/
typed/raw three-cell strings/Lua/sampler graph remains the provider domain;
known raw B318B0 admission does not acquire a fake unknown-renderer provider.
The frame/headers/context and nested cache backing remain retained/reserved.
No Font108 assignment, AB8C30/AB8CE0 call, Font credit, or108/1EC admission is added.

Before native Font retirement, the SAME producer/collection/saved image must
match, and the owner must be terminal RETIRED with no failed/active invocation,
all Text/Model/material/header/shader custody settled, actual scalar/late-flags/
pool/disposal proof and provider quiescence. Healthy bound state or old DEFAULT
settlement alone is insufficient. Extra credits, flags0 and failures retain the
whole graph and the producer; no dead Font or Text payload is queried.

After parent renderer verification and owner.after_native_renderer_drain returns,
Font Host/App explicitly detach this SAME saved link before owner.reset. Native
Font images may already be destroyed: collection/producer C++ metadata and VFS/
descriptors/services remain alive, and saved image is compared as identity only.
No retired name/header/tree/texture dereference or expired callback is permitted.
Existing DEFAULT/prefix/geometry behavior stays unchanged for an unlinked owner.

Raw AB8C30 and owned cached1EC EFFECT credit/retirement are separate blockers.
This interface exports no raw Text/Font pointer or new shader execution path.
Root owns combined source review/build, later preregistered gates and integration.

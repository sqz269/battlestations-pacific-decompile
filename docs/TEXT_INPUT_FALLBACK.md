# Text-owner fallback editing

Read-only audit on 2026-09-09, verifying project `bsp` and program
`/battlestationspacific.exe` before live batches. This supplements
TEXT_INPUT_CONSUMER. No C++, shared metadata or Ghidra annotations changed.

## Owner and call boundary

Fallback00a96750 takes the text owner in ECX and two stack arguments whose low
bytes are event value and category; it returns with RET8. Category zero denotes
a character; every nonzero category takes the special-key branch. It is called
after owner virtual0 declines an already-popped event.

| Owner field | Observed use |
| --- | --- |
| +0 | Virtual callback table |
| +4 | Enabled flag, checked by consumer rather than fallback |
| +8 / +C | Byte-remapping source length / pointer |
| +10 / +14 | Replacement string pair; +14 is directly indexed by remapper |
| +18 / +1C | Editable text length / byte pointer |
| +20 | Cursor index, with branch-specific signed/unsigned comparisons |
| +24 | Context passed in ECX to character acceptance helper00ab6d30 |

These are field roles, not recovered class names or a fully validated layout.
The routines use byte strings, not UTF-16 character indices. No text selection
range is used by the audited fallback.

## Complete simple key branches

For special-key events the following behavior requires no substring helpers:

| Event byte | Action |
| --- | --- |
| 2D Insert | Set global00e12f34 to logical NOT of its old nonzero state; call owner virtual+4 |
| 25 Left | If signed cursor>0, decrement cursor then call virtual+4 |
| 27 Right | If unsigned cursor<unsigned length, increment then call virtual+4 |
| 24 Home | If signed cursor>0, set cursor0 then call virtual+4 |
| 23 End | If unsigned cursor<unsigned length, set cursor=length then call virtual+4 |
| 26 Up | Call virtual+10 |
| 28 Down | Call virtual+14 only when global byte00f8bc01 is zero |
| 09 Tab | Call virtual+18 |
| 21 Page Up | Call virtual+20 |
| 22 Page Down | Call virtual+1C |

These callbacks receive owner in ECX and no additional stack parameters in the
observed calls. Their semantics beyond the triggering event are unestablished;
do not silently supply no-op handlers in a native text-owner integration.
At cursor bounds no notification is issued. Other special-key values fall through
without action, except Delete described below. Caps Lock14h is enqueued by the
window policy but has no fallback action here.

For category0, CR0Dh invokes virtual+8 and Escape1Bh invokes virtual+Ch. They
bypass character acceptance and remapping. Backspace08h uses the edit branch.
These dispatch/navigation branches are the smallest complete fallback behavior
that can be ported with explicit required callbacks, cursor, length and shared
insert-mode state; full fallback must retain an explicit boundary for edits.

## Text mutations and dependencies

The observed substring/concatenation sequence implies these transformations for
ordinary valid cursor indices; exact invalid-range behavior still depends on the
native string helpers:

* Delete2Eh, category nonzero: only when unsigned cursor<length, concatenate
  text[0:cursor] and text[cursor+1:], assign, then virtual+4. Cursor unchanged.
* Backspace08h, category0: only when signed cursor>0, concatenate
  text[0:cursor-1] and text[cursor:], assign, decrement cursor, then virtual+4.
* Accepted ordinary character: form prefix text[0:cursor], append mapped byte,
  append suffix beginning cursor+(global00e12f34==0), assign, increment cursor,
  then virtual+4. Thus nonzero mode preserves the old byte at cursor (insertion);
  zero skips it (overwrite).

Strings are assembled through00469840 (substring),00531030 (one-byte string),
004261a0 (concatenation),00425f40 (assignment), plus temporary destruction and
native allocation helpers. These dependencies are not reconstructed here.
Substring arguments use7FFFFFFFh for the remaining suffix. Do not claim a full
fallback port by replacing invalid cursor behavior with arbitrary clamping.

Character acceptance happens **before** remapping.00a96750 loads owner+24 into
ECX and calls00ab6d30(event). That routine sign-extends the low event byte to EAX,
then compares against positive A2h,A3h,A5h,A7h,AAh,B2h. Those comparisons cannot
match a sign-extended byte; do not turn the apparent blacklist into an unsigned
byte filter. It tail-dispatches00ab6d00, which accepts space20h immediately;
other bytes are sign-extended into AX and passed to00ad4500 using the context's
pointer at+108h. The result in AL decides acceptance. Glyph/context behavior
inside00ad4500 remains unresolved, so a generic isprint or ASCII whitelist would
change the native dependency.

Remapper00a96450 takes owner in ECX, one byte argument on stack, returns byte in
AL, RET4. It searches the source string at owner+C up to unsigned length+8,
using fallback pointer00f8bc02 if source is null. The first equal byte selects
replacement[matching index] from owner+14. No match returns the original byte.
It does not bounds-check the replacement string. The new interface may validate
its paired maps but must document that difference; this is not Unicode folding.

## Clipboard request flag boundary

The window's WM_CHAR16h branch sets platform+44h and also enqueues the ordinary
character event. Two tiny platform helpers were identified:

* 00bec160: ECX platform, `MOV AL,[ECX+44h]; RET` (read flag without clearing).
* 00bec170: ECX platform, `MOV BYTE PTR[ECX+44h],0; RET` (clear flag).

Neither address has a saved Ghidra xref in the verified project. They are not
among the established concrete platform vtable slots. A bounded search of
nearby platform code and direct singleton references did not establish a paste
consumer. This is an analysis limit, not proof that paste is absent or unused.
Do not clear the flag as part of queue pop or fabricate clipboard insertion.

## Disk/live byte evidence

All complete function ranges below match the installed executable and saved
program byte-for-byte; SHA256 values cover exactly the inclusive ranges shown.

| Range | SHA256 |
| --- | --- |
| 00a96750..00a96c87 | 4d4b4c460bb38dc644674fb208addf0a220724be61865ce20184c44bb288e4bd |
| 00a96450..00a9648d | 42fb8013fbd3ca219ad86fed6d3c2c4641d84e722c13dcfbed60a8e9124d546b |
| 00ab6d30..00ab6d6e | 0a93c9db9f5b51cc4d8bae428e274f1243f0c6a65da8b666f7558758cb8d7626 |
| 00ab6d00..00ab6d28 | f75e438aec1c7bc3ce0ec000973f9d0069b7160144bd1ee2ba5df9d84405b4e0 |
| 00bec160..00bec163 | 4e291090f46b0840c5c023d9e4012dbcad1f16e2de0de219861d538e932bb0d4 |
| 00bec170..00bec174 | cf98c2ffb79a62c2a27af0d09b039a229ff8fecef9f6002167f0d22f8bc36559 |

No build or runtime test was required for this audit. A queue/dispatch probe
does not validate native text mutations, glyph acceptance, clipboard insertion
or the surrounding UI owner lifetime.

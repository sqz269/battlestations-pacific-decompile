# Raw string field corrections

Reverse-find 0265ee8ad replaces the raw data/length DWORD overlays with fenced
byte copies, preserving data-first/null-skip/length ordering. Assignment and
concat 276857287 make the same correction only in their raw-context overloads,
including the two concat initialization stores. Shared helpers, typed overloads,
other bodies and the existing concat cleanup behavior remain unchanged.

Primary and independent source review accept both exact commits. Strict x86
object checks pass, as does the combined Release Win32 build at bfd79c206
(existing checks 2/2, 14.10 seconds). Inverse projections preserve the unrelated
source text. These checks do not establish Original ABI, fault or runtime parity.

The raw override body is separate and is not included in this build. Existing
lower-provider fencing/exception qualifications remain. No loaded locale,
raw16 growth, parser, startup or raw Text activation follows. All nine raw Text
choices remain false; the frozen EMPTY source-admission hold is unchanged.

The later equality correction is f8dd5924: only four field-read calls change to
the existing raw byte helpers. Left/right length capture, unequal/zero early
returns, right/left data reads and current CRT comparison stay unchanged. Exact
primary/independent review, strict Win32 object and canonical combined build
pass (2/2 existing checks, 11.83 seconds). This closes a language-call prerequisite,
with language/reload/parser and runtime admission still separate.

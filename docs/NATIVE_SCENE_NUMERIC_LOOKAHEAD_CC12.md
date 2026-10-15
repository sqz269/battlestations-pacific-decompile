# Ordinary numeric token lookahead

`008D8F10` and `008D8F40` now have complete ordinary Source wrappers over a
genuine, already-live 838h tokenizer and its existing retained
`NativeSceneTokenizerContext`. The old `std::string`/`SceneLexer` projections
remain separate. Names are descriptive hypotheses, not recovered symbols.

Each Original body is 35 bytes and 13 instructions: respectively
`[008D8F10,008D8F33)` and `[008D8F40,008D8F63)`. Both have just two direct calls,
raw peek `008D8A70` and the scanner at `00BF7533`, with no branch, consume,
allocation, destructor or x87 instruction in these wrappers. ECX is the live
tokenizer, there are no stack arguments, and the Original returns full EAX zero
or one with `RET`. The new C++ APIs return `bool`; their calling convention and
upper EAX bits do not reproduce the Original ABI.

`PUSH ECX` creates a four-byte local containing the receiver bits. Its address
and the format are pushed before peek. The returned current token pointer is
then passed to scanf. The destination is an `int` for `%d`, a `float` for `%f`;
its value is ignored. The wrappers initialize it with a bit copy before peek,
avoiding evaluation of those bits as a float. The Original
`SUB EAX,1 / NEG / SBB / ADD 1` sequence tests assignment count exactly equal
to one. No whole-token exhaustion or trailing-text policy is introduced.

Original disk/live bytes agree for both whole bodies and the immutable format
roles `00CE3A34 = 25 64 00` and `00CE4334 = 25 66 00`. Fixed Source literals
copy those verified `%d`/`%f` bytes. They supply neither Original numeric
address identity nor new writable/default format cells. The packet creates no
owner, scanner, global or startup binding and accepts no new callback or
caller-selected format.

`std::sscanf` is the explicit linked host CRT boundary for `00BF7533`, as in
the accepted token-value service. The ordinary domain uses stable valid ASCII
NUL-terminated tokens, current C locale, decimal prefixes within signed-int or
finite-binary32 range, and existing raw-tokenizer bounds, lifetime and provider
conditions. A nonconverting token returns false. Prefix success remains success;
there is no full-token validation. Peek may populate the cache and scan source
bytes according to its accepted body; these wrappers never consume the cached
token. No overflow, alias, invalidation, concurrent mutation, reentry, fault,
provider throw or exceptional unwind is admitted. Private VS2005 scanf behavior,
arbitrary exceptional floating values, FP/error-state parity, Original
register/class/FS/EH/private-CRT ABI and game behavior remain unproven.

The first strict compile passed with MSVC Win32 19.51.36244, C++17, `/MD /O2
/Ob2 /Oy- /W4 /WX /EHsc /Gy /GL- /fp:strict`. All 13 COFF sections, 39 symbol
indices including auxiliaries, five code sections (170 bytes, 65 decoded
instructions), 11 code relocations and one metadata relocation were reviewed.
The wrappers are each 44 bytes/17 instructions. Both store the owner bits before
the raw peek call, relocate the exact format bytes, call `_sscanf`, and compare
the count with one. No x87 or consume binding occurs. The other three code
sections are emitted CRT helpers; their option storage, imported
`__stdio_common_vsscanf`, weak AVX datum, format COMDATs, metadata and literal
`MD_DynamicRelease` directives are retained in the complete evidence.

The required ordinary worktree build is recorded separately in the report.
This new TU is intentionally unregistered, so that build checks the current
registered project while the strict compile checks this Source. No new test,
probe, runtime, `008D9B40` float reader, whole typed parser, loader or application
publication is claimed. Integration and shared metadata belong to Root.

Evidence is under ignored `local/WholeSceneNumericLookaheadSource01`:
`precompile_inputs.DATA.json`, complete COFF JSON/listing/dumpbin,
`original_tool_receipts.DATA.json`, declared compiler input pins, and the final
family receipt. Compiler/build receipts retain the first launch and natural
terminal result without replay.

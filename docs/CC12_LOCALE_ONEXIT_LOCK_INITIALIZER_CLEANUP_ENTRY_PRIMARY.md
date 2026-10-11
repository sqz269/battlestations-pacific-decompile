# Root locale numeric target entry review

Root accepts only `6A 0A` at `00C11C18`: PUSH 10 under the saved default
32-bit decode. Root checked both direct entry metadata responses, all four
listing aliases, complete saved nine-byte ownership and ordinary fallthrough.
Root independently decoded opcode 6A and signed immediate 0A, then ran actual
detailed Capstone 5.0.7. On normal completion PUSH writes 0000000A through
current SS:[ESP-4] and leaves flags unchanged. The literal and saved name
establish no unlock, cleanup, caller-argument or lock-number meaning.

The remaining seven body bytes are unread. Only four prototype metadata prefix
lines were used; the CLI's retained additional suffix remains opaque. The
positive caller's E8 numerically selects this entry, with no extra callee ABI.

Root verified 78 core manifest members, six actual frozen Git objects and eight
finite positive inputs. Final delivery has 67 manifest members, 12 object
records/ten distinct actual objects, eight unchanged inputs and exactly two
working/index/committed outputs. Original/copy ZIPs agree with distinct file IDs.
The fully reviewed pure helper passed Root core and delivery audits, each with
one ZIP read, one helper execution, one receipt write and no denials. Root
producer/gate bytes and helper preimages remain pinned. Worker commit:
`9e4cd800dd28e6922bcab51871a01f215fa69041`.

Private evidence: `local/cc12_locale_onexit_lock_initializer_cleanup_entry_primary/`.
Core SHA-256: `0accc63b0233556802e9bf33cd4a9c4947176e74a90f3f5337643627d7ee1ba4`.
Delivery SHA-256: `ce66df2e2e80a20dd9a49ff828acfd934579677521032d760118b22d76da889e`.
The public report pins Root gate, physical decode/Git review and strict replay.
No complete-body, Original ABI, Source, fresh build, fixture, startup or gameplay
credit is added. Frozen Source IDs establish worker revision identity only.

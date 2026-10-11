# Application CRT handler body: qualified operands and local stack transfers

The exact **54-byte body `00BF6B43..00BF6B78`** is now captured and decoded. Two byte reads agree, and independent manual decoding, Capstone 5.0.7 and all **27 context-zero saved instruction texts** agree. This proves the bounded instruction operands and their conditional local effects; the original compiler, FH3 authenticity, real invocation and external target behavior remain unproved.

Root first reviewed the complete per-byte listing gate and then explicitly admitted these exact bytes and instruction starts. Four typed batches cover every byte, for **216 responses**, and all four raw batches match at modification **98**. Six actual count streams report **64,730** functions. All 27 InstructionDB units are wholly owned by BF6B43 and partition its saved range without gaps or overlaps. Every override is NONE. The saved call flow at BF6B64 and terminator at BF6B78 agree with the physical E8 and C3 opcodes.

The body is:

```text
BF6B43 PUSH EBP               BF6B44 MOV EBP,ESP
BF6B46 SUB ESP,8              BF6B49 PUSH EBX
BF6B4A PUSH ESI               BF6B4B PUSH EDI
BF6B4C CLD                    BF6B4D MOV [EBP-4],EAX
BF6B50 XOR EAX,EAX            BF6B52 PUSH EAX
BF6B53 PUSH EAX               BF6B54 PUSH EAX
BF6B55 PUSH DWORD [EBP-4]      BF6B58 PUSH DWORD [EBP+14h]
BF6B5B PUSH DWORD [EBP+10h]    BF6B5E PUSH DWORD [EBP+0Ch]
BF6B61 PUSH DWORD [EBP+8]      BF6B64 CALL 00C07991
BF6B69 ADD ESP,20h            BF6B6C MOV [EBP-8],EAX
BF6B6F POP EDI                BF6B70 POP ESI
BF6B71 POP EBX                BF6B72 MOV EAX,[EBP-8]
BF6B75 MOV ESP,EBP            BF6B77 POP EBP
BF6B78 RET
```

Let **S** be entry ESP; **B/D/X/I/J** be entry EBP/EAX/EBX/ESI/EDI; and **W1..W4** be the incoming DWORDs at `[S+4]`, `[S+8]`, `[S+0Ch]`, `[S+10h]`. These are unknown symbolic words, not assigned exception-parameter meanings. Under sequential nonfaulting execution, the wrapper creates **H=EBP=S-4**, saves D at `[S-8]`, and saves X/I/J at `[S-10h]`, `[S-14h]`, `[S-18h]`. It reserves `[S-0Ch]` without initializing that result slot before the call. It clears DF and EAX, pushes three zeros, pushes saved D, then pushes W4, W3, W2, W1. This body never dereferences D as a pointer.

Immediately before the call, **ESP=S-38h**, EBP=H and EAX=0. The physical E8 displacement is `+10E28h` from BF6B69 to **00C07991**. The call pushes BF6B69 and gives the unexamined target **T=ESP=S-3Ch**, with this word layout:

| Target stack offset | Word |
| --- | --- |
| T+00h | BF6B69, encoded return address |
| T+04h / +08h / +0Ch / +10h | W1 / W2 / W3 / W4 |
| T+14h | D, the opaque incoming EAX word |
| T+18h / +1Ch / +20h | 0 / 0 / 0 |

No argument names or external calling convention follow from that layout. Each typed batch automatically returned one target-function object; every such object remains wholly opaque. C07991 is used only as an encoded address from this call instruction and its site-flow metadata. No target, caller, prototype, descriptor, IAT, stack-content or data query occurred.

If external control reaches BF6B69, call its returned ESP/EBP/EAX **Q/F/A**, with no assumed relation to S/H/D. The local code adds 20h to ESP and stores A at `[F-8]`. Let M1 mean memory after that write. The pops read EDI/ESI/EBX from `M1[Q+20h]`, `M1[Q+24h]`, `M1[Q+28h]`; EAX is reloaded from `M1[F-8]`; ESP becomes F; EBP is loaded from `M1[F]`; and RET takes EIP from `M1[F+4]`, leaving ESP=F+8. Using M1 preserves possible aliasing caused by unknown Q and F.

Only under the additional assumptions **Q=S-38h**, **F=S-4**, and intact saved B/X/I/J plus incoming `[S]`, does the epilogue restore those registers, carry returned A in EAX, return through the incoming `[S]` word and leave ESP=S+4. The target may not return or may violate those assumptions; its behavior was not inspected.

The earlier unwind actions still conditionally require **EBP=storeESP+20h**. This wrapper's own **EBP=S-4** does not identify the original store-time stack, an action invocation, or an action's EBP. Forwarding four unknown words and D supplies no original EH-frame or cleanup authority. The saved FID_conflict library name and earlier undefined(void) prototype remain metadata rather than ABI proof.

The narrow package pins the Root admission, all physical helpers and current query tools, raw metadata/count/schedule/text/byte captures, retained Capstone output and actual HEAD/src/include Git identities. Strict one-ZIP replay passes on the original and a distinct physical copy. Inside the read-only audit guard, replay redoes the manual decode and checks the captured Capstone result; it does not load Capstone or any dynamic library. Post-preload imports, other file opens, network/process activity and filesystem mutations are forbidden. Actual staged/final receipts verify unchanged prerequisites and exactly these two outputs. No Source-content census or inherited archive replay is performed.

No GPR mutation, C++, test, build, fixture, ABI or game-validation credit is added. The existing filename records the packet's metadata origin; its final scope is this explicitly admitted body-operand proof. See `reports/cc12_application_crt_handler_body_listing_metadata.json`.

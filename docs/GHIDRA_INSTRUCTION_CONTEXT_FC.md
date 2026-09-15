# Read-only Ghidra instruction-context query (FC)

Addresses: 00c0504c, 00c05052, 00c0504d, 00bf6aa6

## Result and boundary

`python tools/bsp.py ghidra instruction-context <addresses...> --context N
[--output local/<file>.json] [--lines N]` now queries Ghidra instructions at exact
requested starts without requiring a containing function. The command uses the existing
verified `Client` before its typed POST, defaults to zero neighboring instructions, caps
interactive output, and preserves the complete raw HTTP response bytes when `--output` is
used.

This reports only the current listing instruction, optional neighboring listing
instructions, the bridge mnemonic, and its simple detected-pattern labels. It does not
report or infer a function body, complete control flow, no-return state, thunk state, or
an instruction-length snapshot. No Ghidra mutation endpoint was called: there was no
create, disassemble, save, rename, comment, prototype, or flow-override operation.

## Identity and path pins

- Worktree: `J:/PROG/battlestations-pacific-decompile-orch2-ghidra-instruction-context-fc`
- Branch: `agent/orch2-ghidra-instruction-context-fc`
- Frozen base before this packet commit: `700564653f19b80db9d30f81897d483352e5ea21`
- Payload: `J:/PROG/battlestations-pacific-decompile-orch2-ghidra-instruction-context-fc/tools/bsp.py`
- Evidence document: `J:/PROG/battlestations-pacific-decompile-orch2-ghidra-instruction-context-fc/docs/GHIDRA_INSTRUCTION_CONTEXT_FC.md`
- Existing client: `J:/PROG/battlestations-pacific-decompile-orch2-ghidra-instruction-context-fc/tools/ghidra_export.py`
- Target config: `J:/PROG/battlestations-pacific-decompile-orch2-ghidra-instruction-context-fc/config/target.json`
- Bridge source: `J:/tools/ghidra-mcp/src/main/java/com/xebyte/core/XrefCallGraphService.java`
- Configured Ghidra identity: project `bsp`, program path
  `/battlestationspacific.exe`, language `x86:LE:32:default`, image base
  `00400000`, project file `C:/Users/sqz269/bsp.gpr`, loopback bridge
  `http://127.0.0.1:8089`.

Every live packet query constructed `Client(config)` and ran `Client.verify()` first.
The pre-edit identity check was:

```text
command: python tools/bsp.py ghidra count
exit: 0
stdout:
live function count 64088; snapshot 64088; same
stderr: empty
```

## Endpoint source semantics

The inspected bridge source at `XrefCallGraphService.java:1188` registers
`/get_assembly_context` as POST. It accepts body fields `xref_sources`,
`context_instructions` (bridge default 5), and `include_patterns`, plus `program` in the
query string. For each supplied string it parses the address and calls
`Listing.getInstructionAt(addr)`. A present instruction is returned with its text,
mnemonic, lists gathered by `getInstructionBefore` and `getInstructionAfter`, and simple
pattern labels. A non-start returns exactly `error: "No instruction at address"` for that
input. The implementation accepts `include_patterns` but does not consult it.

The CLI sends normalized 32-bit hexadecimal address strings, a decimal context count
bounded to 0 through 64, and an empty `include_patterns` list. Its default context is 0,
so the default response contains only exact requested starts. It uses three attempts with
the same 90-second timeout and retry classes as `Client.get`, and applies the same decoded
error conventions. The full raw response can only be written to a `.json` path under this
worktree's ignored `local/` directory.

## Commands and complete output

The unavailable pre-edit CLI method was retained before the fix:

```text
command: python tools/bsp.py ghidra instruction-context C0504C C05052 C0504D BF6AA6 --context 0 --lines 80
exit: nonzero (execution harness reported 1)
stdout: empty
stderr:
bsp.py ghidra: argument ghidra_command: invalid choice: 'instruction-context' (choose from count, proto, flow, flow-properties, comments, documentation, xrefs, callers, callees, bytes, decompile, disasm, export, ensure, autostart)
accepted here: --help
one-screen reference: python tools/bsp.py cheatsheet
```

The implemented exact-start batch returned:

```text
command: python tools/bsp.py ghidra instruction-context C0504C C05052 C0504D BF6AA6 --context 0 --lines 80
exit: 0
stdout:
{
 "00c0504c": {
  "address": "00c0504c",
  "instruction": "CALL dword ptr [0x00ce20b8]",
  "context_before": [],
  "context_after": [],
  "mnemonic": "CALL",
  "patterns_detected": [
   "control_flow"
  ]
 },
 "00c05052": {
  "address": "00c05052",
  "instruction": "RET 0x4",
  "context_before": [],
  "context_after": [],
  "mnemonic": "RET",
  "patterns_detected": []
 },
 "00c0504d": {
  "address": "00c0504d",
  "error": "No instruction at address"
 },
 "00bf6aa6": {
  "address": "00bf6aa6",
  "instruction": "POP EAX",
  "context_before": [],
  "context_after": [],
  "mnemonic": "POP",
  "patterns_detected": [
   "stack_operation"
  ]
 }
}
stderr: empty
```

One neighbor in each direction demonstrates the bridge's listing context without making a
function-ownership claim:

```text
command: python tools/bsp.py ghidra instruction-context C0504C --context 1 --lines 40
exit: 0
stdout:
{
 "00c0504c": {
  "address": "00c0504c",
  "instruction": "CALL dword ptr [0x00ce20b8]",
  "context_before": [
   "00c0504b: RET"
  ],
  "context_after": [
   "00c05052: RET 0x4"
  ],
  "mnemonic": "CALL",
  "patterns_detected": [
   "control_flow"
  ]
 }
}
stderr: empty
```

The optional-output invocation and the exact 534-byte response were:

```text
command: python tools/bsp.py ghidra instruction-context C0504C C05052 C0504D BF6AA6 --context 0 --output local/ghidra_instruction_context_fc_response.json --lines 80
exit: 0
stdout:
Saved the full instruction-context response for 4 addresses to local/ghidra_instruction_context_fc_response.json
stderr: empty

exact response bytes interpreted as UTF-8:
{"00c0504c":{"address":"00c0504c","instruction":"CALL dword ptr [0x00ce20b8]","context_before":[],"context_after":[],"mnemonic":"CALL","patterns_detected":["control_flow"]},"00c05052":{"address":"00c05052","instruction":"RET 0x4","context_before":[],"context_after":[],"mnemonic":"RET","patterns_detected":[]},"00c0504d":{"address":"00c0504d","error":"No instruction at address"},"00bf6aa6":{"address":"00bf6aa6","instruction":"POP EAX","context_before":[],"context_after":[],"mnemonic":"POP","patterns_detected":["stack_operation"]}}

response SHA256 9598f12c84e4f1f1c936561c1668fe946020cd1da21149ffe364956ae7e301ee
response SHA512 60474392cb93802f978769dfac41942d47173572eb2284b1291f11fb76f3be8b4b7b6ce8e0a8be2f3f8fd8459dc6d60a64530c86995ac5f920f5ee34f2bbe69a
```

Input bounds fail before a client is constructed, so they cannot issue a request:

```text
command: python tools/bsp.py ghidra instruction-context C0504G --context 0
exit: nonzero (execution harness reported 1)
stdout: empty
stderr:
bsp.py ghidra instruction-context: argument addresses: address must be a 32-bit hexadecimal value
accepted here: --context --help --limit --lines --output
one-screen reference: python tools/bsp.py cheatsheet

command: python tools/bsp.py ghidra instruction-context C0504C --context 65
exit: nonzero (execution harness reported 1)
stdout: empty
stderr:
bsp.py ghidra instruction-context: argument --context: context must be an integer from 0 through 64
accepted here: --context --help --limit --lines --output
one-screen reference: python tools/bsp.py cheatsheet
```

Syntax validation used a cache outside tracked source:

```text
command: $env:PYTHONPYCACHEPREFIX=(Join-Path (Get-Location) 'local\pycache-fc'); python -m py_compile tools\bsp.py
exit: 0
stdout: empty
stderr: empty
```

No build, test suite, native executable, game, helper executable, Ghidra analysis, or Ghidra
write operation was run for this tooling-only packet.

## Exact `tools/bsp.py` source edit diff

```diff
diff --git a/tools/bsp.py b/tools/bsp.py
index f76cbd18..af85447c 100644
--- a/tools/bsp.py
+++ b/tools/bsp.py
@@ -12 +12 @@ from the snapshot, sharded ledgers, tags, call graph, partition, PE strings and
-  python tools/bsp.py ghidra count|proto|flow|xrefs|callers|callees|bytes|comments|decompile|disasm|export ...
+  python tools/bsp.py ghidra count|proto|flow|instruction-context|xrefs|callers|callees|bytes|comments|decompile|disasm|export ...
@@ -701,0 +702,53 @@ def first_int(text):
+def instruction_address(text):
+    value = str(text).strip()
+    if not re.fullmatch(r'(?:0[xX])?[0-9a-fA-F]{1,8}', value):
+        raise argparse.ArgumentTypeError('address must be a 32-bit hexadecimal value')
+    return f'{int(value, 16):08x}'
+
+
+def instruction_context_count(text):
+    try:
+        value = int(text, 10)
+    except ValueError as exc:
+        raise argparse.ArgumentTypeError('context must be an integer from 0 through 64') from exc
+    if not 0 <= value <= 64:
+        raise argparse.ArgumentTypeError('context must be an integer from 0 through 64')
+    return value
+
+
+def ghidra_instruction_context(c, addresses, context):
+    """Read exact listing starts through the bridge's typed POST endpoint."""
+    import time
+    from urllib.parse import urlencode
+    from urllib.request import Request, urlopen
+
+    endpoint = 'get_assembly_context'
+    body = {
+        'xref_sources': addresses,
+        'context_instructions': context,
+        'include_patterns': [],
+    }
+    url = c.config['ghidra_url'].rstrip('/') + '/' + endpoint + '?' + urlencode({'program': c.config['program']})
+    request = Request(url, data=json.dumps(body).encode('utf-8'),
+                      headers={'Content-Type': 'application/json'}, method='POST')
+    for attempt in range(3):
+        try:
+            with urlopen(request, timeout=90) as response:
+                raw = response.read()
+            break
+        except (TimeoutError, OSError):
+            if attempt == 2:
+                raise
+            time.sleep(1 + attempt)
+    text = raw.decode('utf-8')
+    try:
+        result = json.loads(text)
+    except json.JSONDecodeError:
+        result = text
+    if isinstance(result, dict) and (result.get('error') or result.get('success') is False):
+        raise RuntimeError(f'{endpoint}: {result}')
+    if isinstance(result, str) and result.lstrip().lower().startswith(('error', 'failed', 'no program')):
+        raise RuntimeError(f'{endpoint}: {result}')
+    return result, raw
+
+
@@ -745,0 +799,11 @@ def ghidra_cmd(args):
+    elif sub == 'instruction-context':
+        result, raw = ghidra_instruction_context(c, args.addresses, args.context)
+        if args.output:
+            path = (ROOT / args.output).resolve()
+            if not path.is_relative_to((ROOT / 'local').resolve()) or path.suffix != '.json':
+                sys.exit('Instruction-context output must be a JSON file under local/.')
+            path.parent.mkdir(parents=True, exist_ok=True)
+            path.write_bytes(raw)
+            print(f'Saved the full instruction-context response for {len(args.addresses)} addresses to {path.relative_to(ROOT).as_posix()}')
+        else:
+            cap(as_text(result), args.lines)
@@ -1043 +1107 @@ local/output/ and prints the path. --full or BSP_OUTPUT_BUDGET=0 lifts the cap.
-  ghidra count|proto|flow|xrefs|callers|callees|bytes|comments|decompile|disasm|documentation|export
+  ghidra count|proto|flow|instruction-context|xrefs|callers|callees|bytes|comments|decompile|disasm|documentation|export
@@ -1047,0 +1112 @@ local/output/ and prints the path. --full or BSP_OUTPUT_BUDGET=0 lifts the cap.
+      instruction-context <addresses> [--context 0..64] [--output local/response.json] reads exact listing starts
@@ -1129,0 +1195,3 @@ def main():
+    q = gs.add_parser('instruction-context', help='read instructions at exact addresses plus bounded listing neighbors; does not infer function flow')
+    q.add_argument('addresses', nargs='+', type=instruction_address); q.add_argument('--context', type=instruction_context_count, default=0)
+    q.add_argument('--output'); q.add_argument('--lines', '--limit', dest='lines', type=int, default=80)
```

## Two-pass seals

The seal payload is `tools/bsp.py`. Stable external inputs are
`tools/ghidra_export.py`, `config/target.json`, and the absolute bridge source path listed
above. The self-referential evidence file is excluded by its full relative path,
`docs/GHIDRA_INSTRUCTION_CONTEXT_FC.md`.

Before edit:

| Path | SHA256 | SHA512 |
| --- | --- | --- |
| `tools/bsp.py` | `2b58204b1364c71e17fa7abc1f099ec0597e53ab07b4c17f509b7e653f45c6b0` | `69fb2bfed9ac898118fff6f35c14f32ee8457deca94c113c774622786e0339431b2e93ba66ed5709d524ec36d9b41a05047413dc4b963772aba101d9d8802ca6` |
| `tools/ghidra_export.py` | `0367957cab3518c622104dc20309d571c0a253d0b0163f8e303ed409ccae0612` | `d94d997404a63db48d77bfadb1fd7e466fc7b9921833358e338a3d3b1fa04816f6072049c3057d283b1f08a14831f3bb02e79a70f5778cf7d93d87a7b3c9c886` |
| `config/target.json` | `366e5a64ac945473bb8d91607a78e56fee52e2579d38e66df3c240000b108ffb` | `4d63e102f408469c1bc103bc636873befeb5b8e7ced9254b7a64b03c91ed378680a0ac2c4f585b4f36bd572aa79a26404ae5595cddd1a1499be16d6188d7545c` |
| `J:/tools/ghidra-mcp/src/main/java/com/xebyte/core/XrefCallGraphService.java` | `cef01ac3ba70af7bd9c5b7bcbd4b1d7b3991680408a9689141bc0cf37e4463e5` | `7e4d8bf8a90160662c568a90f79bfdae95a229de0790139781c0f9bf72ee6378018c07bc727ebf4508fb67fa5a8dc0710f42292a43d0eea45a40ffdaa50b2ae8` |

Current after edit:

| Path | SHA256 | SHA512 |
| --- | --- | --- |
| `tools/bsp.py` | `4b439f59dedfdc914f88393d5b6106af7044fcf1a9a6fde5fee8c0cbce63fb81` | `747fc3df062d053d5479a15bf2fcf690dbfbe8f4d87540d725a534da9aa3b4139432e77d16c0721e82cf41992767a31c95b2a8356b1a1b042b9948e922347ebd` |
| `tools/ghidra_export.py` | `0367957cab3518c622104dc20309d571c0a253d0b0163f8e303ed409ccae0612` | `d94d997404a63db48d77bfadb1fd7e466fc7b9921833358e338a3d3b1fa04816f6072049c3057d283b1f08a14831f3bb02e79a70f5778cf7d93d87a7b3c9c886` |
| `config/target.json` | `366e5a64ac945473bb8d91607a78e56fee52e2579d38e66df3c240000b108ffb` | `4d63e102f408469c1bc103bc636873befeb5b8e7ced9254b7a64b03c91ed378680a0ac2c4f585b4f36bd572aa79a26404ae5595cddd1a1499be16d6188d7545c` |
| `J:/tools/ghidra-mcp/src/main/java/com/xebyte/core/XrefCallGraphService.java` | `cef01ac3ba70af7bd9c5b7bcbd4b1d7b3991680408a9689141bc0cf37e4463e5` | `7e4d8bf8a90160662c568a90f79bfdae95a229de0790139781c0f9bf72ee6378018c07bc727ebf4508fb67fa5a8dc0710f42292a43d0eea45a40ffdaa50b2ae8` |

The two algorithms were run independently in each pass. The three stable external inputs
retained identical values. Only the owned payload changed.

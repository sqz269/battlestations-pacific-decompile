// Repair listing defects the GhidraMCP bridge cannot: truncated function bodies, desynced
// instruction runs and code that sits under a defined data item.
//
// The bridge exposes disassemble_bytes and create_function but no way to clear code units, clear
// a fall-through override or set a function body, so a function whose flow stops early (GGame::OnMove
// 004e4a40 kept an 8-byte body after every bridge-side re-creation), a run of instructions decoded
// one byte late (00643c0c) and a routine under a data item (004c9800) stay wrong. This script runs
// inside Ghidra's Script Manager, where the program API is complete: for each range it clears the
// fall-through and flow overrides, clears every code unit in the range, disassembles from the start
// (flow-following), and creates or re-bodies the function at the start with the name it had.
//
// Usage: add the repository's tools/ghidra_scripts directory to the Script Manager's script
// directories, open battlestationspacific.exe in the CodeBrowser, run this script, check the console
// output (one line per range with the resulting body), then save the program. Re-run
// `python tools/bsp.py snapshot --force` and `python tools/bsp.py index` afterwards so the local
// index sees the new bodies. Add rows to RANGES for new defects; keep them in docs/GHIDRA_LISTING_DEFECTS.md.
//
// @category BSP
// @author Battlestations Pacific reconstruction

import ghidra.app.cmd.function.CreateFunctionCmd;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressSet;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Listing;
import ghidra.program.model.listing.FlowOverride;

public class RepairListingDefects extends GhidraScript {

    // {start, inclusive end, name to keep or apply ("" = keep whatever is there)}
    private static final String[][] RANGES = {
        {"004e4a40", "004e5537", "BSP_Game_OnMove"},                    // body stopped after MOV EAX,FS:[0]
        {"00643c0c", "00643c18", ""},                                   // one-byte-late decode inside the HUD marker routine
        {"004c9800", "004c981d", "BSP_SceneRecordPlayerSlot_Construct"}, // defined data blocked create_function
    };

    @Override
    public void run() throws Exception {
        Listing listing = currentProgram.getListing();
        for (String[] row : RANGES) {
            Address start = toAddr(Long.parseLong(row[0], 16));
            Address end = toAddr(Long.parseLong(row[1], 16));
            AddressSet range = new AddressSet(start, end);
            Function existing = listing.getFunctionContaining(start);
            String name = row[2];
            boolean createHere = !name.isEmpty();  // a named row starts a function; an unnamed row is a hole inside one
            if (name.isEmpty() && existing != null) {
                name = existing.getName();
            }
            boolean ownsEntry = existing != null && existing.getEntryPoint().equals(start);

            // 1. clear overrides that stop flow, then every code unit in the range
            InstructionIterator it = listing.getInstructions(range, true);
            while (it.hasNext()) {
                Instruction ins = it.next();
                if (ins.isFallThroughOverridden()) {
                    ins.clearFallThroughOverride();
                    println(row[0] + ": cleared fall-through override at " + ins.getAddress());
                }
                if (ins.getFlowOverride() != FlowOverride.NONE) {
                    ins.setFlowOverride(FlowOverride.NONE);
                    println(row[0] + ": cleared flow override at " + ins.getAddress());
                }
            }
            if (createHere && ownsEntry) {
                removeFunction(existing);
            }
            listing.clearCodeUnits(start, end, false);

            // 2. disassemble from the start, following flow
            disassemble(start);

            // 3. re-create the function at the start when the row names one
            if (createHere) {
                CreateFunctionCmd cmd = new CreateFunctionCmd(name.isEmpty() ? null : name, start, null,
                        ghidra.program.model.symbol.SourceType.USER_DEFINED, true, true);
                cmd.applyTo(currentProgram, monitor);
                Function created = listing.getFunctionAt(start);
                if (created == null) {
                    println(row[0] + ": FAILED to create a function: " + cmd.getStatusMsg());
                } else {
                    println(row[0] + ": " + created.getName() + " body " + created.getBody().getMinAddress()
                            + " - " + created.getBody().getMaxAddress());
                }
            } else {
                // the range lies inside an existing function: extend its body to cover the new units
                Function owner = listing.getFunctionContaining(start.subtract(1));
                if (owner != null) {
                    AddressSet body = new AddressSet(owner.getBody());
                    body.add(range);
                    owner.setBody(body);
                    println(row[0] + ": re-bodied " + owner.getName() + " to " + body.getMinAddress()
                            + " - " + body.getMaxAddress());
                } else {
                    println(row[0] + ": disassembled; no function owns the range");
                }
            }
        }
        println("Done. Save the program, then run: python tools/bsp.py snapshot --force; python tools/bsp.py index");
    }
}

// Ghidra headless script: creates functions at the listed addresses.
//
// Input: one line per function, "0xADDR<TAB>evidence". Addresses that already
// start a function are skipped; the evidence is stored as the plate comment so
// the reason a function exists stays visible in Ghidra.
//
//   analyzeHeadless OUTPUT\ghidra MH2 -process Manhunt2_nosecu.exe -noanalysis \
//       -scriptPath OUTPUT\tools -postScript CreateFunctions.java <list.txt>
//
//@category MH2

import java.io.BufferedReader;
import java.io.FileReader;

import ghidra.app.cmd.disassemble.DisassembleCommand;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class CreateFunctions extends GhidraScript {

    @Override
    public void run() throws Exception {
        int created = 0, existing = 0, failed = 0;
        try (BufferedReader br = new BufferedReader(new FileReader(getScriptArgs()[0]))) {
            String line;
            while ((line = br.readLine()) != null && !monitor.isCancelled()) {
                line = line.trim();
                if (line.isEmpty() || line.startsWith("#")) {
                    continue;
                }
                String[] p = line.split("\t", 2);
                Address a = toAddr(Long.parseLong(p[0].substring(2), 16));
                if (getFunctionAt(a) != null) {
                    existing++;
                    continue;
                }
                if (getInstructionAt(a) == null) {
                    new DisassembleCommand(a, null, true).applyTo(currentProgram, monitor);
                }
                Function f = createFunction(a, null);
                if (f == null) {
                    failed++;
                    println("  could not create " + p[0]);
                    continue;
                }
                setPlateComment(a, "Added from BinViz cross-check: " + (p.length > 1 ? p[1] : ""));
                created++;
            }
        }
        println("created: " + created + "   already existed: " + existing + "   failed: " + failed);
    }
}

// Restore the FMOD Ex SDK field names on the FMOD structs filled by tools/fmod_sdk_types.py.
//
// The GhidraMCP bridge rewrites every struct field it creates or renames into Hungarian
// notation (cbsize -> nCbsize, debugLogFilename -> pDebugLogFilename), and the bridge exposes
// no way to switch that off. This script runs inside Ghidra's Script Manager, where no such
// rule applies, and renames each component back to the name recorded in
// config/fmod_ex_types.json. Padding components keep the padding_<offset> spelling.
//
// Usage: add the repository's tools/ghidra_scripts directory to the Script Manager's script
// directories, open battlestationspacific.exe in the CodeBrowser, run this script, and enter
// the path of config/fmod_ex_types.json when asked. Save the program afterwards.
//
// @category BSP
// @author Battlestations Pacific reconstruction

import java.io.File;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.data.DataType;
import ghidra.program.model.data.DataTypeComponent;
import ghidra.program.model.data.DataTypeManager;
import ghidra.program.model.data.Structure;

public class RestoreFmodFieldNames extends GhidraScript {

    private static final String[] STRUCTS = {
        "FMOD_VECTOR", "FMOD_ADVANCEDSETTINGS", "FMOD_CREATESOUNDEXINFO", "FMOD_EVENT_INFO"
    };

    @Override
    public void run() throws Exception {
        File json = askFile("config/fmod_ex_types.json", "Load");
        String text = new String(Files.readAllBytes(json.toPath()), StandardCharsets.UTF_8);
        DataTypeManager dtm = currentProgram.getDataTypeManager();
        int renamed = 0;
        for (String name : STRUCTS) {
            List<String> fields = fieldNames(text, name);
            if (fields.isEmpty()) {
                printerr("no field list for " + name + " in " + json);
                continue;
            }
            Structure struct = findStruct(dtm, name);
            if (struct == null) {
                printerr("struct " + name + " not found in the program");
                continue;
            }
            DataTypeComponent[] components = struct.getDefinedComponents();
            for (int i = 0; i < components.length; i++) {
                DataTypeComponent component = components[i];
                String wanted;
                if (i < fields.size()) {
                    wanted = fields.get(i);
                } else {
                    wanted = "padding_" + Integer.toHexString(component.getOffset());
                }
                String current = component.getFieldName();
                if (!wanted.equals(current)) {
                    component.setFieldName(wanted);
                    println(name + " +0x" + Integer.toHexString(component.getOffset()) + ": "
                        + current + " -> " + wanted);
                    renamed++;
                }
            }
        }
        println("renamed " + renamed + " fields; save the program to keep them");
    }

    private static List<String> fieldNames(String text, String struct) {
        List<String> names = new ArrayList<>();
        int start = text.indexOf("\"structs\"");
        if (start < 0) {
            return names;
        }
        Matcher block = Pattern.compile("\"" + struct + "\":\\s*\\[(.*?)\\n\\s*\\]", Pattern.DOTALL)
            .matcher(text);
        if (!block.find(start)) {
            return names;
        }
        Matcher field = Pattern.compile("\\[\\s*\"(\\w+)\"\\s*,\\s*\"[^\"]*\"\\s*\\]").matcher(block.group(1));
        while (field.find()) {
            names.add(field.group(1));
        }
        return names;
    }

    private static Structure findStruct(DataTypeManager dtm, String name) {
        List<DataType> found = new ArrayList<>();
        dtm.findDataTypes(name, found);
        for (DataType dt : found) {
            if (dt instanceof Structure) {
                return (Structure) dt;
            }
        }
        return null;
    }
}

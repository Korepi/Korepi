import json
import re


def update_checksums(file_name: str, modules: dict):

    checksums = {
        "game_version": input("Input version: "),
        "modules": {}
    }

    for moduleName, moduleData in modules.items():
        checksums["modules"][moduleName] = moduleData["hash"]

    with open(file_name, "w") as file:
        json.dump(checksums, file, indent=2)


def apply_offsets(file_name: str, pattern: re.Pattern[str], offsets: dict, reverse_order=False):
    with open(file_name) as file:
        lines = file.readlines()

    for i, line in enumerate(lines):
        m = pattern.match(line)
        if m is None:
            continue

        filtered_groups = (group for group in m.groups() if group is not None)
        if reverse_order:
            name, prev_offset = filtered_groups
        else:
            prev_offset, name = filtered_groups

        if name not in offsets:
            print(f"Not found offset for {name}")
            continue

        offset = offsets[name]
        if int(offset, 16) != 0:
            lines[i] = line.replace(prev_offset, f"{int(offset, 16):08X}")

    with open(file_name, "w") as file:
        file.writelines(lines)


def process():
    with open("saved_offsets.json") as saved_offsets_file:
        content = json.load(saved_offsets_file)

    api_pattern = re.compile(r"#define *([\w\d_]+)_ptr *0x([A-Fa-f\d]+)")
    apply_offsets("../../src/appdata/il2cpp-api-functions-ptr.h", api_pattern, content["apiFunctions"], True)

    typedef_pattern = re.compile(r"(?:DO_TYPEDEF\(|DO_SINGLETONEDEF\() *0x([A-Fa-f\d]+), *([\w\d_]+) *\);")
    apply_offsets("../../src/appdata/il2cpp-types-ptr.h", typedef_pattern, content["typeInfo"])

    function_pattern = re.compile(r"DO_APP_FUNC *(?:\(|_METHODINFO\() *(?:0x|)([A-Fa-f\d]+),"
                                  r"(?:[^,]+, *([\w\d_]+), | *([\w\d_]+) *)")

    method_info_offsets: dict = content["methodInfo"]
    apply_offsets("../../src/appdata/il2cpp-functions.h", function_pattern,
                  dict(content["moduleInfo"]["UserAssembly.dll"]["functions"], **method_info_offsets))

    apply_offsets("../../src/appdata/il2cpp-unityplayer-functions.h", function_pattern,
                  dict(content["moduleInfo"]["UnityPlayer.dll"]["functions"], **method_info_offsets))

    update_checksums("../assembly_checksum.json", content["moduleInfo"])


if __name__ == "__main__":
    process()

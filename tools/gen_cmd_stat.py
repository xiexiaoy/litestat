#!/usr/bin/env python3
# Usage: ./generate_cmd_stat.py /path/to/commands.stat
# Output: /path/to/commands_stat.h  /path/to/commands_stat.cpp

import sys
import os

def parse_stat_file(path):
    namespace = None
    commands = []

    with open(path, "r") as f:
        for raw in f:
            line = raw.strip()

            # 忽略注释和空行
            if not line or line.startswith("#"):
                continue

            # 解析 namespace
            if namespace is None and line.startswith("namespace "):
                ns = line[len("namespace "):].strip()
                if ns:
                    namespace = ns
                continue

            # 解析 symbol: literal
            if ":" not in line:
                raise ValueError(f"Invalid format: {line}")

            sym, lit = line.split(":", 1)
            sym = sym.strip()
            lit = lit.strip()

            # 允许双引号或无引号
            if ((lit.startswith('"') and lit.endswith('"')) or
                (lit.startswith("'") and lit.endswith("'"))):
                lit = lit[1:-1]

            commands.append((sym, lit))

    return namespace, commands


def generate_header(namespace, commands, header_path):
    with open(header_path, "w") as h:
        h.write("#pragma once\n\n")
        h.write("#include <mutex>\n")
        h.write("#include <vector>\n")
        h.write("#include <iterator>\n\n")
        h.write('#include "litestat/cmd_stat.h"\n\n')

        if namespace:
            h.write(f"namespace {namespace} {{\n\n")

        for sym, lit in commands:
            h.write(f"class CmdStat_{sym} final : public litestat::CmdStatBase {{\n")
            h.write("public:\n")
            h.write(f'    CmdStat_{sym}() : CmdStatBase("{lit}") {{}}\n')
            h.write("    void OnEndStat(const litestat::Record &record) override;\n")
            h.write("    void ExportAndReset(\n")
            h.write("        std::back_insert_iterator<std::vector<litestat::ExportCmdStat>> out) override;\n")
            h.write("private:\n")
            h.write("    void EnsureTLSCmdStat(std::thread::id thread_id);\n")
            h.write("private:\n")
            h.write("    std::mutex cmd_stat_mux_;\n")
            h.write("    std::vector<litestat::TLSCmdStat *> cmd_stat_;\n")
            h.write("    static thread_local litestat::TLSCmdStat::Uptr tls_cmd_stat_;\n")
            h.write("};\n\n")
            h.write(f"extern CmdStat_{sym} {sym.lower()}_stat;\n\n")

        if namespace:
            h.write(f"}} // namespace {namespace}\n")


def generate_cpp(namespace, commands, header_path, cpp_path):
    header_filename = os.path.basename(header_path)

    with open(cpp_path, "w") as cpp:
        cpp.write(f'#include "{header_filename}"\n\n')
        cpp.write("#include <algorithm>\n\n")

        if namespace:
            cpp.write(f"namespace {namespace} {{\n\n")

        for sym, lit in commands:
            cpp.write(f"thread_local litestat::TLSCmdStat::Uptr CmdStat_{sym}::tls_cmd_stat_;\n")
        cpp.write("\n")
        for sym, lit in commands:
            cpp.write(f"void CmdStat_{sym}::OnEndStat(const litestat::Record &record) {{\n")
            cpp.write("    EnsureTLSCmdStat(std::this_thread::get_id());\n")
            cpp.write("    tls_cmd_stat_ ->OnEndStat(record);\n")
            cpp.write("}\n\n")
            cpp.write(f"void CmdStat_{sym}::ExportAndReset(\n")
            cpp.write("     std::back_insert_iterator<std::vector<litestat::ExportCmdStat>> out) {\n");
            cpp.write("}\n\n")
            cpp.write(f"void CmdStat_{sym}::EnsureTLSCmdStat(std::thread::id thread_id) {{\n")
            cpp.write("    if (tls_cmd_stat_ == nullptr) {\n")
            cpp.write("        std::lock_guard<std::mutex> lock(cmd_stat_mux_);\n")
            cpp.write("        auto less = [](const litestat::TLSCmdStat *tls_cmd_stat,\n")
            cpp.write("                       const std::thread::id thread_id) {\n")
            cpp.write("            return tls_cmd_stat->ThreadID() < thread_id;\n")
            cpp.write("        };\n")
            cpp.write("        auto it = std::lower_bound(cmd_stat_.begin(), cmd_stat_.end(),\n")
            cpp.write("                                   thread_id, less);\n")
            cpp.write("        if (it == cmd_stat_.end() || (*it)->ThreadID() != thread_id) {\n")
            cpp.write("            tls_cmd_stat_ = std::make_unique<litestat::TLSCmdStat>(thread_id, this);\n")
            cpp.write("            cmd_stat_.emplace(it, tls_cmd_stat_.get());\n")
            cpp.write("        }\n")
            cpp.write("    }\n")
            cpp.write("}\n\n")

        if namespace:
            cpp.write(f"\n}} // namespace {namespace}\n")


def main():
    if len(sys.argv) != 2:
        print("Usage: ./generate_cmd_stat.py commands.stat")
        sys.exit(1)

    input_file = os.path.abspath(sys.argv[1])

    # 输入文件所在目录，用于放输出文件
    input_dir = os.path.dirname(input_file)

    base_name = os.path.basename(input_file)
    base_no_ext = os.path.splitext(base_name)[0]

    header_path = os.path.join(input_dir, f"{base_no_ext}_stat.h")
    cpp_path = os.path.join(input_dir, f"{base_no_ext}_stat.cpp")

    namespace, commands = parse_stat_file(input_file)

    generate_header(namespace, commands, header_path)
    generate_cpp(namespace, commands, header_path, cpp_path)

    print(f"Generated:\n  {header_path}\n  {cpp_path}")
    if namespace:
        print(f"Namespace: {namespace}")
    print(f"Commands: {len(commands)}")


if __name__ == "__main__":
    main()

The following are options to decode the machine instructions:

1. disassemble the machine instruction from the file directly:
objdump -m i386:x86-64 -b binary --start-address=0x78 --stop-address=0x99 -D ./build/child

2. extract the machine instructions manually from the file and dump it with ZydisDisasm:
- somehow find the offset of where the machine instructions resides (read from ELF, etc...)
- example extracting with dd if=build/child of=raw.bin bs=1 skip=$((0x78)) count=$((0x99-0x78))
- ZydisDisasm -64 raw.bin (or objdump, or really any disassembler, etc...)

3. read the machine instructions directly without decode it from the runtime
you can try it by running tracer program (you might need to install disassembler in your head first, lol)

recent changes add support for decoding machine instructions with the help of the Zydis library; you don't need to install a disassembler in your head anymore!

current problem:
- currently no problem

TODOs:
- patching some instruction
- injecting some instruction

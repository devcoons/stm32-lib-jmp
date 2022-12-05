# JMP LIBRARY
Changing microcontroller execution. Used to support 2 or more firmware binaries be executed by the same microcontroller.

# Supported Hardware
- 
# Functions Guide
- `jmp_goto_address` : to jump from a firmware address to another.
- `jmp_validate_partition` : to validate a memory partition.
- `jmp_validate_firmware` : to validate the firmware code.
# How to use
- Add `lib-jmp` to you project.
- Add `lib-ifm` and `lib-crypto` to your project.
- Modify the `lib_jmp_config.h.example` as needed and rename it `lib_jmp_config.h`


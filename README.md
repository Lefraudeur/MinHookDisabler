# MinHook disabler

A small external program that will attempt to detect and disable MinHook hooks in a process.
Could be useful to disable some protections.
Like anticheats hooking the LoadLibrary function.

#### How it works:
For each .text section of each module loaded into the target process:
- Search for relative jmp (0xE9)
- Open the corresponding dll/module file, and see if there should be a jmp here or if it has been added
- If the dll file instructions, and the ones loaded into memory aren't the same, restore the original instructions
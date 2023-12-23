# MinHook disabler

A small external program that will attempt to detect and disable MinHook hooks in a process.
<br/>Could be useful to disable some protections.
<br/>For example some basic anticheats hook the LoadLibraryA function to prevent injection.

#### How it works:
For each module loaded into the target process:
- Search for relative jmp (0xE9) in the .text section
- Open the corresponding dll file, and see if there should be a jmp here or if it has been added
- If the dll file instructions differ from the ones loaded into memory, restore the original instructions

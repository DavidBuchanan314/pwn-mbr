# pwn-mbr
A simple MBR hijack demonstration


## Principles of operation:

### Phase 1: Injection

1. A malicious binary is run with root privileges.
2. The original MBR is copied to the next "free" location (first sector found containing only zeroes) on the disk, before the first partition. A magic number is appended, so that it can be found later.
3. The boot sector is overwritten with "malicious" code.

### Phase 2: Execution

1. Next time the machine is rebooted, the BIOS starts execution of the payload.
2. In this example, the text "MBR PWNED!" is written to the screen a few hundered times.
3. The payload locates the backup of the original boot sector via the magic number and copies it over to where it would normally reside in memory (`0x7C00`). However, this is where the payload is initially running from, so it copies itself elsewhere first.
4. Finally, the payload jumps back to `0x7C00`, resuming normal boot operations.


## Notes:

- Only works on BIOS/legacy boot systems.
- Although this demo doesn't do anything malicious, it is very possible that it corrupts your filesystem, so only run it on a dedicated VM unless you're very brave.


## Demo:

![Demo](https://github.com/DavidBuchanan314/pwn-mbr/raw/master/demo.gif)

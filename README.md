# minitools

## Simple Path Manager (spm)
Manage Windows PATH with `bin\spm.exe`. Run as admin for system `PATH`, otherwise affects user `PATH`. Changes persist in registry.
- `spm list`: Show all PATH entries with indexes
- `spm add <path>`: Add relative (e.g., `.`) or absolute (e.g., `C:\Test`) path
- `spm remove <index>`: Remove path by index from list

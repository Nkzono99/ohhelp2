# OhHelp v1 Documentation

This directory contains historical material from the original OhHelp line.
These files are references for migration and design comparison, not binding v2
specifications.

- [`original/`](original/) stores the source PDF documents.
- [`markdown/`](markdown/README.md) stores generated Markdown conversions.

Regenerate the Markdown from the repository root with:

```powershell
.\.venv\Scripts\python.exe scripts\convert_pdfs_to_md.py
```

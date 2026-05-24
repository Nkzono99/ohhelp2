# OhHelp PDF Markdown Documentation

Markdown conversions of the PDF documents in `doc/v1/original/`.

The documents are split by chapter or section. Long sections are further split on subsection boundaries where possible.

## Documents

- [OhHelp Library Package Manual](ohhelp-man/README.md) from `../original/ohhelp-man.pdf`
- [OhHelp Library Package Implementation Document](ohhelp/README.md) from `../original/ohhelp.pdf`

## Regeneration

From the repository root:

```powershell
.\.venv\Scripts\python.exe scripts\convert_pdfs_to_md.py
```

Python dependencies are listed in `requirements-doc.txt`.

# OhHelp PDF Markdown Documentation

Markdown conversions of the PDF documents in `doc/v1/original/`.

The documents are split by chapter or section. Long sections are further split on subsection boundaries where possible.

## Documents

- [OhHelp Library Package Manual](ohhelp-man/README.md) from `../original/ohhelp-man.pdf`
- [OhHelp Library Package Implementation Document](ohhelp/README.md) from `../original/ohhelp.pdf`

## Regeneration

From the repository root:

```sh
python3 scripts/convert_pdfs_to_md.py
```

To verify that the committed files match a fresh conversion:

```sh
bash scripts/check-v1-markdown.sh
```

Use Python 3.10 or newer. Dependencies are listed in `requirements-doc.txt`.
If the default `python3` is older, set `PYTHON`:

```sh
PYTHON=python3.11 bash scripts/check-v1-markdown.sh
```

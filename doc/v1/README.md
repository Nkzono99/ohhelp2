# OhHelp v1 Documentation

This directory contains historical material from the original OhHelp line.
These files are references for migration and design comparison, not binding v2
specifications.

- [`original/`](original/) stores the source PDF documents.
- [`markdown/`](markdown/README.md) stores generated Markdown conversions.

Regenerate the Markdown from the repository root with Python 3.10 or newer:

```sh
python3 scripts/convert_pdfs_to_md.py
```

Check that committed Markdown is reproducible with:

```sh
bash scripts/check-v1-markdown.sh
```

If the default `python3` is older, set `PYTHON`:

```sh
PYTHON=python3.11 bash scripts/check-v1-markdown.sh
```

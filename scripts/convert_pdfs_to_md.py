from __future__ import annotations

import re
import shutil
from dataclasses import dataclass
from pathlib import Path

import fitz


ROOT = Path(__file__).resolve().parents[1]
SOURCE_DIR = ROOT / "doc" / "v1" / "original"
OUT_DIR = ROOT / "doc" / "v1" / "markdown"

PDFS = {
    "ohhelp-man.pdf": {
        "title": "OhHelp Library Package Manual",
        "description": "Usage manual extracted from ohhelp-man.pdf.",
        "max_pages": 28,
    },
    "ohhelp.pdf": {
        "title": "OhHelp Library Package Implementation Document",
        "description": "Implementation document extracted from ohhelp.pdf.",
        "max_pages": 32,
    },
}

NUMBER_RE = re.compile(r"^\d+(?:\.\d+)*$")
NUMBER_TITLE_RE = re.compile(r"^(\d+(?:\.\d+)*)\s+(.+)$")
TOC_LINE_RE = re.compile(r"^\s*(\d+(?:\.\d+)*)\s+(.+?)\s+(\d+)\s*$")
SPECIAL_HEADINGS = {"Abstract", "Contents", "References", "Acknowledgments", "Index"}
LIGATURES = str.maketrans(
    {
        "ﬁ": "fi",
        "ﬂ": "fl",
        "ﬀ": "ff",
        "ﬃ": "ffi",
        "ﬄ": "ffl",
    }
)


def write_text_lf(path: Path, text: str) -> None:
    with path.open("w", encoding="utf-8", newline="\n") as f:
        f.write(text)


@dataclass(frozen=True)
class Heading:
    number: str | None
    title: str
    depth: int
    page: int


@dataclass(frozen=True)
class Segment:
    start: int
    end: int
    heading: Heading
    part: int | None = None


@dataclass(frozen=True)
class PdfPages:
    text: list[str]
    toc_headings: list[Heading]
    headings_by_page: dict[int, list[Heading]]
    front_end: int


def normalize_text(text: str) -> str:
    text = text.translate(LIGATURES)
    text = text.replace("\u0000", "")
    return text.replace("\r\n", "\n").replace("\r", "\n")


def clean_title(title: str) -> str:
    title = re.sub(r"\s+", " ", title).strip()
    title = title.strip(". ")
    return title or "Untitled"


def slugify(value: str) -> str:
    value = value.translate(LIGATURES).lower()
    value = value.replace("_", " ")
    value = re.sub(r"[^a-z0-9]+", "-", value)
    value = re.sub(r"-+", "-", value).strip("-")
    return value or "section"


def heading_from_lines(lines: list[str], page: int) -> list[Heading]:
    headings: list[Heading] = []
    scan_limit = min(14, len(lines))

    for idx in range(scan_limit):
        line = lines[idx].strip()
        if not line:
            continue
        if line in SPECIAL_HEADINGS:
            headings.append(Heading(None, line, 1, page))
            continue

        match = NUMBER_TITLE_RE.match(line)
        if match:
            number, title = match.groups()
            headings.append(
                Heading(number, clean_title(title), number.count(".") + 1, page)
            )
            continue

        if NUMBER_RE.match(line) and idx + 1 < len(lines):
            title = clean_title(lines[idx + 1])
            if title and not NUMBER_RE.match(title):
                headings.append(
                    Heading(line, title, line.count(".") + 1, page)
                )

    return headings


def line_is_dot_leader(line: str) -> bool:
    compact = line.replace(" ", "")
    return bool(compact) and set(compact) <= {"."}


def cleanup_toc_title(parts: list[str]) -> str:
    text = " ".join(part for part in parts if part.strip())
    text = re.sub(r"(?:\s+\.\s*){2,}", " ", text)
    return clean_title(text)


def parse_toc_entries(lines: list[str], page_count: int) -> list[tuple[str, str, int]]:
    entries: list[tuple[str, str, int]] = []
    for line in lines:
        match = TOC_LINE_RE.match(line)
        if not match:
            continue
        number, title, page_ref_text = match.groups()
        page_ref = int(page_ref_text)
        title = cleanup_toc_title([title])
        if title and 1 <= page_ref <= page_count:
            entries.append((number, title, page_ref))
    if entries:
        return entries

    idx = 0
    while idx < len(lines):
        number = lines[idx].strip()
        if not NUMBER_RE.match(number):
            idx += 1
            continue

        title_parts: list[str] = []
        cursor = idx + 1
        page_ref: int | None = None
        while cursor < len(lines):
            line = lines[cursor].strip()
            if not line or line_is_dot_leader(line):
                cursor += 1
                continue
            if NUMBER_RE.match(line) and title_parts:
                candidate = int(line.split(".")[0])
                if 1 <= candidate <= page_count:
                    page_ref = candidate
                    break
            if NUMBER_RE.match(line) and not title_parts:
                break
            title_parts.append(line)
            cursor += 1

        if page_ref is not None and title_parts:
            entries.append((number, cleanup_toc_title(title_parts), page_ref))
            idx = cursor + 1
        else:
            idx += 1
    return entries


def page_lines(text: str) -> list[str]:
    return [line.strip() for line in normalize_text(text).splitlines() if line.strip()]


def primary_heading(headings: list[Heading]) -> Heading | None:
    if not headings:
        return None
    numeric = [h for h in headings if h.number]
    if numeric:
        return numeric[0]
    return headings[0]


def collect_pages(pdf_path: Path) -> PdfPages:
    doc = fitz.open(pdf_path)
    pages: list[str] = []
    page_count = doc.page_count
    for page in doc:
        text = normalize_text(page.get_text("text", sort=True))
        pages.append(text)

    first_toc_page = next(
        (idx for idx, text in enumerate(pages, start=1) if page_lines(text)[:1] == ["Contents"]),
        None,
    )
    if first_toc_page is None:
        raise RuntimeError(f"No Contents page found in {pdf_path}")

    first_toc_text = normalize_text(doc[first_toc_page - 1].get_text("text", sort=True))
    first_entries = parse_toc_entries(page_lines(first_toc_text), page_count)
    if not first_entries:
        raise RuntimeError(f"Could not parse Contents entries in {pdf_path}")

    front_end = first_entries[0][2] - 1
    toc_lines: list[str] = []
    for page_no in range(first_toc_page, front_end + 1):
        toc_text = normalize_text(doc[page_no - 1].get_text("text", sort=True))
        toc_lines.extend(page_lines(toc_text))

    entries = parse_toc_entries(toc_lines, page_count)
    toc_headings: list[Heading] = []
    seen: set[tuple[str, str, int]] = set()
    for number, title, page_no in entries:
        key = (number, title, page_no)
        if key not in seen:
            toc_headings.append(
                Heading(number, title, number.count(".") + 1, page_no)
            )
            seen.add(key)

    headings_by_page: dict[int, list[Heading]] = {}
    for heading in toc_headings:
        headings_by_page.setdefault(heading.page, []).append(heading)

    for page_no, text in enumerate(pages, start=1):
        lines = page_lines(text)
        if lines and lines[0] in {"Acknowledgments", "Index", "References"}:
            heading = Heading(None, lines[0], 1, page_no)
            if heading not in headings_by_page.get(page_no, []):
                headings_by_page.setdefault(page_no, []).append(heading)

    for page_no in headings_by_page:
        headings_by_page[page_no].sort(key=lambda h: (h.depth, h.number or h.title))

    return PdfPages(pages, toc_headings, headings_by_page, front_end)


def should_start_segment(heading: Heading) -> bool:
    if heading.number is None:
        return heading.title in {"References", "Acknowledgments", "Index"}
    return heading.depth <= 2


def segment_starts(
    pdf_pages: PdfPages,
) -> list[Heading]:
    starts = [Heading("0", "Front Matter and Contents", 1, 1)]
    for heading in pdf_pages.toc_headings:
        if should_start_segment(heading):
            starts.append(heading)
    for page, headings in pdf_pages.headings_by_page.items():
        for heading in headings:
            if heading.number is None and should_start_segment(heading):
                starts.append(heading)
    deduped: dict[int, Heading] = {}
    for heading in starts:
        deduped.setdefault(heading.page, heading)
    return sorted(deduped.values(), key=lambda h: h.page)


def split_oversized_segment(
    start: int,
    end: int,
    heading: Heading,
    max_pages: int,
    split_headings: list[Heading],
) -> list[Segment]:
    if end - start + 1 <= max_pages:
        return [Segment(start, end, heading)]

    sub_starts = sorted(
        {
            heading.page
            for heading in split_headings
            if heading.number and heading.depth == 3 and start < heading.page <= end
        }
    )

    segments: list[Segment] = []
    cursor = start
    part = 1
    while cursor <= end:
        if end - cursor + 1 <= max_pages:
            segments.append(Segment(cursor, end, heading, part))
            break

        limit = min(end + 1, cursor + max_pages)
        aligned = [page for page in sub_starts if cursor < page <= limit]
        next_start = aligned[-1] if aligned else limit
        if next_start <= cursor:
            next_start = min(end + 1, cursor + max_pages)
        segments.append(Segment(cursor, next_start - 1, heading, part))
        cursor = next_start
        part += 1

    return segments


def build_segments(
    pdf_pages: PdfPages,
    max_pages: int,
) -> list[Segment]:
    page_count = len(pdf_pages.text)
    starts = segment_starts(pdf_pages)
    raw_segments: list[Segment] = []
    for idx, heading in enumerate(starts):
        next_page = starts[idx + 1].page if idx + 1 < len(starts) else page_count + 1
        raw_segments.append(Segment(heading.page, next_page - 1, heading))

    segments: list[Segment] = []
    for segment in raw_segments:
        segments.extend(
            split_oversized_segment(
                segment.start,
                segment.end,
                segment.heading,
                max_pages,
                pdf_pages.toc_headings,
            )
        )
    return segments


def heading_markdown(heading: Heading, file_heading: Heading) -> str:
    depth = heading.depth + 1 if heading.number else 2
    depth = min(max(depth, 2), 6)
    text = f"{heading.number} {heading.title}" if heading.number else heading.title
    if heading == file_heading:
        depth = 2
    return f"{'#' * depth} {text}"


def known_heading_for_lines(
    number: str | None,
    title: str,
    known_headings: list[Heading],
) -> Heading | None:
    title = clean_title(title)
    for heading in known_headings:
        if heading.number == number and clean_title(heading.title) == title:
            return heading
    return None


def convert_page_to_markdown(
    text: str,
    page_no: int,
    file_heading: Heading,
    known_headings: list[Heading],
) -> str:
    raw_lines = normalize_text(text).splitlines()
    while raw_lines and not raw_lines[-1].strip():
        raw_lines.pop()
    if raw_lines and raw_lines[-1].strip() == str(page_no):
        raw_lines.pop()

    out: list[str] = [f"<!-- Page {page_no} -->", ""]
    idx = 0
    while idx < len(raw_lines):
        line = raw_lines[idx].strip()
        if not line:
            out.append("")
            idx += 1
            continue

        if line in SPECIAL_HEADINGS and (
            line in {"Abstract", "Contents"}
            or known_heading_for_lines(None, line, known_headings)
        ):
            out.append(f"## {line}")
            idx += 1
            continue

        number_title = NUMBER_TITLE_RE.match(line)
        if number_title:
            number, title = number_title.groups()
            heading = known_heading_for_lines(number, title, known_headings)
            if heading:
                out.append(heading_markdown(heading, file_heading))
                idx += 1
                continue

        if NUMBER_RE.match(line) and idx + 1 < len(raw_lines):
            title = clean_title(raw_lines[idx + 1])
            if title and not NUMBER_RE.match(title):
                heading = known_heading_for_lines(line, title, known_headings)
                if heading:
                    out.append(heading_markdown(heading, file_heading))
                    idx += 2
                    continue

        if line.startswith("•"):
            out.append("- " + line[1:].strip())
        elif line.startswith("–"):
            out.append("  - " + line[1:].strip())
        else:
            out.append(line)
        idx += 1

    return "\n".join(out).rstrip() + "\n"


def segment_filename(segment: Segment) -> str:
    heading = segment.heading
    if heading.number == "0":
        prefix = "00"
    elif heading.number:
        prefix = heading.number.replace(".", "-").zfill(2)
    else:
        prefix = slugify(heading.title)
    title = slugify(heading.title)
    suffix = f"-part-{segment.part:02d}" if segment.part else ""
    return f"{prefix}-{title}{suffix}.md"


def write_document(pdf_name: str, spec: dict[str, object]) -> list[tuple[str, Segment]]:
    pdf_path = SOURCE_DIR / pdf_name
    pdf_pages = collect_pages(pdf_path)
    pages = pdf_pages.text
    segments = build_segments(pdf_pages, int(spec["max_pages"]))

    doc_dir = OUT_DIR / pdf_path.stem
    if doc_dir.exists():
        shutil.rmtree(doc_dir)
    doc_dir.mkdir(parents=True, exist_ok=True)

    written: list[tuple[str, Segment]] = []
    for segment in segments:
        filename = segment_filename(segment)
        title = (
            f"{segment.heading.number} {segment.heading.title}"
            if segment.heading.number and segment.heading.number != "0"
            else segment.heading.title
        )
        if segment.part:
            title = f"{title} - Part {segment.part}"

        body = [
            f"# {title}",
            "",
            f"Source: `doc/v1/original/{pdf_name}`, pages {segment.start}-{segment.end}.",
            "",
        ]
        for page_no in range(segment.start, segment.end + 1):
            body.append(
                convert_page_to_markdown(
                    pages[page_no - 1],
                    page_no,
                    segment.heading,
                    pdf_pages.headings_by_page.get(page_no, []),
                )
            )
            body.append("")

        write_text_lf(doc_dir / filename, "\n".join(body).rstrip() + "\n")
        written.append((filename, segment))

    readme_lines = [
        f"# {spec['title']}",
        "",
        str(spec["description"]),
        "",
        f"Source PDF: `../original/{pdf_name}`",
        f"Pages: {len(pages)}",
        f"Generated files: {len(written)}",
        "",
        "## Files",
        "",
    ]
    for filename, segment in written:
        title = (
            f"{segment.heading.number} {segment.heading.title}"
            if segment.heading.number and segment.heading.number != "0"
            else segment.heading.title
        )
        if segment.part:
            title = f"{title} - Part {segment.part}"
        readme_lines.append(
            f"- [{title}]({filename}) - pages {segment.start}-{segment.end}"
        )

    write_text_lf(doc_dir / "README.md", "\n".join(readme_lines) + "\n")
    return [(str(doc_dir.relative_to(OUT_DIR) / filename), segment) for filename, segment in written]


def write_root_readme(all_written: dict[str, list[tuple[str, Segment]]]) -> None:
    lines = [
        "# OhHelp PDF Markdown Documentation",
        "",
        "Markdown conversions of the PDF documents in `doc/v1/original/`.",
        "",
        "The documents are split by chapter or section. Long sections are further split on subsection boundaries where possible.",
        "",
        "## Documents",
        "",
    ]
    for pdf_name, spec in PDFS.items():
        stem = Path(pdf_name).stem
        lines.append(f"- [{spec['title']}]({stem}/README.md) from `../original/{pdf_name}`")
    lines.extend(
        [
            "",
            "## Regeneration",
            "",
            "From the repository root:",
            "",
            "```powershell",
            ".\\.venv\\Scripts\\python.exe scripts\\convert_pdfs_to_md.py",
            "```",
            "",
            "Python dependencies are listed in `requirements-doc.txt`.",
        ]
    )
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    write_text_lf(OUT_DIR / "README.md", "\n".join(lines) + "\n")


def main() -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    all_written: dict[str, list[tuple[str, Segment]]] = {}
    for pdf_name, spec in PDFS.items():
        all_written[pdf_name] = write_document(pdf_name, spec)
    write_root_readme(all_written)

    for pdf_name, files in all_written.items():
        print(f"{pdf_name}: wrote {len(files)} Markdown files")


if __name__ == "__main__":
    main()

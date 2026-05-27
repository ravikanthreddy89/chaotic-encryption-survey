#!/usr/bin/env python3
import sys
from pathlib import Path

if len(sys.argv) != 4:
    print("usage: paper1_fill_draft.py <draft_md> <tables_md> <out_md>")
    sys.exit(1)

draft_path = Path(sys.argv[1])
tables_path = Path(sys.argv[2])
out_path = Path(sys.argv[3])

draft = draft_path.read_text()
tables = tables_path.read_text()

# Very simple insertion: append tables section under Results.
needle = "## 6. Results\n"
if needle not in draft:
    print("Results section marker not found")
    sys.exit(1)

head, tail = draft.split(needle, 1)
new_text = head + needle + "\n" + tables + "\n\n" + tail
out_path.write_text(new_text)
print(f"written: {out_path}")

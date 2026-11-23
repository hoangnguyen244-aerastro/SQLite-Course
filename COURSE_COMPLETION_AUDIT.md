# Course completion audit — 23 November 2026

This file distinguishes **repository evidence** from **confirmed assignment completion**. It is not a substitute for the professor's instructions in `template/Tema_1.pdf` through `Tema_7.pdf` and `Term_paper_assignment.pdf`.

| Module | PDF reports found in repository | Still to check |
| --- | --- | --- |
| Tema 1 | b1, b2, b3, b4, b5, b6 | Compare each exercise with `Tema_1.pdf`; validate Windows builds |
| Tema 2 | b1, b2 | Compare with `Tema_2.pdf`; verify primary key and non-destructive save behavior |
| Tema 3 | b1, b2, b3 | Recreate SQL VIEW and verify results (command below) |
| Tema 4 | b1, b2 | Rebuild after latest code change; verify persistence after restarting; add missing Glade screenshot |
| Tema 5 | None found | Read and enumerate red assignment text from `Tema_5.pdf` before implementation |
| Tema 6 | None found | Read and enumerate red assignment text from `Tema_6.pdf` before implementation |
| Tema 7 | None found | Read and enumerate red assignment text from `Tema_7.pdf` before implementation |
| Term paper | No matching completed term-paper deliverable identified | Read `Term_paper_assignment.pdf`, choose subject area and implement required application |

**Important:** The number of reports is not proof of approval by the teacher. No fabricated screenshots, run results or completed flags should be added.

## Reproducibility improvement — Tema 3, exercise 3

The repository already includes `Tema3_b3_configured/01_query_employee_counts.sql` and `02_create_view.sql`; its README notes the tracked `b9.db` does not persist the newly created view. The new verification helper works on a **temporary backup copy** so the tracked original is never modified.

Run with Python 3:

```sh
python tools/verify_tema3_b3.py
# Or supply a different existing b9.db:
python tools/verify_tema3_b3.py /path/to/b9.db
```

Expected behavior: confirm the two required tables, run both supplied scripts, verify the VIEW exists and yields the same rows as the direct query. This is a reproducibility check, not proof that the GTK Windows build works.

## Manual evidence needed for Tema 4, exercise 2

1. Open `Tema4_b2_configured/b10/b10.cbp` in the supplied Code::Blocks environment.
2. Rebuild and run; use department form, select manager, save, exit and relaunch.
3. Verify the selected value survives the full restart and is stored in SQLite.
4. Capture a real Glade screenshot of `comboboxtext2`; append new screenshots and note the test date in the report.
5. Do not change the report to say these steps passed before they have been performed.

## Branch scope

This branch adds an audit and a **non-destructive** verification script. It intentionally does not claim completion of Tema 5–7 because the exact PDF exercises have not yet been extracted and checked.

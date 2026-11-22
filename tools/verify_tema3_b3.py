#!/usr/bin/env python3
"""Reproduce Tema 3, task 3 on a COPY of b9.db without altering the original."""
import argparse
import sqlite3
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
VIEW_NAME = "Количество_сотрудников_по_отделам"
DEFAULT_DB = ROOT / "Tema3_b2_configured/b9/bin/Debug/b9.db"

def verify(source: Path) -> None:
    if not source.is_file():
        raise FileNotFoundError(f"Database not found: {source}")
    with tempfile.TemporaryDirectory(prefix="tema3_b3_") as workdir:
        destination = Path(workdir) / "b9_copy.db"
        with sqlite3.connect(f"file:{source.resolve().as_posix()}?mode=ro", uri=True) as original:
            with sqlite3.connect(destination) as copy:
                original.backup(copy)
        with sqlite3.connect(destination) as con:
            objects = {r[0] for r in con.execute(
                "SELECT name FROM sqlite_master WHERE type='table'")}
            required = {"Отделы", "Сотрудники"}
            if not required.issubset(objects):
                raise RuntimeError(f"Missing tables: {sorted(required - objects)}")
            query = (ROOT / "Tema3_b3_configured/01_query_employee_counts.sql").read_text(encoding="utf-8")
            setup = (ROOT / "Tema3_b3_configured/02_create_view.sql").read_text(encoding="utf-8")
            before = con.execute(query).fetchall()
            con.executescript(setup)
            exists = con.execute("SELECT 1 FROM sqlite_master WHERE type='view' AND name=?", (VIEW_NAME,)).fetchone()
            if not exists:
                raise AssertionError("VIEW was not created")
            view_rows = con.execute(f'SELECT * FROM "{VIEW_NAME}"').fetchall()
            if sorted(before, key=repr) != sorted(view_rows, key=repr):
                raise AssertionError("VIEW result differs from the original query")
            print("OK: query and VIEW yield the same department/count rows.")
            for name, count in view_rows:
                print(f"  {name!r}: {count}")
            print(f"Original database unchanged: {source}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("database", nargs="?", type=Path, default=DEFAULT_DB)
    verify(parser.parse_args().database)

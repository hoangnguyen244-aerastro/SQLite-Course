#!/usr/bin/env python3
"""SQLite database deployment/check/backup tool. Python 3.8+ standard library.
Designed as a Tema 6 deployment demonstration, NOT a confirmed transcript
of the uninspected original Tema_6.pdf.
"""
import argparse
import sqlite3
import sys
import tempfile
import os
from pathlib import Path
from contextlib import closing

TABLES = ("Клиенты", "Услуги", "Заказы", "Позиции заказа")

def validate(db_path):
    path = Path(db_path)
    if not path.is_file():
        raise FileNotFoundError(path)
    # mode=ro avoids silently creating a database for a mistyped path.
    with closing(sqlite3.connect(path.resolve().as_uri() + "?mode=ro", uri=True)) as connection:
        integrity = connection.execute("PRAGMA integrity_check").fetchone()[0]
        if integrity != "ok":
            raise ValueError(f"integrity_check failed: {integrity}")
        fk = connection.execute("PRAGMA foreign_key_check").fetchall()
        if fk:
            raise ValueError(f"foreign_key_check failed: {fk[:3]}")
        actual = {r[0] for r in connection.execute(
            "SELECT name FROM sqlite_master WHERE type='table'"
        )}
        missing = set(TABLES) - actual
        if missing:
            raise ValueError(f"missing Tema5 photo-service tables: {sorted(missing)}")
        counts = {table: connection.execute(
            'SELECT COUNT(*) FROM "' + table + '"'
        ).fetchone()[0] for table in TABLES}
    return counts

def backup(source, dest, force=False):
    source, dest = Path(source), Path(dest)
    if source.resolve() == dest.resolve():
        raise ValueError("Source and destination must differ")
    counts = validate(source)
    dest.parent.mkdir(parents=True, exist_ok=True)
    if dest.exists() and not force:
        raise FileExistsError(f"Destination exists: {dest}; use --force")
    # Use SQLite Online Backup API rather than copying a live .db byte-for-byte.
    handle, filename = tempfile.mkstemp(suffix=".db", prefix=".sqlite_backup_", dir=dest.parent)
    os.close(handle)
    tmp = Path(filename)
    try:
        with closing(sqlite3.connect(source.resolve().as_uri() + "?mode=ro", uri=True)) as src:
            with closing(sqlite3.connect(tmp)) as dst:
                src.backup(dst)
        checked = validate(tmp)
        if checked != counts:
            raise ValueError(f"Counts differ between source and backup: {counts} vs {checked}")
        # Atomic replacement (within same directory); --force is explicit.
        os.replace(str(tmp), str(dest))
    finally:
        tmp.unlink(missing_ok=True)
    return counts

def main():
    parser = argparse.ArgumentParser(description="Deploy and validate Tema5 SQLite database")
    sub = parser.add_subparsers(dest="cmd", required=True)
    v = sub.add_parser("verify", help="Read-only integrity and foreign key check")
    v.add_argument("database")
    for command in ("deploy", "backup", "restore"):
        p = sub.add_parser(command, help="Safe SQLite online backup to a target path")
        p.add_argument("source")
        p.add_argument("destination")
        p.add_argument("--force", action="store_true", help="Explicitly replace existing destination")
    args = parser.parse_args()
    try:
        if args.cmd == "verify":
            counts = validate(args.database)
        else:
            counts = backup(args.source, args.destination, args.force)
        print("PASS: integrity_check = ok; foreign_key_check = []")
        for table, count in counts.items():
            print(f"  {table}: {count}")
        if args.cmd != "verify":
            print(f"PASS: {args.cmd} created {args.destination}")
    except Exception as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        return 1
    return 0

if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Verify Tema 5 task 1 SQL in an isolated temporary SQLite database."""
from pathlib import Path
import sqlite3

HERE = Path(__file__).resolve().parent
with sqlite3.connect(":memory:") as db:
    for filename in ("01_schema_3nf.sql", "02_seed.sql", "03_normal_forms_1nf_2nf.sql"):
        db.executescript((HERE / filename).read_text(encoding="utf-8"))
    counts = {
        table: db.execute('SELECT count(*) FROM "' + table + '"').fetchone()[0]
        for table in ("Клиенты", "Услуги", "Заказы", "Позиции заказа", "Заказы_1НФ", "Заказы_2НФ")
    }
    assert counts == {
        "Клиенты": 10, "Услуги": 5, "Заказы": 10, "Позиции заказа": 20,
        "Заказы_1НФ": 20, "Заказы_2НФ": 10
    }, counts
    invalid = db.execute("""
        SELECT c."Код клиента" FROM "Клиенты" c
        LEFT JOIN "Заказы" o ON o."Код клиента" = c."Код клиента"
        LEFT JOIN "Позиции заказа" p ON p."Код заказа" = o."Код заказа"
        GROUP BY c."Код клиента"
        HAVING COUNT(DISTINCT p."Код услуги") < 2
    """).fetchall()
    assert not invalid, invalid
    assert not db.execute("PRAGMA foreign_key_check").fetchall()
    assert db.execute("""
      SELECT COUNT(*) FROM "Позиции заказа" p
      JOIN "Заказы" o USING ("Код заказа")
      JOIN "Клиенты" c USING ("Код клиента")
      JOIN "Услуги" s USING ("Код услуги")
    """).fetchone()[0] == 20
    print("PASS:", counts)
    print("PASS: each client has >= 2 distinct services")
    print("PASS: foreign_key_check empty, 3NF join has 20 rows")

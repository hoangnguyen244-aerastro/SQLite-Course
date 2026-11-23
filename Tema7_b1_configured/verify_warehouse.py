#!/usr/bin/env python3
"""Read-only audit of b16.db and isolated in-memory stock movement smoke test.
Never modifies the on-disk database. Python 3.8+.
"""
import argparse
import sqlite3
from pathlib import Path

REQUIRED_TABLES = {"Товары", "Накладные", "Строки накладной"}
REQUIRED_VIEWS = {"Приходные накладные", "Расходные накладные"}

def check_file(path):
    dbpath = Path(path)
    if not dbpath.is_file():
        raise FileNotFoundError(dbpath)
    with sqlite3.connect(dbpath.resolve().as_uri()+"?mode=ro", uri=True) as original:
        with sqlite3.connect(":memory:") as temp:
            original.backup(temp)
            schemas = dict(temp.execute("SELECT name, type FROM sqlite_master"))
            missing = (REQUIRED_TABLES | REQUIRED_VIEWS) - schemas.keys()
            if missing:
                raise ValueError(f"Missing tables/views: {sorted(missing)}")
            view_sql = temp.execute(
                "SELECT sql FROM sqlite_master WHERE type='view' AND name='Расходные накладные'"
            ).fetchone()[0]
            if not ("Тип = 0" in view_sql or 'Тип=0' in view_sql):
                raise ValueError("Expected outgoing-invoice view with Тип=0")
            integrity = temp.execute("PRAGMA integrity_check").fetchone()[0]
            if integrity != "ok":
                raise ValueError(f"PRAGMA integrity_check: {integrity}")
            prod = temp.execute('SELECT "Код товара" FROM Товары LIMIT 1').fetchone()
            if prod is None:
                temp.execute('INSERT INTO Товары(Товар,Артикул,Количество) '
                             'VALUES(?,?,?)', ("Контрольный товар", "TEST", 10))
                product = temp.execute("SELECT last_insert_rowid()").fetchone()[0]
            else:
                product = prod[0]
                temp.execute('UPDATE Товары SET Количество=10 WHERE "Код товара"=?',
                             (product,))
            temp.execute("INSERT INTO Накладные(Тип,Дата,Номер,Сумма) "
                         "VALUES(0,'2026-11-23',500,0)")
            docid = temp.execute("SELECT last_insert_rowid()").fetchone()[0]
            temp.execute('UPDATE Товары SET Количество=Количество-3 '
                         'WHERE "Код товара"=? AND Количество>=3', (product,))
            assert temp.execute("SELECT changes()").fetchone()[0] == 1
            temp.execute('INSERT INTO "Строки накладной" '
                         '("Код товара","Код накладной",Приход,Расход) '
                         'VALUES(?,?,0,3)', (product, docid))
            assert temp.execute('SELECT Количество FROM Товары '
                                'WHERE "Код товара"=?', (product,)).fetchone()[0] == 7
            out = temp.execute('SELECT COUNT(*) FROM "Расходные накладные" '
                               'WHERE "Код накладной"=?', (docid,)).fetchone()[0]
            assert out == 1
            temp.rollback()
            print("PASS: required tables and views, integrity_check=ok")
            print("PASS: outgoing view Тип=0, simulated stock 10 -> 7")
            print("PASS: original database unchanged (test used in-memory copy)")

if __name__ == "__main__":
    ap=argparse.ArgumentParser()
    ap.add_argument("db",nargs="?",default="b16/bin/Debug/b16.db")
    check_file(ap.parse_args().db)

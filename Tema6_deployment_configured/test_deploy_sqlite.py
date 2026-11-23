#!/usr/bin/env python3
import sqlite3
import tempfile
import unittest
from pathlib import Path
from deploy_sqlite import backup, validate

class DeploymentTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.src = self.root / "source.db"
        with sqlite3.connect(self.src) as db:
            db.executescript("""
                PRAGMA foreign_keys=ON;
                CREATE TABLE "Клиенты"(id INTEGER PRIMARY KEY);
                CREATE TABLE "Услуги"(id INTEGER PRIMARY KEY);
                CREATE TABLE "Заказы"(id INTEGER PRIMARY KEY, client INTEGER REFERENCES "Клиенты"(id));
                CREATE TABLE "Позиции заказа"(id INTEGER PRIMARY KEY, ord INTEGER REFERENCES "Заказы"(id));
                INSERT INTO "Клиенты" VALUES(1);
                INSERT INTO "Услуги" VALUES(1);
                INSERT INTO "Заказы" VALUES(1,1);
                INSERT INTO "Позиции заказа" VALUES(1,1);
            """)
    def test_validate_and_backup(self):
        self.assertEqual(validate(self.src)["Клиенты"], 1)
        dst=self.root/"output"/"backup.db"
        self.assertEqual(backup(self.src,dst)["Заказы"],1)
        self.assertEqual(validate(dst)["Позиции заказа"],1)
        with self.assertRaises(FileExistsError):
            backup(self.src,dst)
        backup(self.src,dst,force=True)
    def test_corrupt_reference_detected(self):
        with sqlite3.connect(self.src) as db:
            db.execute('PRAGMA foreign_keys=OFF')
            db.execute('INSERT INTO "Заказы" VALUES(2,999)')
        with self.assertRaises(ValueError):
            validate(self.src)

if __name__ == "__main__":
    unittest.main(verbosity=2)

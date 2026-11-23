#!/usr/bin/env python3
import sqlite3
import tempfile
import unittest
import subprocess
import sys
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

    def test_cli_verify_deploy_backup_restore(self):
        tool = Path(__file__).with_name("deploy_sqlite.py")
        deployed = self.root / "deployed" / "result.db"
        backed_up = self.root / "backups" / "result.db"
        restored = self.root / "restored" / "result.db"
        for args in [
            ["verify", str(self.src)],
            ["deploy", str(self.src), str(deployed)],
            ["backup", str(deployed), str(backed_up)],
            ["restore", str(backed_up), str(restored)],
            ["verify", str(restored)],
        ]:
            run = subprocess.run(
                [sys.executable, str(tool), *args],
                capture_output=True, text=True, encoding="utf-8", errors="replace",
            )
            self.assertEqual(run.returncode, 0, run.stdout + "\n" + run.stderr)
        self.assertEqual(validate(restored)["Клиенты"], 1)

    def test_missing_database_rejected(self):
        with self.assertRaises(FileNotFoundError):
            validate(self.root / "does_not_exist.db")

if __name__ == "__main__":
    unittest.main(verbosity=2)

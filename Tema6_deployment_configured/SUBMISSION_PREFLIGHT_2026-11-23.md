# Tema 6 — submission preflight (23 November 2026)

**Status: NOT READY FOR FINAL SUBMISSION.** This branch hardens the *existing demonstrator*, not a verified implementation of the original assignment. The exact red-highlighted tasks in `template/Tema_6.pdf` have not been inspected; do not claim that this demonstrator satisfies them.

## Changes
- `deploy_sqlite.py`: explicitly closes read-only and temporary SQLite connections before replacing a backup file (especially important on Windows).
- `test_deploy_sqlite.py`: adds CLI round-trip checks for verify, deploy, backup and restore, plus a missing-file test.
- Local Linux verification of the revised test suite: **4 tests passed** with a synthetic four-table database. This is NOT a Windows run or a test of the tracked Fotосервис database.

## Windows checklist
1. In GitHub Desktop switch to `fix/tema6-close-sqlite-connections-20261123`, fetch/pull and open `Tema6_deployment_configured`.
2. Keep a separate backup of `source/Tema5_b1_Fotoservice.db`. Use Python 3.8+ in PowerShell.
3. Run:
```powershell
python -m unittest test_deploy_sqlite.py -v
python deploy_sqlite.py verify source/Tema5_b1_Fotoservice.db
python deploy_sqlite.py deploy source/Tema5_b1_Fotoservice.db deployed/Fotoservice.db
python deploy_sqlite.py verify deployed/Fotoservice.db
python deploy_sqlite.py backup deployed/Fotoservice.db backups/Fotoservice_backup.db
python deploy_sqlite.py restore backups/Fotoservice_backup.db restored/Fotoservice_restored.db
python deploy_sqlite.py verify restored/Fotoservice_restored.db
```
These create destination files and refuse to overwrite existing ones without `--force`. For a repeat run, use NEW destination paths; never remove the original database.
4. In SQLiteStudio open the original/deployed/restored DB, run `PRAGMA integrity_check;` (expect 'ok') and `PRAGMA foreign_key_check;` (expect 0 rows). Compare record counts for the tables Клиенты, Услуги, Заказы, Позиции заказа.
5. Take genuine screenshots of the PowerShell output, DB tables, integrity checks and the restored DB.
6. **Critical:** Open `template/Tema_6.pdf` and record every red-highlighted assignment verbatim. Compare with this project and implement all actual required tasks, including Code::Blocks/GTK exercises if demanded.
7. Only AFTER that, replace `REPORT_DRAFT_RU.md` with a verified final Russian PDF report, including task text, code, real screenshots and conclusions; submit via the instructor's VK channel and save the submission confirmation.

Deadline in the supplied VK screenshots: **24 November 2026** (assuming they refer to this academic year).

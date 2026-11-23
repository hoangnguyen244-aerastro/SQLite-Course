# Тема 6 — Развёртывание базы данных (учебный стенд)

**Статус: подготовлен и проверяемый пример развёртывания SQLite.**
**Внимание:** текст заданий из `template/Tema_6.pdf` не удалось получить из GitHub из-за ограничения загрузки PDF. Поэтому данный пакет — демонстрация развёртывания на базе завершённой работы Tema5_b1, но его **нельзя считать подтверждённым выполнением всех заданий Темы 6**, пока не будет проверен исходный PDF.

## Входные данные
Исходная учебная БД `Tema5_b1_Fotoservice.db` размещена в каталоге `source` этого пакета; это отдельная копия и она не должна заменять исходные работы Темы 5.

## Требования
Windows с Python 3.8+ или Linux/macOS с Python 3.8+. Используется только стандартная библиотека `sqlite3`.

## Команды запуска
Откройте терминал в каталоге `Tema6_deployment_configured`:
```
python deploy_sqlite.py verify source/Tema5_b1_Fotoservice.db
python deploy_sqlite.py deploy source/Tema5_b1_Fotoservice.db deployed/Fotoservice.db
python deploy_sqlite.py verify deployed/Fotoservice.db
python deploy_sqlite.py backup deployed/Fotoservice.db backups/Fotoservice_backup.db
python deploy_sqlite.py restore backups/Fotoservice_backup.db restored/Fotoservice_restored.db
python -m unittest test_deploy_sqlite.py -v
```

Команды `deploy`, `backup`, `restore` используют SQLite Online Backup API, а не простое копирование открытой базы.
Проверка выполняет `PRAGMA integrity_check`, `PRAGMA foreign_key_check`, проверку имён основных таблиц, а также числа строк. Нельзя перезаписывать существующий путь без `--force`. При ошибке команда возвращает ненулевой код завершения.

## Что фотографировать для отчёта после выполнения
1. `verify` оригинальной базы и вывод целостности;
2. создание `deployed/Fotoservice.db`;
3. таблицы развернутой базы в SQLiteStudio;
4. резервное копирование в `backups`;
5. восстановление из резервной копии и контроль совпадения таблиц.

**Пока не записывать в отчёт «проверено в Windows»: пользователь ещё не запустил эти действия.**

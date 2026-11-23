-- Тема 5, задание 1. Фотосервис. Итоговая структура 3НФ.
PRAGMA foreign_keys = ON;
CREATE TABLE IF NOT EXISTS "Клиенты" (
 "Код клиента" INTEGER PRIMARY KEY,
 "Фамилия" TEXT NOT NULL,
 "Имя" TEXT NOT NULL,
 "Отчество" TEXT NOT NULL,
 "Домашний адрес" TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS "Услуги" (
 "Код услуги" INTEGER PRIMARY KEY,
 "Наименование услуги" TEXT NOT NULL UNIQUE,
 "Базовая цена" NUMERIC NOT NULL CHECK ("Базовая цена" >= 0)
);
CREATE TABLE IF NOT EXISTS "Заказы" (
 "Код заказа" INTEGER PRIMARY KEY,
 "Код клиента" INTEGER NOT NULL REFERENCES "Клиенты"("Код клиента"),
 "Дата приемки" TEXT NOT NULL,
 "Дата выдачи" TEXT NOT NULL,
 CHECK ("Дата выдачи" >= "Дата приемки")
);
CREATE TABLE IF NOT EXISTS "Позиции заказа" (
 "Код заказа" INTEGER NOT NULL REFERENCES "Заказы"("Код заказа") ON DELETE CASCADE,
 "Код услуги" INTEGER NOT NULL REFERENCES "Услуги"("Код услуги"),
 "Количество единиц заказа" INTEGER NOT NULL CHECK ("Количество единиц заказа" > 0),
 "Цена за единицу" NUMERIC NOT NULL CHECK ("Цена за единицу" >= 0),
 PRIMARY KEY ("Код заказа","Код услуги")
);
CREATE INDEX IF NOT EXISTS idx_orders_client ON "Заказы"("Код клиента");

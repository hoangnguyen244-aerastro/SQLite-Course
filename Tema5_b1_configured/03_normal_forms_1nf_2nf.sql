-- 1НФ: атомарные поля, одна услуга на строку, составной ключ.
-- Есть частичные зависимости от Код заказа и Код услуги; это ещё не 2НФ.
CREATE TABLE "Заказы_1НФ" (
 order_id INTEGER, service_id INTEGER, client_id INTEGER,
 last_name TEXT, first_name TEXT, patronymic TEXT, address TEXT,
 service_name TEXT, quantity INTEGER, unit_price NUMERIC,
 accepted_on TEXT, issued_on TEXT, PRIMARY KEY(order_id,service_id)
);
INSERT INTO "Заказы_1НФ"
SELECT o."Код заказа",s."Код услуги",c."Код клиента",
c."Фамилия",c."Имя",c."Отчество",c."Домашний адрес",
s."Наименование услуги",p."Количество единиц заказа",p."Цена за единицу",
o."Дата приемки",o."Дата выдачи"
FROM "Позиции заказа" p
JOIN "Заказы" o ON o."Код заказа"=p."Код заказа"
JOIN "Клиенты" c ON c."Код клиента"=o."Код клиента"
JOIN "Услуги" s ON s."Код услуги"=p."Код услуги";
-- 2НФ: устраняем зависимости от частей составного ключа.
-- В Заказы_2НФ остаётся транзитивная зависимость: order_id -> client_id -> client_details.
CREATE TABLE "Заказы_2НФ" (
 order_id INTEGER PRIMARY KEY, client_id INTEGER,
 last_name TEXT, first_name TEXT, patronymic TEXT, address TEXT,
 accepted_on TEXT, issued_on TEXT
);
INSERT INTO "Заказы_2НФ"
SELECT o."Код заказа",c."Код клиента",c."Фамилия",c."Имя",
c."Отчество",c."Домашний адрес",o."Дата приемки",o."Дата выдачи"
FROM "Заказы" o JOIN "Клиенты" c ON c."Код клиента"=o."Код клиента";
CREATE TABLE "Услуги_2НФ" (service_id INTEGER PRIMARY KEY,service_name TEXT,base_price NUMERIC);
INSERT INTO "Услуги_2НФ" SELECT "Код услуги","Наименование услуги","Базовая цена" FROM "Услуги";
CREATE TABLE "Позиции_2НФ" (
 order_id INTEGER,service_id INTEGER,quantity INTEGER,unit_price NUMERIC,
 PRIMARY KEY(order_id,service_id)
);
INSERT INTO "Позиции_2НФ"
SELECT "Код заказа","Код услуги","Количество единиц заказа","Цена за единицу" FROM "Позиции заказа";
-- 3НФ: Клиенты / Услуги / Заказы / Позиции заказа (см. 01_schema_3nf.sql).

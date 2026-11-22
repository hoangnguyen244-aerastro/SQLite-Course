-- Файл предназначен для SQLiteStudio, возвращает проверяемые выборки.
PRAGMA foreign_key_check;
SELECT 'clients' AS metric, COUNT(*) AS value FROM "Клиенты"
UNION ALL SELECT 'services',COUNT(*) FROM "Услуги"
UNION ALL SELECT 'orders',COUNT(*) FROM "Заказы"
UNION ALL SELECT 'items',COUNT(*) FROM "Позиции заказа"
UNION ALL SELECT '1nf_rows',COUNT(*) FROM "Заказы_1НФ"
UNION ALL SELECT '2nf_orders',COUNT(*) FROM "Заказы_2НФ";
-- Результат должен быть пустым: у каждого клиента не меньше двух РАЗНЫХ фотоуслуг.
SELECT c."Код клиента",c."Фамилия",COUNT(DISTINCT p."Код услуги") AS service_count
FROM "Клиенты" c LEFT JOIN "Заказы" o ON o."Код клиента"=c."Код клиента"
LEFT JOIN "Позиции заказа" p ON p."Код заказа"=o."Код заказа"
GROUP BY c."Код клиента" HAVING service_count < 2;
-- Полный вид данных и расчёт суммы заказа.
SELECT c."Фамилия",c."Имя",s."Наименование услуги",
p."Количество единиц заказа",p."Цена за единицу",
p."Количество единиц заказа"*p."Цена за единицу" AS "Стоимость позиции",
o."Дата приемки",o."Дата выдачи"
FROM "Позиции заказа" p
JOIN "Заказы" o ON o."Код заказа"=p."Код заказа"
JOIN "Клиенты" c ON c."Код клиента"=o."Код клиента"
JOIN "Услуги" s ON s."Код услуги"=p."Код услуги"
ORDER BY o."Код заказа",s."Код услуги";

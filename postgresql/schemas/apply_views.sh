#!/bin/bash

# Скрипт для применения SQL views к базе данных

set -e

# Цвета для вывода
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== Применение SQL Views ===${NC}"

# Параметры подключения (можно переопределить через переменные окружения)
DB_HOST="${DB_HOST:-localhost}"
DB_PORT="${DB_PORT:-5432}"
DB_NAME="${DB_NAME:-game_userver_db_1}"
DB_USER="${DB_USER:-user}"

# Путь к файлу с views
VIEWS_FILE="$(dirname "$0")/views.sql"

if [ ! -f "$VIEWS_FILE" ]; then
    echo -e "${RED}Ошибка: Файл $VIEWS_FILE не найден${NC}"
    exit 1
fi

echo -e "${BLUE}Параметры подключения:${NC}"
echo "  Host: $DB_HOST"
echo "  Port: $DB_PORT"
echo "  Database: $DB_NAME"
echo "  User: $DB_USER"
echo ""

# Проверка доступности PostgreSQL
echo -e "${BLUE}Проверка подключения к базе данных...${NC}"
if ! PGPASSWORD="$DB_PASSWORD" psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -c "SELECT 1" > /dev/null 2>&1; then
    echo -e "${RED}Ошибка: Не удалось подключиться к базе данных${NC}"
    echo "Убедитесь, что:"
    echo "  1. PostgreSQL запущен"
    echo "  2. База данных существует"
    echo "  3. Переменная окружения DB_PASSWORD установлена"
    echo ""
    echo "Пример использования:"
    echo "  export DB_PASSWORD='your_password'"
    echo "  ./apply_views.sh"
    exit 1
fi

echo -e "${GREEN}✓ Подключение успешно${NC}"
echo ""

# Применение views
echo -e "${BLUE}Применение SQL views...${NC}"
if PGPASSWORD="$DB_PASSWORD" psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -f "$VIEWS_FILE"; then
    echo ""
    echo -e "${GREEN}✓ Views успешно применены${NC}"
    echo ""
    
    # Список созданных views
    echo -e "${BLUE}Созданные views:${NC}"
    PGPASSWORD="$DB_PASSWORD" psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -c "
        SELECT schemaname, viewname 
        FROM pg_views 
        WHERE schemaname = 'quiz' 
        ORDER BY viewname;
    "
    
    echo ""
    echo -e "${GREEN}Готово! Теперь вы можете использовать views для анализа данных.${NC}"
    echo ""
    echo "Примеры запросов:"
    echo "  SELECT * FROM quiz.game_sessions_view;"
    echo "  SELECT * FROM quiz.players_view;"
    echo "  SELECT * FROM quiz.top_players_view LIMIT 10;"
    echo ""
    echo "Подробная документация: docs/DATABASE_VIEWS.md"
else
    echo ""
    echo -e "${RED}✗ Ошибка при применении views${NC}"
    exit 1
fi
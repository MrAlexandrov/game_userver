# Быстрый старт Telegram Bot

## За 5 минут

### 1. Установка Poetry (если еще не установлен)

```bash
curl -sSL https://install.python-poetry.org | python3 -
```

### 2. Установка зависимостей

```bash
cd telegram_bot
poetry install
```

### 3. Настройка

```bash
# Скопируйте пример конфигурации
cp .env.example .env

# Отредактируйте .env и укажите ваш токен от @BotFather
nano .env  # или используйте любой редактор
```

В файле `.env` укажите:
```bash
TELEGRAM_BOT_TOKEN=ваш_токен_от_BotFather
API_BASE_URL=http://localhost:8080
```

### 4. Убедитесь, что backend запущен

```bash
# В другом терминале, из корня проекта
cd build
./game_userver --config ../configs/static_config.yaml --config_vars ../configs/config_vars.yaml
```

### 5. Создайте тестовые данные (если еще не создали)

```bash
# Из корня проекта
python3 create_test_data.py
```

### 6. Запустите бота

```bash
# Из папки telegram_bot
poetry run python bot.py
```

### 7. Играйте!

1. Найдите вашего бота в Telegram
2. Отправьте `/start`
3. Отправьте `/newgame`
4. Выберите пак вопросов
5. Начните игру и отвечайте на вопросы!

## Альтернатива: без Poetry

Если вы предпочитаете pip:

```bash
cd telegram_bot
pip install -r requirements.txt
python3 bot.py
```

## Проблемы?

- **Бот не отвечает**: Проверьте токен в `.env`
- **Нет паков**: Запустите `create_test_data.py`
- **Ошибка подключения**: Убедитесь, что backend запущен на порту 8080

Подробнее см. [README.md](README.md)
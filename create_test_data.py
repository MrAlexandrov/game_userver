#!/usr/bin/env python3
"""
Скрипт для создания тестовых данных для викторины
"""

import requests
import sys

API_URL = "http://localhost:8080"


def create_pack(title):
    """Создать пак вопросов"""
    response = requests.post(
        f"{API_URL}/create-pack",
        json={"title": title}
    )
    response.raise_for_status()
    return response.json()["id"]


def create_question(pack_id, text, image_url=None):
    """Создать вопрос"""
    response = requests.post(
        f"{API_URL}/create-question",
        json={
            "pack_id": pack_id,
            "text": text,
            "image_url": image_url
        }
    )
    response.raise_for_status()
    return response.json()["id"]


def create_variant(question_id, text, is_correct):
    """Создать вариант ответа"""
    response = requests.post(
        f"{API_URL}/create-variant",
        json={
            "question_id": question_id,
            "text": text,
            "is_correct": is_correct
        }
    )
    response.raise_for_status()
    return response.json()["id"]


def create_sample_quiz():
    """Создать пример викторины"""
    print("🎮 Создание тестовой викторины...")
    
    # Создаем пак
    print("\n📦 Создание пака вопросов...")
    pack_id = create_pack("Общие знания")
    print(f"✅ Пак создан с ID: {pack_id}")
    
    # Вопрос 1
    print("\n❓ Создание вопроса 1...")
    q1_id = create_question(
        pack_id,
        "Какая столица России?"
    )
    create_variant(q1_id, "Москва", True)
    create_variant(q1_id, "Санкт-Петербург", False)
    create_variant(q1_id, "Казань", False)
    create_variant(q1_id, "Новосибирск", False)
    print("✅ Вопрос 1 создан")
    
    # Вопрос 2
    print("\n❓ Создание вопроса 2...")
    q2_id = create_question(
        pack_id,
        "Сколько будет 2 + 2?"
    )
    create_variant(q2_id, "3", False)
    create_variant(q2_id, "4", True)
    create_variant(q2_id, "5", False)
    create_variant(q2_id, "22", False)
    print("✅ Вопрос 2 создан")
    
    # Вопрос 3
    print("\n❓ Создание вопроса 3...")
    q3_id = create_question(
        pack_id,
        "Какой язык программирования используется в этом проекте для backend?"
    )
    create_variant(q3_id, "Python", False)
    create_variant(q3_id, "C++", True)
    create_variant(q3_id, "Java", False)
    create_variant(q3_id, "Go", False)
    print("✅ Вопрос 3 создан")
    
    # Вопрос 4
    print("\n❓ Создание вопроса 4...")
    q4_id = create_question(
        pack_id,
        "Какой фреймворк используется для создания HTTP API?"
    )
    create_variant(q4_id, "Django", False)
    create_variant(q4_id, "Flask", False)
    create_variant(q4_id, "userver", True)
    create_variant(q4_id, "FastAPI", False)
    print("✅ Вопрос 4 создан")
    
    # Вопрос 5
    print("\n❓ Создание вопроса 5...")
    q5_id = create_question(
        pack_id,
        "Какая база данных используется в проекте?"
    )
    create_variant(q5_id, "MySQL", False)
    create_variant(q5_id, "MongoDB", False)
    create_variant(q5_id, "PostgreSQL", True)
    create_variant(q5_id, "SQLite", False)
    print("✅ Вопрос 5 создан")
    
    print("\n" + "="*50)
    print("🎉 Тестовая викторина успешно создана!")
    print(f"📦 ID пака: {pack_id}")
    print("="*50)
    print("\n💡 Теперь вы можете запустить Telegram бота и начать игру!")
    print("   Используйте команду /newgame в боте")


def create_programming_quiz():
    """Создать викторину по программированию"""
    print("🎮 Создание викторины по программированию...")
    
    pack_id = create_pack("Программирование")
    print(f"✅ Пак создан с ID: {pack_id}")
    
    questions = [
        {
            "text": "Что такое рекурсия?",
            "variants": [
                ("Цикл for", False),
                ("Функция, вызывающая саму себя", True),
                ("Тип данных", False),
                ("Оператор условия", False)
            ]
        },
        {
            "text": "Какая сложность у бинарного поиска?",
            "variants": [
                ("O(n)", False),
                ("O(log n)", True),
                ("O(n²)", False),
                ("O(1)", False)
            ]
        },
        {
            "text": "Что означает SOLID в программировании?",
            "variants": [
                ("Название языка", False),
                ("Принципы ООП", True),
                ("Тип данных", False),
                ("Паттерн проектирования", False)
            ]
        }
    ]
    
    for i, q_data in enumerate(questions, 1):
        print(f"\n❓ Создание вопроса {i}...")
        q_id = create_question(pack_id, q_data["text"])
        for variant_text, is_correct in q_data["variants"]:
            create_variant(q_id, variant_text, is_correct)
        print(f"✅ Вопрос {i} создан")
    
    print("\n" + "="*50)
    print("🎉 Викторина по программированию создана!")
    print(f"📦 ID пака: {pack_id}")
    print("="*50)


def main():
    """Главная функция"""
    print("="*50)
    print("  Создание тестовых данных для викторины")
    print("="*50)
    
    try:
        # Проверяем доступность API
        print(f"\n🔍 Проверка доступности API: {API_URL}")
        response = requests.get(f"{API_URL}/ping", timeout=5)
        response.raise_for_status()
        print("✅ API доступен")
        
        # Создаем тестовые данные
        create_sample_quiz()
        
        print("\n" + "="*50)
        print("Хотите создать еще одну викторину? (y/n)")
        if input().lower() == 'y':
            create_programming_quiz()
        
        print("\n✨ Готово! Все данные созданы успешно!")
        
    except requests.exceptions.ConnectionError:
        print(f"\n❌ Ошибка: Не удалось подключиться к API по адресу {API_URL}")
        print("   Убедитесь, что сервер запущен")
        sys.exit(1)
    except requests.exceptions.HTTPError as e:
        print(f"\n❌ Ошибка HTTP: {e}")
        print("   Проверьте логи сервера")
        sys.exit(1)
    except Exception as e:
        print(f"\n❌ Неожиданная ошибка: {e}")
        sys.exit(1)


if __name__ == '__main__':
    main()
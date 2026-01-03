#!/usr/bin/env python3
"""
Telegram Bot для игры в викторину через HTTP API
"""

import os
import logging
from typing import Dict, Optional
import requests
from telegram import Update, InlineKeyboardButton, InlineKeyboardMarkup
from telegram.ext import (
    Application,
    CommandHandler,
    CallbackQueryHandler,
    ContextTypes,
    ConversationHandler,
)

# Настройка логирования
logging.basicConfig(
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    level=logging.INFO
)
logger = logging.getLogger(__name__)

# Конфигурация
API_BASE_URL = os.getenv('API_BASE_URL', 'http://localhost:8080')
TELEGRAM_TOKEN = os.getenv('TELEGRAM_BOT_TOKEN')

# Состояния для ConversationHandler
SELECTING_PACK, WAITING_FOR_PLAYERS, PLAYING = range(3)

# Хранилище активных игр (в продакшене использовать Redis или БД)
active_games: Dict[int, dict] = {}


class GameAPI:
    """Класс для работы с HTTP API игры"""
    
    @staticmethod
    def get_all_packs():
        """Получить все доступные паки вопросов"""
        try:
            response = requests.get(f'{API_BASE_URL}/get-all-packs', timeout=5)
            response.raise_for_status()
            return response.json()
        except Exception as e:
            logger.error(f"Error getting packs: {e}")
            return []
    
    @staticmethod
    def create_game_session(pack_id: str):
        """Создать новую игровую сессию"""
        try:
            response = requests.post(
                f'{API_BASE_URL}/game/create-session',
                json={'pack_id': pack_id},
                timeout=5
            )
            response.raise_for_status()
            return response.json()
        except Exception as e:
            logger.error(f"Error creating game session: {e}")
            return None
    
    @staticmethod
    def add_player(game_session_id: str, player_name: str):
        """Добавить игрока в сессию"""
        try:
            response = requests.post(
                f'{API_BASE_URL}/game/add-player',
                json={
                    'game_session_id': game_session_id,
                    'player_name': player_name
                },
                timeout=5
            )
            response.raise_for_status()
            return response.json()
        except Exception as e:
            logger.error(f"Error adding player: {e}")
            return None
    
    @staticmethod
    def start_game(game_session_id: str):
        """Начать игру"""
        try:
            response = requests.post(
                f'{API_BASE_URL}/game/start',
                json={'game_session_id': game_session_id},
                timeout=5
            )
            response.raise_for_status()
            return response.json()
        except Exception as e:
            logger.error(f"Error starting game: {e}")
            return None
    
    @staticmethod
    def get_current_question(game_session_id: str):
        """Получить текущий вопрос"""
        try:
            response = requests.get(
                f'{API_BASE_URL}/game/current-question',
                params={'game_session_id': game_session_id},
                timeout=5
            )
            response.raise_for_status()
            return response.json()
        except Exception as e:
            logger.error(f"Error getting current question: {e}")
            return None
    
    @staticmethod
    def submit_answer(player_id: str, variant_id: str):
        """Отправить ответ игрока"""
        try:
            response = requests.post(
                f'{API_BASE_URL}/game/submit-answer',
                json={
                    'player_id': player_id,
                    'variant_id': variant_id
                },
                timeout=5
            )
            response.raise_for_status()
            return response.json()
        except Exception as e:
            logger.error(f"Error submitting answer: {e}")
            return None
    
    @staticmethod
    def get_game_results(game_session_id: str):
        """Получить результаты игры"""
        try:
            response = requests.get(
                f'{API_BASE_URL}/game/results',
                params={'game_session_id': game_session_id},
                timeout=5
            )
            response.raise_for_status()
            return response.json()
        except Exception as e:
            logger.error(f"Error getting game results: {e}")
            return None


async def start(update: Update, context: ContextTypes.DEFAULT_TYPE) -> int:
    """Начало работы с ботом"""
    user = update.effective_user
    await update.message.reply_text(
        f"Привет, {user.first_name}! 👋\n\n"
        "Я бот для игры в викторину! 🎮\n\n"
        "Используй /newgame чтобы начать новую игру\n"
        "Используй /help для получения справки"
    )
    return ConversationHandler.END


async def help_command(update: Update, context: ContextTypes.DEFAULT_TYPE):
    """Показать справку"""
    help_text = """
🎮 *Команды бота:*

/start - Начать работу с ботом
/newgame - Создать новую игру
/cancel - Отменить текущую игру
/help - Показать эту справку

*Как играть:*
1. Создайте новую игру командой /newgame
2. Выберите пак вопросов
3. Дождитесь других игроков или начните игру
4. Отвечайте на вопросы, выбирая правильные варианты
5. В конце игры увидите результаты!

Удачи! 🍀
    """
    await update.message.reply_text(help_text, parse_mode='Markdown')


async def new_game(update: Update, context: ContextTypes.DEFAULT_TYPE) -> int:
    """Создать новую игру"""
    chat_id = update.effective_chat.id
    
    # Получаем доступные паки
    packs = GameAPI.get_all_packs()
    
    if not packs:
        await update.message.reply_text(
            "❌ Не удалось загрузить список паков. Попробуйте позже."
        )
        return ConversationHandler.END
    
    # Создаем клавиатуру с паками
    keyboard = []
    for pack in packs:
        keyboard.append([
            InlineKeyboardButton(
                pack['title'],
                callback_data=f"pack_{pack['id']}"
            )
        ])
    
    reply_markup = InlineKeyboardMarkup(keyboard)
    
    await update.message.reply_text(
        "🎯 Выберите пак вопросов:",
        reply_markup=reply_markup
    )
    
    return SELECTING_PACK


async def pack_selected(update: Update, context: ContextTypes.DEFAULT_TYPE) -> int:
    """Обработка выбора пака"""
    query = update.callback_query
    await query.answer()
    
    pack_id = query.data.replace('pack_', '')
    chat_id = update.effective_chat.id
    user = update.effective_user
    
    # Создаем игровую сессию
    game_session = GameAPI.create_game_session(pack_id)
    
    if not game_session:
        await query.edit_message_text(
            "❌ Не удалось создать игровую сессию. Попробуйте позже."
        )
        return ConversationHandler.END
    
    # Добавляем создателя как первого игрока
    player = GameAPI.add_player(
        game_session['id'],
        user.first_name or f"Player_{user.id}"
    )
    
    if not player:
        await query.edit_message_text(
            "❌ Не удалось добавить игрока. Попробуйте позже."
        )
        return ConversationHandler.END
    
    # Сохраняем информацию об игре
    active_games[chat_id] = {
        'game_session_id': game_session['id'],
        'players': {user.id: player['id']},
        'current_question': None
    }
    
    keyboard = [
        [InlineKeyboardButton("▶️ Начать игру", callback_data="start_game")],
        [InlineKeyboardButton("❌ Отменить", callback_data="cancel_game")]
    ]
    reply_markup = InlineKeyboardMarkup(keyboard)
    
    await query.edit_message_text(
        f"✅ Игра создана!\n\n"
        f"Игрок: {user.first_name}\n\n"
        f"Другие игроки могут присоединиться, написав /join\n"
        f"Когда все будут готовы, нажмите 'Начать игру'",
        reply_markup=reply_markup
    )
    
    return WAITING_FOR_PLAYERS


async def start_game_callback(update: Update, context: ContextTypes.DEFAULT_TYPE) -> int:
    """Начать игру"""
    query = update.callback_query
    await query.answer()
    
    chat_id = update.effective_chat.id
    
    if chat_id not in active_games:
        await query.edit_message_text("❌ Игра не найдена.")
        return ConversationHandler.END
    
    game_info = active_games[chat_id]
    
    # Запускаем игру
    result = GameAPI.start_game(game_info['game_session_id'])
    
    if not result:
        await query.edit_message_text(
            "❌ Не удалось начать игру. Попробуйте позже."
        )
        return ConversationHandler.END
    
    await query.edit_message_text("🎮 Игра началась! Загружаю первый вопрос...")
    
    # Показываем первый вопрос
    await show_question(update, context)
    
    return PLAYING


async def show_question(update: Update, context: ContextTypes.DEFAULT_TYPE):
    """Показать текущий вопрос"""
    chat_id = update.effective_chat.id
    
    if chat_id not in active_games:
        return
    
    game_info = active_games[chat_id]
    
    # Получаем текущий вопрос
    question_data = GameAPI.get_current_question(game_info['game_session_id'])
    
    if not question_data or 'error' in question_data:
        # Игра закончилась
        await show_results(update, context)
        return
    
    question = question_data['question']
    variants = question_data['variants']
    
    # Сохраняем текущий вопрос
    game_info['current_question'] = question
    
    # Создаем клавиатуру с вариантами ответов
    keyboard = []
    for variant in variants:
        keyboard.append([
            InlineKeyboardButton(
                variant['text'],
                callback_data=f"answer_{variant['id']}"
            )
        ])
    
    reply_markup = InlineKeyboardMarkup(keyboard)
    
    question_text = f"❓ *Вопрос:*\n\n{question['text']}"
    
    if 'image_url' in question and question['image_url']:
        question_text += f"\n\n🖼 Изображение: {question['image_url']}"
    
    if update.callback_query:
        await update.callback_query.message.reply_text(
            question_text,
            reply_markup=reply_markup,
            parse_mode='Markdown'
        )
    else:
        await context.bot.send_message(
            chat_id=chat_id,
            text=question_text,
            reply_markup=reply_markup,
            parse_mode='Markdown'
        )


async def answer_callback(update: Update, context: ContextTypes.DEFAULT_TYPE) -> int:
    """Обработка ответа игрока"""
    query = update.callback_query
    await query.answer()
    
    chat_id = update.effective_chat.id
    user_id = update.effective_user.id
    
    if chat_id not in active_games:
        await query.edit_message_text("❌ Игра не найдена.")
        return ConversationHandler.END
    
    game_info = active_games[chat_id]
    
    if user_id not in game_info['players']:
        await query.answer("❌ Вы не участвуете в этой игре!", show_alert=True)
        return PLAYING
    
    variant_id = query.data.replace('answer_', '')
    player_id = game_info['players'][user_id]
    
    # Отправляем ответ
    result = GameAPI.submit_answer(player_id, variant_id)
    
    if not result:
        await query.answer("❌ Ошибка при отправке ответа!", show_alert=True)
        return PLAYING
    
    # Показываем результат ответа
    if result.get('game_finished'):
        await query.edit_message_text(
            f"{'✅ Правильно!' if result['result'] == 'correct' else '❌ Неправильно!'}\n\n"
            f"🏁 Игра завершена! Показываю результаты..."
        )
        await show_results(update, context)
        return ConversationHandler.END
    else:
        await query.edit_message_text(
            f"{'✅ Правильно!' if result['result'] == 'correct' else '❌ Неправильно!'}\n\n"
            f"Загружаю следующий вопрос..."
        )
        await show_question(update, context)
        return PLAYING


async def show_results(update: Update, context: ContextTypes.DEFAULT_TYPE):
    """Показать результаты игры"""
    chat_id = update.effective_chat.id
    
    if chat_id not in active_games:
        return
    
    game_info = active_games[chat_id]
    
    # Получаем результаты
    results = GameAPI.get_game_results(game_info['game_session_id'])
    
    if not results:
        message = "❌ Не удалось загрузить результаты."
    else:
        players = results.get('players', [])
        # Сортируем по очкам
        players.sort(key=lambda p: p['score'], reverse=True)
        
        message = "🏆 *Результаты игры:*\n\n"
        
        medals = ['🥇', '🥈', '🥉']
        for i, player in enumerate(players):
            medal = medals[i] if i < len(medals) else '👤'
            message += f"{medal} {player['name']}: {player['score']} очков\n"
        
        message += "\n\nСпасибо за игру! 🎉\n"
        message += "Используйте /newgame чтобы сыграть еще раз!"
    
    if update.callback_query:
        await update.callback_query.message.reply_text(
            message,
            parse_mode='Markdown'
        )
    else:
        await context.bot.send_message(
            chat_id=chat_id,
            text=message,
            parse_mode='Markdown'
        )
    
    # Удаляем игру из активных
    if chat_id in active_games:
        del active_games[chat_id]


async def cancel(update: Update, context: ContextTypes.DEFAULT_TYPE) -> int:
    """Отменить текущую игру"""
    chat_id = update.effective_chat.id
    
    if chat_id in active_games:
        del active_games[chat_id]
    
    await update.message.reply_text(
        "❌ Игра отменена.\n\n"
        "Используйте /newgame чтобы начать новую игру."
    )
    
    return ConversationHandler.END


async def cancel_callback(update: Update, context: ContextTypes.DEFAULT_TYPE) -> int:
    """Отменить игру через callback"""
    query = update.callback_query
    await query.answer()
    
    chat_id = update.effective_chat.id
    
    if chat_id in active_games:
        del active_games[chat_id]
    
    await query.edit_message_text(
        "❌ Игра отменена.\n\n"
        "Используйте /newgame чтобы начать новую игру."
    )
    
    return ConversationHandler.END


def main():
    """Запуск бота"""
    if not TELEGRAM_TOKEN:
        logger.error("TELEGRAM_BOT_TOKEN not set!")
        return
    
    # Создаем приложение
    application = Application.builder().token(TELEGRAM_TOKEN).build()
    
    # Создаем ConversationHandler для игры
    conv_handler = ConversationHandler(
        entry_points=[CommandHandler('newgame', new_game)],
        states={
            SELECTING_PACK: [
                CallbackQueryHandler(pack_selected, pattern='^pack_')
            ],
            WAITING_FOR_PLAYERS: [
                CallbackQueryHandler(start_game_callback, pattern='^start_game$'),
                CallbackQueryHandler(cancel_callback, pattern='^cancel_game$')
            ],
            PLAYING: [
                CallbackQueryHandler(answer_callback, pattern='^answer_')
            ],
        },
        fallbacks=[CommandHandler('cancel', cancel)],
    )
    
    # Регистрируем обработчики
    application.add_handler(CommandHandler('start', start))
    application.add_handler(CommandHandler('help', help_command))
    application.add_handler(conv_handler)
    
    # Запускаем бота
    logger.info("Starting bot...")
    application.run_polling(allowed_updates=Update.ALL_TYPES)


if __name__ == '__main__':
    main()
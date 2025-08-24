#!/usr/bin/env python3
"""
Simple Telegram bot for the quiz game.

This bot demonstrates how to interact with the quiz service via gRPC.
"""

import os
import logging
from telegram import Update
from telegram.ext import Application, CommandHandler, MessageHandler, filters, ContextTypes

# Configure logging
logging.basicConfig(
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    level=logging.INFO
)
logger = logging.getLogger(__name__)

# Global variables for game state
game_sessions = {}  # user_id -> game_session_id
current_questions = {}  # user_id -> current_question

async def start(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """Send a message when the command /start is issued."""
    user = update.effective_user
    await update.message.reply_text(
        f"Hi {user.first_name}! Welcome to the Quiz Bot.\n"
        "Use /newgame to start a new quiz game."
    )

async def new_game(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """Start a new game session."""
    user = update.effective_user
    await update.message.reply_text(
        "Starting a new quiz game!\n"
        "Please enter the pack ID you want to use for this game:"
    )
    # In a real implementation, you would:
    # 1. Connect to the gRPC service
    # 2. Create a new game session
    # 3. Store the game session ID for this user
    # 4. Get the first question
    # 5. Send the question to the user

async def handle_message(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """Handle incoming messages."""
    user = update.effective_user
    text = update.message.text
    
    # In a real implementation, you would:
    # 1. Check if the user is in a game session
    # 2. If waiting for pack ID, create game session with that pack
    # 3. If waiting for answer, submit the answer
    # 4. Get the next question or end the game
    
    await update.message.reply_text(
        f"You said: {text}\n"
        "This is a demo bot. In a real implementation, this would interact with the quiz service."
    )

async def help_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """Send a message when the command /help is issued."""
    await update.message.reply_text(
        "Available commands:\n"
        "/start - Start the bot\n"
        "/newgame - Start a new quiz game\n"
        "/help - Show this help message"
    )

def main() -> None:
    """Start the bot."""
    # Get the bot token from environment variable
    token = os.environ.get('TELEGRAM_BOT_TOKEN')
    if not token:
        logger.error("TELEGRAM_BOT_TOKEN environment variable not set")
        return
    
    # Create the Application and pass it your bot's token
    application = Application.builder().token(token).build()

    # Register handlers
    application.add_handler(CommandHandler("start", start))
    application.add_handler(CommandHandler("newgame", new_game))
    application.add_handler(CommandHandler("help", help_command))
    application.add_handler(MessageHandler(filters.TEXT & ~filters.COMMAND, handle_message))

    # Run the bot until the user presses Ctrl-C
    application.run_polling(allowed_updates=Update.ALL_TYPES)

if __name__ == "__main__":
    main()
# Development Guide

This guide explains how to set up and run the quiz backend service for development and testing.

## Prerequisites

- Docker and Docker Compose
- Basic understanding of gRPC and C++

## Development Environment Setup

### Running the Backend Service

To run the backend service in development mode:

```bash
docker-compose -f docker-compose.dev.yml up
```

This will start:
1. PostgreSQL database with the required schema
2. The quiz backend service (automatically builds and runs)
3. A template for the Telegram bot service

The services will be available at:
- Backend HTTP API: http://localhost:8080
- Backend gRPC API: localhost:8081
- Database: localhost:15433

### Running for Production

To run the service in production mode:

```bash
docker-compose -f docker-compose.run.yml up
```

## Service Endpoints

### gRPC Methods

The service provides the following gRPC methods:

#### Pack Management
- `CreatePack` - Create a new quiz pack
- `GetPackById` - Get a pack by ID
- `GetAllPacks` - Get all packs

#### Question Management
- `CreateQuestion` - Create a new question in a pack
- `GetQuestionById` - Get a question by ID
- `GetQuestionsByPackId` - Get all questions in a pack

#### Variant Management
- `CreateVariant` - Create a new variant for a question
- `GetVariantById` - Get a variant by ID
- `GetVariantsByQuestionId` - Get all variants for a question

#### Game Session Management
- `CreateGameSession` - Create a new game session
- `GetGameSession` - Get a game session by ID
- `StartGameSession` - Start a game session
- `EndGameSession` - End a game session
- `GetGameSessions` - Get all game sessions
- `GetCurrentQuestion` - Get the current question in a game session
- `AdvanceToNextQuestion` - Advance to the next question in a game session

#### Player Management
- `AddPlayer` - Add a player to a game session
- `GetPlayers` - Get all players in a game session

#### Player Answer Management
- `SubmitAnswer` - Submit a player's answer
- `GetPlayerAnswers` - Get all answers for a player

#### Additional Methods
- `GetQuestionsWithVariants` - Get all questions with their variants for a pack

## Development Workflow

1. Make changes to the C++ code
2. The service will automatically rebuild and restart
3. Test the changes using gRPC clients or the Telegram bot

## Database Schema

The database schema is defined in `postgresql/schemas/db_1.sql` and includes:
- `quiz.packs` - Quiz packs
- `quiz.questions` - Questions in packs
- `quiz.variants` - Answer variants for questions
- `quiz.game_sessions` - Game sessions
- `quiz.players` - Players in game sessions
- `quiz.player_answers` - Player answers

## Testing

To run the tests:

```bash
make test-debug
```

## Building

To manually build the service:

```bash
make build-debug
```

Or for release build:

```bash
make build-release
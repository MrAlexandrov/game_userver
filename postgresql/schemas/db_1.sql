DROP SCHEMA IF EXISTS hello_schema CASCADE;

CREATE SCHEMA IF NOT EXISTS hello_schema;

CREATE TABLE IF NOT EXISTS hello_schema.users (
    name TEXT PRIMARY KEY,
    count INTEGER DEFAULT(1)
);

CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- Создаём схему quiz
CREATE SCHEMA IF NOT EXISTS quiz;

-- Таблица packs: id UUID, генерируется автоматически
CREATE TABLE IF NOT EXISTS quiz.packs (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    title TEXT NOT NULL
);

-- Таблица questions
CREATE TABLE IF NOT EXISTS quiz.questions (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    pack_id UUID NOT NULL REFERENCES quiz.packs(id) ON DELETE CASCADE,
    text TEXT NOT NULL,
    image_url TEXT
);

-- Таблица variants
CREATE TABLE IF NOT EXISTS quiz.variants (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    question_id UUID NOT NULL REFERENCES quiz.questions(id) ON DELETE CASCADE,
    text TEXT NOT NULL,
    is_correct BOOLEAN NOT NULL DEFAULT FALSE
);

-- Индексы для ускорения JOIN'ов и фильтрации
CREATE INDEX idx_questions_pack_id ON quiz.questions(pack_id);
CREATE INDEX idx_variants_question_id ON quiz.variants(question_id);

-- Таблица game_sessions
CREATE TABLE IF NOT EXISTS quiz.game_sessions (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    pack_id UUID NOT NULL REFERENCES quiz.packs(id) ON DELETE CASCADE,
    state TEXT NOT NULL DEFAULT 'waiting',  -- waiting, active, finished
    current_question_index INTEGER NOT NULL DEFAULT 0,
    created_at TIMESTAMP NOT NULL DEFAULT NOW(),
    started_at TIMESTAMP,
    finished_at TIMESTAMP
);

-- Таблица players
CREATE TABLE IF NOT EXISTS quiz.players (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    game_session_id UUID NOT NULL REFERENCES quiz.game_sessions(id) ON DELETE CASCADE,
    name TEXT NOT NULL,
    score INTEGER NOT NULL DEFAULT 0,
    joined_at TIMESTAMP NOT NULL DEFAULT NOW()
);

-- Таблица player_answers
CREATE TABLE IF NOT EXISTS quiz.player_answers (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    player_id UUID NOT NULL REFERENCES quiz.players(id) ON DELETE CASCADE,
    question_id UUID NOT NULL REFERENCES quiz.questions(id) ON DELETE CASCADE,
    variant_id UUID NOT NULL REFERENCES quiz.variants(id) ON DELETE CASCADE,
    is_correct BOOLEAN NOT NULL,
    answered_at TIMESTAMP NOT NULL DEFAULT NOW()
);

-- Индексы для game tables
CREATE INDEX idx_game_sessions_pack_id ON quiz.game_sessions(pack_id);
CREATE INDEX idx_players_game_session_id ON quiz.players(game_session_id);
CREATE INDEX idx_player_answers_player_id ON quiz.player_answers(player_id);
CREATE INDEX idx_player_answers_question_id ON quiz.player_answers(question_id);

CREATE TYPE quiz.pack AS (
    id UUID,
    title TEXT
);

CREATE TYPE quiz.question AS (
    id UUID,
    pack_id UUID,
    text TEXT,
    image_url TEXT
);

CREATE TYPE quiz.variant AS (
    id UUID,
    question_id UUID,
    text TEXT,
    is_correct BOOLEAN
);

CREATE TYPE quiz.game_session AS (
    id UUID,
    pack_id UUID,
    state TEXT,
    current_question_index INTEGER,
    created_at TIMESTAMP,
    started_at TIMESTAMP,
    finished_at TIMESTAMP
);

CREATE TYPE quiz.player AS (
    id UUID,
    game_session_id UUID,
    name TEXT,
    score INTEGER,
    joined_at TIMESTAMP
);

CREATE TYPE quiz.player_answer AS (
    id UUID,
    player_id UUID,
    question_id UUID,
    variant_id UUID,
    is_correct BOOLEAN,
    answered_at TIMESTAMP
);

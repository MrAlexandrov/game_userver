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

---

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

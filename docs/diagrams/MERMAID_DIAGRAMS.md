# Mermaid диаграммы для игровой системы

Коллекция Mermaid диаграмм для визуализации архитектуры и процессов.

**Как использовать**: Скопируйте код диаграммы и вставьте в:
- [Mermaid Live Editor](https://mermaid.live/)
- GitHub/GitLab (поддерживают Mermaid нативно)
- VS Code с расширением Mermaid

---

## 1. Архитектура системы (Layered Architecture)

```mermaid
graph TB
    subgraph "Presentation Layer"
        H1[HTTP Handlers]
        H2[gRPC Handlers]
    end
    
    subgraph "Business Logic Layer"
        GS[GameService<br/>Singleton]
        OBS[Observer Manager]
        VAL[Validator Factory]
    end
    
    subgraph "Data Access Layer"
        ST1[Game Sessions Storage]
        ST2[Players Storage]
        ST3[Content Storage]
    end
    
    subgraph "Database"
        PG[(PostgreSQL)]
    end
    
    H1 --> GS
    H2 --> GS
    GS --> OBS
    GS --> VAL
    GS --> ST1
    GS --> ST2
    GS --> ST3
    ST1 --> PG
    ST2 --> PG
    ST3 --> PG
    
    style GS fill:#90EE90
    style OBS fill:#FFB6C1
    style PG fill:#87CEEB
```

---

## 2. Компонентная архитектура (userver Components)

```mermaid
graph LR
    subgraph "userver Components"
        MC[MinimalServerComponentList]
        PGC[PostgreSQL Component]
        GSC[GameService Component<br/>SINGLETON]
        HC[HTTP Client]
        GRPC[gRPC Server]
    end
    
    subgraph "Handlers"
        H1[CreateGameSession]
        H2[AddPlayer]
        H3[SubmitAnswer]
    end
    
    MC --> PGC
    MC --> GSC
    MC --> HC
    MC --> GRPC
    
    GSC --> |DI| H1
    GSC --> |DI| H2
    GSC --> |DI| H3
    
    PGC --> |Connection Pool| GSC
    
    style GSC fill:#FFD700
    style PGC fill:#87CEEB
```

---

## 3. Состояния игровой сессии (State Machine)

```mermaid
stateDiagram-v2
    [*] --> waiting: POST /games
    
    waiting --> active: POST /games/{id}/start<br/>(min 1 player)
    waiting --> waiting: POST /games/{id}/players<br/>(add player)
    
    active --> active: POST /games/{id}/answers<br/>(not all answered)
    active --> active: All answered<br/>(not last question)
    active --> finished: All answered<br/>(last question)
    
    finished --> [*]
    
    note right of waiting
        Can add players
        Cannot submit answers
    end note
    
    note right of active
        Cannot add players
        Can submit answers
        Auto-advance questions
    end note
    
    note right of finished
        Cannot modify
        Can view results
    end note
```

---

## 4. Игровой процесс (Game Flow Sequence)

```mermaid
sequenceDiagram
    participant A as Admin
    participant P1 as Player 1
    participant P2 as Player 2
    participant H as Handler
    participant GS as GameService
    participant DB as PostgreSQL
    participant O as Observers
    
    A->>H: POST /games
    H->>GS: CreateGameSession()
    GS->>DB: INSERT game_session
    DB-->>GS: game_id
    GS->>O: GameSessionCreatedEvent
    GS-->>H: game_session
    H-->>A: {game_id}
    
    P1->>H: POST /games/{id}/players
    H->>GS: AddPlayer()
    GS->>DB: INSERT player
    GS->>O: PlayerAddedEvent
    GS-->>H: player
    H-->>P1: {player_id}
    
    P2->>H: POST /games/{id}/players
    H->>GS: AddPlayer()
    GS->>DB: INSERT player
    GS->>O: PlayerAddedEvent
    GS-->>H: player
    H-->>P2: {player_id}
    
    A->>H: POST /games/{id}/start
    H->>GS: StartGame()
    GS->>DB: UPDATE state='active'
    GS->>O: GameStartedEvent
    O-->>P1: Push notification
    O-->>P2: Push notification
    
    P1->>H: POST /games/{id}/answers
    H->>GS: SubmitAnswer()
    GS->>DB: INSERT player_answer
    GS->>DB: UPDATE score
    GS->>O: AnswerSubmittedEvent
    
    P2->>H: POST /games/{id}/answers
    H->>GS: SubmitAnswer()
    GS->>DB: INSERT player_answer
    GS->>GS: CheckAllAnswered()
    GS->>DB: UPDATE current_question_index
    GS->>O: QuestionAdvancedEvent
    O-->>P1: Next question
    O-->>P2: Next question
```

---

## 5. Observer Pattern

```mermaid
classDiagram
    class IGameObserver {
        <<interface>>
        +OnEvent(GameEvent) void
        +ShouldHandleEvent(GameEventType) bool
    }
    
    class GameObserverManager {
        -observers: vector~shared_ptr~
        +AddObserver(observer)
        +RemoveObserver(observer)
        +NotifyObservers(event)
        +GetObserverCount() size_t
    }
    
    class LoggingObserver {
        +OnEvent(event)
        +ShouldHandleEvent(type) bool
    }
    
    class NotificationObserver {
        +OnEvent(event)
        +ShouldHandleEvent(type) bool
    }
    
    class StatisticsObserver {
        +OnEvent(event)
        +ShouldHandleEvent(type) bool
    }
    
    class GameService {
        -observer_manager: GameObserverManager
        +AddObserver(observer)
        +NotifyObservers(event)
    }
    
    IGameObserver <|.. LoggingObserver
    IGameObserver <|.. NotificationObserver
    IGameObserver <|.. StatisticsObserver
    GameObserverManager o-- IGameObserver
    GameService *-- GameObserverManager
```

---

## 6. Поток данных при отправке ответа (Data Flow)

```mermaid
flowchart TD
    Start([Player submits answer]) --> Parse[Parse request<br/>player_id, variant_id]
    Parse --> GetPlayer[Get Player from DB]
    GetPlayer --> GetSession[Get GameSession from DB]
    GetSession --> GetQuestion[Get Current Question]
    GetQuestion --> Validate[Validate Answer<br/>using ValidatorFactory]
    
    Validate --> CheckCorrect{Is Correct?}
    CheckCorrect -->|Yes| UpdateScore[Update Player Score]
    CheckCorrect -->|No| SaveAnswer[Save Answer]
    UpdateScore --> SaveAnswer
    
    SaveAnswer --> Notify[Notify Observers]
    Notify --> CheckAll{All Players<br/>Answered?}
    
    CheckAll -->|No| Return1[Return result]
    CheckAll -->|Yes| CheckLast{Last<br/>Question?}
    
    CheckLast -->|No| Advance[Advance to Next Question]
    CheckLast -->|Yes| Finish[Finish Game]
    
    Advance --> NotifyAdvance[Notify: QuestionAdvanced]
    Finish --> NotifyFinish[Notify: GameFinished]
    
    NotifyAdvance --> Return2[Return result]
    NotifyFinish --> Return2
    
    Return1 --> End([Response to client])
    Return2 --> End
    
    style CheckCorrect fill:#FFE4B5
    style CheckAll fill:#FFE4B5
    style CheckLast fill:#FFE4B5
    style Notify fill:#90EE90
    style NotifyAdvance fill:#90EE90
    style NotifyFinish fill:#90EE90
```

---

## 7. Database Schema (ER Diagram)

```mermaid
erDiagram
    PACKS ||--o{ QUESTIONS : contains
    QUESTIONS ||--o{ VARIANTS : has
    QUESTIONS ||--o{ TEXT_ANSWERS : has
    PACKS ||--o{ GAME_SESSIONS : uses
    GAME_SESSIONS ||--o{ PLAYERS : has
    PLAYERS ||--o{ PLAYER_ANSWERS : submits
    QUESTIONS ||--o{ PLAYER_ANSWERS : answered
    VARIANTS ||--o{ PLAYER_ANSWERS : selected
    
    PACKS {
        uuid id PK
        text title
    }
    
    QUESTIONS {
        uuid id PK
        uuid pack_id FK
        text text
        text image_url
        text question_type
    }
    
    VARIANTS {
        uuid id PK
        uuid question_id FK
        text text
        boolean is_correct
    }
    
    TEXT_ANSWERS {
        uuid id PK
        uuid question_id FK
        text text
        timestamp created_at
    }
    
    GAME_SESSIONS {
        uuid id PK
        uuid pack_id FK
        text state
        integer current_question_index
        timestamp created_at
        timestamp started_at
        timestamp finished_at
    }
    
    PLAYERS {
        uuid id PK
        uuid game_session_id FK
        text name
        integer score
        timestamp joined_at
    }
    
    PLAYER_ANSWERS {
        uuid id PK
        uuid player_id FK
        uuid question_id FK
        uuid variant_id FK
        text text_answer
        boolean is_correct
        timestamp answered_at
    }
```

---

## 8. Deployment Architecture

```mermaid
graph TB
    subgraph "Docker Compose"
        subgraph "App Container"
            APP[game_userver<br/>C++ Application<br/>Port 8080, 8081]
        end
        
        subgraph "Database Container"
            PG[(PostgreSQL 14<br/>Port 5432)]
        end
        
        subgraph "Volumes"
            V1["/postgresql/data"]
            V2["/configs"]
        end
    end
    
    subgraph "External"
        CLIENT[Clients<br/>HTTP/gRPC]
        ADMIN[Admin<br/>Management]
    end
    
    CLIENT -->|HTTP :8080| APP
    CLIENT -->|gRPC :8081| APP
    ADMIN -->|HTTP :8080| APP
    
    APP -->|SQL| PG
    PG -.->|Persist| V1
    APP -.->|Read| V2
    
    style APP fill:#90EE90
    style PG fill:#87CEEB
    style V1 fill:#FFE4B5
    style V2 fill:#FFE4B5
```

---

## 9. Request Processing Pipeline

```mermaid
flowchart LR
    subgraph "Client"
        C[HTTP Client]
    end
    
    subgraph "userver Framework"
        L[Listener<br/>:8080]
        TP[Task Processor<br/>Worker Threads]
        R[Router]
    end
    
    subgraph "Application"
        H[Handler]
        GS[GameService]
        S[Storage]
    end
    
    subgraph "Database"
        CP[Connection Pool]
        DB[(PostgreSQL)]
    end
    
    C -->|HTTP Request| L
    L --> TP
    TP --> R
    R -->|Route| H
    H -->|Business Logic| GS
    GS -->|Data Access| S
    S -->|Get Connection| CP
    CP -->|SQL Query| DB
    DB -->|Result| CP
    CP -->|Return| S
    S -->|Data| GS
    GS -->|Response| H
    H -->|JSON| R
    R --> TP
    TP --> L
    L -->|HTTP Response| C
    
    style TP fill:#FFE4B5
    style GS fill:#90EE90
    style CP fill:#87CEEB
```

---

## 10. Validator Factory Pattern

```mermaid
classDiagram
    class IAnswerValidator {
        <<interface>>
        +Validate(question_id, answer) ValidationResult
    }
    
    class ValidatorFactory {
        -pg_cluster: ClusterPtr
        +CreateValidator(QuestionType) unique_ptr~IAnswerValidator~
    }
    
    class MultipleChoiceValidator {
        -pg_cluster: ClusterPtr
        +Validate(question_id, answer) ValidationResult
    }
    
    class FreeTextValidator {
        -pg_cluster: ClusterPtr
        +Validate(question_id, answer) ValidationResult
    }
    
    class CustomValidator {
        -pg_cluster: ClusterPtr
        +Validate(question_id, answer) ValidationResult
    }
    
    class ValidationResult {
        +is_correct: bool
        +message: string
    }
    
    IAnswerValidator <|.. MultipleChoiceValidator
    IAnswerValidator <|.. FreeTextValidator
    IAnswerValidator <|.. CustomValidator
    ValidatorFactory ..> IAnswerValidator : creates
    IAnswerValidator ..> ValidationResult : returns
```

---

## 11. Concurrent Game Sessions

```mermaid
graph TB
    subgraph "GameService Singleton"
        GS[GameService Instance]
        OM[Observer Manager]
    end
    
    subgraph "Game Session 1"
        GS1[Session: uuid-1<br/>State: active<br/>Players: 3<br/>Question: 2/10]
    end
    
    subgraph "Game Session 2"
        GS2[Session: uuid-2<br/>State: waiting<br/>Players: 5<br/>Question: 0/15]
    end
    
    subgraph "Game Session 3"
        GS3[Session: uuid-3<br/>State: finished<br/>Players: 2<br/>Question: 5/5]
    end
    
    subgraph "PostgreSQL"
        DB[(Database<br/>Connection Pool)]
    end
    
    GS --> GS1
    GS --> GS2
    GS --> GS3
    
    GS1 -.->|Isolated Data| DB
    GS2 -.->|Isolated Data| DB
    GS3 -.->|Isolated Data| DB
    
    OM -.->|Events| GS1
    OM -.->|Events| GS2
    OM -.->|Events| GS3
    
    style GS fill:#FFD700
    style OM fill:#FFB6C1
    style DB fill:#87CEEB
```

---

## 12. Error Handling Flow

```mermaid
flowchart TD
    Start([Request]) --> Try{Try}
    
    Try -->|Success| Process[Process Request]
    Try -->|Exception| Catch[Catch Exception]
    
    Process --> Return[Return 200 OK]
    
    Catch --> CheckType{Exception Type}
    
    CheckType -->|ValidationError| Log1[LOG_WARNING]
    CheckType -->|DatabaseError| Log2[LOG_ERROR]
    CheckType -->|UnknownError| Log3[LOG_ERROR]
    
    Log1 --> Return400[Return 400<br/>Bad Request]
    Log2 --> Return500[Return 500<br/>Internal Error]
    Log3 --> Return500
    
    Return --> End([Response])
    Return400 --> End
    Return500 --> End
    
    style Catch fill:#FFB6C1
    style Log1 fill:#FFE4B5
    style Log2 fill:#FF6B6B
    style Log3 fill:#FF6B6B
```

---

## Как использовать эти диаграммы

### 1. В GitHub/GitLab
Просто вставьте код в markdown файл:
````markdown
```mermaid
graph TD
    A[Start] --> B[End]
```
````

### 2. В Mermaid Live Editor
1. Откройте https://mermaid.live/
2. Вставьте код диаграммы
3. Экспортируйте как PNG/SVG

### 3. В VS Code
1. Установите расширение "Markdown Preview Mermaid Support"
2. Откройте preview (Ctrl+Shift+V)
3. Диаграммы отобразятся автоматически

### 4. Генерация изображений через CLI
```bash
# Установка mermaid-cli
npm install -g @mermaid-js/mermaid-cli

# Генерация PNG
mmdc -i diagram.mmd -o diagram.png

# Генерация SVG
mmdc -i diagram.mmd -o diagram.svg
```

---

## Дополнительные диаграммы

Если нужны дополнительные диаграммы для:
- Конкретных use cases
- Детальных sequence diagrams
- Специфичных компонентов

Просто запросите, и я добавлю их в этот файл!
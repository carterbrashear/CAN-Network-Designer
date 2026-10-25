# TODO List
-[ ] Fix Dear ImGui to a commit to prevent breaking changes

## High Level System Diagram
```mermaid
flowchart TB
    subgraph PresentationLayer["Presentation Layer (GLFW / ImGui)"]
        Window["Window (GLFW Handle)"]
        UI["UserInterface Runtime"]
        Canvas["Canvas2D (Schematic Viewport)"]
        subgraph UIPanels["UI Panels"]
            MsgPanel["MessageTablePanel"]
            SigPanel["SignalInspectorPanel (BitGrid)"]
            DevPanel["DeviceInspectorPanel"]
            BusPanel["BusConfigPanel"]
        end
    end

    subgraph ApplicationLayer["Application & CAD Engine"]
        App["Application Controller"]
        CmdStack["CommandStack"]
        subgraph Commands["ICommand Implementations"]
            MoveCmd["MoveDeviceCommand"]
            ConnCmd["ConnectDeviceBusCommand"]
            AddMsgCmd["AddMessageCommand"]
            ModSigCmd["ModifySignalCommand"]
        end
    end

    subgraph DomainLayer["In-Memory CAN Domain Model"]
        Project["NetworkProject"]
        Device["Device (ECU)"]
        Bus["Bus (CAN / CAN-FD)"]
        Message["Message"]
        Signal["Signal"]
        SigVal["SignalValue (Enums)"]
    end

    subgraph ServicesLayer["Analysis & I/O Services"]
        Validator["NetworkValidator"]
        Report["ValidationReport"]
        DBC["DbcParser"]
    end

    subgraph PersistenceLayer["Storage & Database Layer"]
        DBMgr["DatabaseManager"]
        DB["Database (sqlite3 C-API)"]
        Disk[(SQLite DB File)]
        DbcFile[(".dbc Export/Import File")]
    end

%% Presentation to Application
    Window --> App
    UI --> App
    Canvas --> CmdStack
    UIPanels --> CmdStack

%% Application to Domain
    App --> Project
    CmdStack --> Commands
    Commands --> Project

%% Domain Relationships (Using standard flowchart syntax)
    Project -->|contains| Device
    Project -->|contains| Bus
    Project -->|contains| Message
    Message -->|contains| Signal
    Signal -->|defines| SigVal

%% Services interactions
    Validator --> Project
    Validator --> Report
    DBC <--> Project
    DBC <--> DbcFile

%% Persistence interactions
    DBMgr --> DB
    DB --> Disk
    DBMgr <--> Project
    App --> DBMgr
```
## Application Runtime Loop Sequence Diagram
```mermaid
sequenceDiagram
    autonumber
    participant GLFW as GLFW / OS Window
    participant App as Application
    participant UI as UserInterface
    participant Canvas as Canvas2D
    participant Panels as Inspector & Config Panels
    participant Stack as CommandStack
    participant Project as NetworkProject

    loop Main Render Loop (while !shouldClose)
        App->>GLFW: pollEvents()
        App->>App: OnUpdate()
        
        App->>UI: begin()
        Note over UI: Prepare ImGui Frame Context
        
        App->>Canvas: render(project, command_stack)
        alt User interacted with topology (Drag node, Wire bus)
            Canvas->>Stack: execute(MoveDeviceCommand / ConnectBusCommand)
            Stack->>Project: execute() [Mutate coordinates / links]
        end
        
        App->>Panels: render(project, command_stack)
        alt Value modified in BitGrid or Property Inspector
            Panels->>Stack: execute(ModifySignalCommand)
            Stack->>Project: execute() [Apply bit shift / payload change]
        end
        
        App->>UI: end()
        Note over UI: Finalize ImGui vertex buffers & draw lists
        
        App->>GLFW: swapBuffers()
    end
```
## Undo / Redo Command Execution Cycle Sequence Diagram
```mermaid
sequenceDiagram
    autonumber
    actor Engineer as Automotive Engineer
    participant UI as SignalInspectorPanel
    participant Stack as CommandStack
    participant Cmd as ModifySignalCommand
    participant Project as NetworkProject
    participant DB as DatabaseManager

    Engineer->>UI: Change Signal start_bit (24 -> 32)
    UI->>Cmd: create(project, signal_id, old_state, new_state)
    UI->>Stack: execute(cmd)
    
    activate Stack
    Stack->>Cmd: execute()
    Cmd->>Project: Mutate signal struct in memory
    Stack->>Stack: push to undo_stack
    Stack->>Stack: clear redo_stack()
    deactivate Stack
    
    Note over Engineer, UI: Engineer presses CTRL + Z (Undo)
    Engineer->>UI: Trigger Undo Action
    UI->>Stack: undo()
    
    activate Stack
    Stack->>Cmd: undo()
    Cmd->>Project: Revert signal struct (start_bit = 24)
    Stack->>Stack: pop from undo_stack -> push to redo_stack
    deactivate Stack

    Note over Engineer, DB: Engineer saves file
    Engineer->>UI: Save Project
    UI->>DB: saveProject("project.db", project)
    DB->>DB: SQLite Transaction Write
```
## DBC Input and Validation Pipeline
```mermaid
sequenceDiagram
    autonumber
    actor Engineer as Engineer
    participant UI as UserInterface
    participant DBC as DbcParser
    participant Validator as NetworkValidator
    participant Project as NetworkProject
    participant DBMgr as DatabaseManager
    participant SQLite as SQLite Database

    Engineer->>UI: Select "Import Vector DBC File"
    UI->>DBC: importDbc("chassis_bus.dbc", project)
    
    activate DBC
    DBC->>DBC: Tokenize (BU_, BO_, SG_, VAL_, SIG_VALTYPE_)
    DBC->>Project: addBus("CAN_Chassis", 500k, 2M, is_fd)
    DBC->>Project: addDevice("BrakeController")
    DBC->>Project: addMessage(CAN_ID 0x120, DLC 8)
    DBC->>Project: addSignal("VehicleSpeed", start=16, len=16)
    deactivate DBC

    UI->>Validator: validateNetwork(project)
    activate Validator
    Validator->>Validator: checkIdCollisions()
    Validator->>Validator: checkSignalOverlaps()
    Validator->>Validator: calculateBusLoad()
    Validator-->>UI: return ValidationReport (Valid, Warnings: 0)
    deactivate Validator

    UI->>DBMgr: saveProject("chassis.candb", project)
    activate DBMgr
    DBMgr->>SQLite: BEGIN TRANSACTION
    DBMgr->>SQLite: INSERT INTO buses, devices, messages, signals...
    DBMgr->>SQLite: COMMIT TRANSACTION
    DBMgr-->>UI: Success
    deactivate DBMgr
    
    UI-->>Engineer: Network Loaded & Visualized
```

## Project Workspace Lifecycle Diagram
```mermaid
stateDiagram-v2
    [*] --> Idle : Launch Application

    state Idle {
        [*] --> EmptyWorkspace
        EmptyWorkspace --> ProjectLoaded : Load .candb / Import DBC
        EmptyWorkspace --> NewProjectCreated : New Project Dialog
    }

    state ProjectEditing {
        [*] --> Clean
        Clean --> Dirty : Mutate Topology / Modify Signal (via CommandStack)
        Dirty --> Dirty : Repeated Edits
        Dirty --> Clean : Undo all edits back to root
        
        state ValidationContext {
            [*] --> Validating : Command Executed
            Validating --> Valid : ID & Bits Compliant
            Validating --> Invalid : Collision / Bit Overlap
        }
    }

    Idle --> ProjectEditing : Open / Create
    ProjectEditing --> SavingState : Save / Save As
    
    state SavingState {
        [*] --> BeginTransaction
        BeginTransaction --> WritingTables : SQLite Sync
        WritingTables --> CommitTransaction : Integrity Verified
        CommitTransaction --> [*]
    }

    SavingState --> Clean : Persisted successfully
    Clean --> [*] : Exit Application
```

## [UML Class Diagram](UML.md)
## [ERD Database Diagram](Database.md)
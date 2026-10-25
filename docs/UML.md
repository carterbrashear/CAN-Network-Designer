# CAN Network Designer - UML Architecture

---

## 1. System Architecture & UI Subsystem

This diagram models the application runtime, windowing, UI panel composition, and application lifecycle.

```mermaid
classDiagram
    %% Core Runtime
    class Application {
        -Window main_window
        -UserInterface ui
        -DatabaseManager db_manager
        -NetworkProject active_project
        -CommandStack command_stack
        +Run() void
        -OnUpdate() void
        -OnRenderScenes() void
        -OnRenderUI() void
    }

    class Window {
        -static const char* window_name
        -static bool vsync
        -int width
        -int height
        -GLFWwindow* window
        +init() void
        +pollEvents() void
        +swapBuffers() void
        +shouldClose() bool
        +getWindow() GLFWwindow*
        +getWidth() int
        +getHeight() int
    }

    class UserInterface {
        -static bool dark_mode
        -Window& window
        -Canvas2D canvas
        -MessageTablePanel message_panel
        -SignalInspectorPanel signal_panel
        -DeviceInspectorPanel device_panel
        -BusConfigPanel bus_panel
        +init() void
        +begin() void
        +end() void
        +render(NetworkProject& project, CommandStack& commands) void
    }

    class Canvas2D {
        -float zoom_level
        -float pan_x
        -float pan_y
        -int64_t selected_device_id
        -int64_t selected_bus_id
        +render(NetworkProject& project, CommandStack& commands) void
        +screenToWorld(float sx, float sy) pair~float, float~
        +worldToScreen(float wx, float wy) pair~float, float~
    }

    class MessageTablePanel {
        -int64_t selected_message_id
        -string filter_query
        +render(NetworkProject& project, CommandStack& commands) void
    }

    class SignalInspectorPanel {
        -int64_t active_message_id
        -int64_t selected_signal_id
        +render(NetworkProject& project, CommandStack& commands) void
        -renderBitGrid(Message& message) void
        -renderSignalDetails(Signal& signal) void
    }

    class DeviceInspectorPanel {
        -int64_t selected_device_id
        +render(NetworkProject& project, CommandStack& commands) void
    }

    class BusConfigPanel {
        -int64_t selected_bus_id
        +render(NetworkProject& project, CommandStack& commands) void
    }

    %% Relationships
    Application *-- Window
    Application *-- UserInterface
    Application *-- DatabaseManager
    Application *-- NetworkProject
    Application *-- CommandStack

    UserInterface *-- Canvas2D
    UserInterface *-- MessageTablePanel
    UserInterface *-- SignalInspectorPanel
    UserInterface *-- DeviceInspectorPanel
    UserInterface *-- BusConfigPanel
```

---

## 2. CAN Domain Model (Schema Abstraction)

This diagram represents the domain entities mapped directly to [`Schema.sql`](file:///home/rocket/CLionProjects/CAN-Network-Designer/src/Database/Schema.sql).
- A **Bus** carries traffic at defined nominal and data bit rates.
- A **Device** (ECU) connects to one or more buses via `BusConnection`.
- A **Message** belongs to a **Bus** and is transmitted by an optional **Device**.
- A **Signal** belongs to a **Message**, supports bit positioning, multiplexing (`MuxType`), scaling, and can be received by multiple **Devices**.
- A **SignalValue** defines human-readable descriptions for discrete signal states (enums).

```mermaid
classDiagram
    %% Domain Entities
    class NetworkProject {
        -string file_path
        -vector~Device~ devices
        -vector~Bus~ buses
        -vector~Message~ messages
        +addDevice(Device device) int64_t
        +addBus(Bus bus) int64_t
        +connectDeviceToBus(int64_t device_id, int64_t bus_id) bool
        +disconnectDeviceFromBus(int64_t device_id, int64_t bus_id) bool
        +addMessage(Message message) int64_t
        +findDevice(int64_t device_id) Device*
        +findBus(int64_t bus_id) Bus*
        +findMessage(int64_t message_id) Message*
        +clear() void
    }

    class Device {
        +int64_t id
        +string name
        +float pos_x
        +float pos_y
        +string comment
        +vector~int64_t~ connected_bus_ids
    }

    class Bus {
        +int64_t id
        +string name
        +int32_t nominal_bit_rate
        +int32_t data_bit_rate
        +bool is_fd
        +uint32_t color
        +string comment
        +calculateBusLoad(const vector~Message~& messages) double
    }

    class BusConnection {
        +int64_t device_id
        +int64_t bus_id
    }

    class Message {
        +int64_t id
        +int64_t bus_id
        +optional~int64_t~ transmitter_id
        +uint32_t can_id
        +bool is_extended
        +bool is_fd
        +uint8_t dlc
        +string name
        +uint32_t cycle_time_ms
        +string comment
        +vector~Signal~ signals
        +getPayloadBytes() uint8_t
        +validateSignalFit(const Signal& new_sig) bool
        +validateNoCollision(const Signal& new_sig) bool
    }

    class Signal {
        +int64_t id
        +int64_t message_id
        +string name
        +uint16_t start_bit
        +uint16_t bit_length
        +ByteOrder byte_order
        +bool is_signed
        +double factor
        +double offset
        +double min_value
        +double max_value
        +string unit
        +MuxType mux_type
        +optional~uint32_t~ mux_value
        +string comment
        +vector~SignalValue~ value_descriptions
        +vector~int64_t~ receiver_device_ids
        +rawToPhysical(uint64_t raw) double
        +physicalToRaw(double physical) uint64_t
    }

    class SignalValue {
        +int64_t id
        +int64_t signal_id
        +int64_t raw_value
        +string description
    }

    class ByteOrder {
        <<enumeration>>
        INTEL_LITTLE_ENDIAN = 0
        MOTOROLA_BIG_ENDIAN = 1
    }

    class MuxType {
        <<enumeration>>
        STANDARD = 0
        MULTIPLEXER_SWITCH = 1
        MULTIPLEXED = 2
    }

    %% Relationships
    NetworkProject "1" *-- "*" Device : contains
    NetworkProject "1" *-- "*" Bus : contains
    NetworkProject "1" *-- "*" Message : contains

    Device "*" -- "*" Bus : connected via BusConnection
    Bus "1" o-- "*" Message : transports
    Device "1" o-- "*" Message : transmits (optional)

    Message "1" *-- "*" Signal : packs
    Signal "1" *-- "*" SignalValue : defines state
    Signal "*" -- "*" Device : received by

    Signal ..> ByteOrder : uses
    Signal ..> MuxType : uses
```

---

## 3. Persistence, Database & Services Layer

This diagram abstracts SQLite operations, database lifecycle, DBC file import/export, and network validation against CAN bus physical rules.

```mermaid
classDiagram
    class Database {
        -sqlite3* handle
        +open(string_view path) bool
        +close() void
        +execute(string_view sql) bool
        +beginTransaction() void
        +commitTransaction() void
        +rollbackTransaction() void
        +isOpen() bool
        +getRawHandle() sqlite3*
    }

    class DatabaseManager {
        -Database db
        +initializeSchema(string_view schema_sql) bool
        +loadProject(string_view path, NetworkProject& out_project) bool
        +saveProject(string_view path, const NetworkProject& project) bool
        +insertDevice(Device& device) bool
        +updateDevice(const Device& device) bool
        +deleteDevice(int64_t device_id) bool
        +insertBus(Bus& bus) bool
        +insertMessage(Message& message) bool
        +insertSignal(Signal& signal) bool
    }

    class DbcParser {
        +importDbc(string_view dbc_path, NetworkProject& project) bool
        +exportDbc(const NetworkProject& project, int64_t bus_id, string_view out_path) bool
    }

    class NetworkValidator {
        +validateNetwork(const NetworkProject& project) ValidationReport
        +checkIdCollisions(const Bus& bus, const vector~Message~& messages) vector~string~
        +checkSignalOverlaps(const Message& message) vector~string~
        +calculateBusLoad(const Bus& bus, const vector~Message~& messages) double
    }

    class ValidationReport {
        +vector~string~ errors
        +vector~string~ warnings
        +bool isValid() bool
    }

    DatabaseManager *-- Database
    DatabaseManager ..> NetworkProject : populates / serializes
    DbcParser ..> NetworkProject : imports / exports
    NetworkValidator ..> NetworkProject : analyzes
    NetworkValidator ..> ValidationReport : produces
```

---

## 4. CAD Command Pattern (Undo / Redo Architecture)

Professional CAD software relies on an undo/redo command stack for schematic mutations. This pattern decouples UI actions from the domain model and database synchronization.

```mermaid
classDiagram
    class ICommand {
        <<interface>>
        +execute() bool*
        +undo() bool*
        +getName() string_view*
    }

    class CommandStack {
        -vector~unique_ptr~ICommand~~ undo_stack
        -vector~unique_ptr~ICommand~~ redo_stack
        +execute(unique_ptr~ICommand~ command) bool
        +undo() bool
        +redo() bool
        +canUndo() bool
        +canRedo() bool
        +clear() void
    }

    class MoveDeviceCommand {
        -NetworkProject& project
        -int64_t device_id
        -float old_x
        -float old_y
        -float new_x
        -float new_y
        +execute() bool
        +undo() bool
    }

    class ConnectDeviceBusCommand {
        -NetworkProject& project
        -int64_t device_id
        -int64_t bus_id
        +execute() bool
        +undo() bool
    }

    class AddMessageCommand {
        -NetworkProject& project
        -Message message
        -int64_t generated_id
        +execute() bool
        +undo() bool
    }

    class ModifySignalCommand {
        -NetworkProject& project
        -int64_t signal_id
        -Signal old_state
        -Signal new_state
        +execute() bool
        +undo() bool
    }

    ICommand <|.. MoveDeviceCommand
    ICommand <|.. ConnectDeviceBusCommand
    ICommand <|.. AddMessageCommand
    ICommand <|.. ModifySignalCommand

    CommandStack *-- ICommand
```
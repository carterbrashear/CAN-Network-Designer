-- TestDatabaseSetup.sql
-- CAN Network Designer Demo Database Setup Script

-- 1. Devices (Electronic Control Units)
INSERT INTO devices (name, pos_x, pos_y, comment) VALUES
    ('Engine_Control_Module', 150.0, 100.0, 'Primary powertrain controller for engine management'),
    ('Transmission_Control_Module', 400.0, 100.0, 'Controls gearbox shift scheduling and clutch actuation'),
    ('ABS_Brake_System', 150.0, 300.0, 'Monitors wheel speeds and controls brake pressure'),
    ('Body_Control_Module', 400.0, 300.0, 'Controls lighting, door locks, and cabin accessories'),
    ('Instrument_Cluster', 650.0, 200.0, 'Digital cluster display for driver gauges and alerts');

-- 2. Buses
INSERT INTO buses (name, nominal_bit_rate, data_bit_rate, is_fd, color, comment) VALUES
    ('Powertrain_CAN', 500000, 2000000, 1, 65280, 'High-speed CAN FD bus for real-time powertrain control'),
    ('Body_CAN', 250000, 250000, 0, 255, 'Standard CAN bus for body and convenience systems');

-- 3. Bus Connections
-- Engine_Control_Module -> Powertrain_CAN
INSERT INTO bus_connections (device_id, bus_id, num) VALUES (1, 1, 0);
-- Transmission_Control_Module -> Powertrain_CAN
INSERT INTO bus_connections (device_id, bus_id, num) VALUES (2, 1, 0);
-- ABS_Brake_System -> Powertrain_CAN
INSERT INTO bus_connections (device_id, bus_id, num) VALUES (3, 1, 0);
-- Body_Control_Module -> Powertrain_CAN and Body_CAN (Gateway ECU)
INSERT INTO bus_connections (device_id, bus_id, num) VALUES (4, 1, 0);
INSERT INTO bus_connections (device_id, bus_id, num) VALUES (4, 2, 0);
-- Instrument_Cluster -> Powertrain_CAN and Body_CAN
INSERT INTO bus_connections (device_id, bus_id, num) VALUES (5, 1, 0);
INSERT INTO bus_connections (device_id, bus_id, num) VALUES (5, 2, 0);

-- 4. Messages
INSERT INTO messages (bus_id, transmitter_id, can_id, is_extended, is_fd, dlc, name, cycle_time_ms, comment) VALUES
    (1, 1, 256, 0, 1, 8, 'Engine_Data_01', 10, 'Engine telemetry and RPM broadcast'),
    (1, 2, 384, 0, 1, 8, 'Transmission_Status', 20, 'Current gear state and transmission alerts'),
    (1, 3, 512, 0, 1, 8, 'Wheel_Speed_Report', 10, 'Four wheel individual speed readings'),
    (2, 4, 848, 0, 0, 4, 'Body_Lights_Status', 100, 'Exterior and interior lighting status');

-- 5. Signals
-- Signals for Engine_Data_01 (message_id = 1)
INSERT INTO signals (message_id, name, start_bit, bit_length, byte_order, is_signed, factor, offset, min_value, max_value, unit, mux_type, mux_value, comment) VALUES
    (1, 'EngineSpeed_RPM', 0, 16, 0, 0, 0.25, 0.0, 0.0, 8000.0, 'rpm', 0, NULL, 'Crankshaft rotational speed'),
    (1, 'ThrottlePosition', 16, 8, 0, 0, 0.392, 0.0, 0.0, 100.0, '%', 0, NULL, 'Accelerator pedal throttle input'),
    (1, 'EngineCoolantTemp', 24, 8, 0, 0, 1.0, -40.0, -40.0, 215.0, 'degC', 0, NULL, 'Engine temperature sensor reading');

-- Signals for Transmission_Status (message_id = 2)
INSERT INTO signals (message_id, name, start_bit, bit_length, byte_order, is_signed, factor, offset, min_value, max_value, unit, mux_type, mux_value, comment) VALUES
    (2, 'CurrentGear', 0, 4, 0, 0, 1.0, 0.0, 0.0, 8.0, '', 0, NULL, 'Engaged gear position'),
    (2, 'ClutchEngaged', 4, 1, 0, 0, 1.0, 0.0, 0.0, 1.0, '', 0, NULL, 'Clutch engagement binary indicator');

-- Signals for Wheel_Speed_Report (message_id = 3)
INSERT INTO signals (message_id, name, start_bit, bit_length, byte_order, is_signed, factor, offset, min_value, max_value, unit, mux_type, mux_value, comment) VALUES
    (3, 'FrontLeftWheelSpeed', 0, 16, 0, 0, 0.01, 0.0, 0.0, 300.0, 'km/h', 0, NULL, 'Front-left ABS sensor speed'),
    (3, 'FrontRightWheelSpeed', 16, 16, 0, 0, 0.01, 0.0, 0.0, 300.0, 'km/h', 0, NULL, 'Front-right ABS sensor speed');

-- Signals for Body_Lights_Status (message_id = 4)
INSERT INTO signals (message_id, name, start_bit, bit_length, byte_order, is_signed, factor, offset, min_value, max_value, unit, mux_type, mux_value, comment) VALUES
    (4, 'HeadlightsOn', 0, 1, 0, 0, 1.0, 0.0, 0.0, 1.0, '', 0, NULL, 'Low/High beam active state'),
    (4, 'TurnSignalStatus', 1, 2, 0, 0, 1.0, 0.0, 0.0, 3.0, '', 0, NULL, 'Indicator direction status');

-- 6. Signal Values (Enum Descriptions)
INSERT INTO signal_values (signal_id, raw_value, description) VALUES
    (4, 0, 'Neutral'),
    (4, 1, '1st Gear'),
    (4, 2, '2nd Gear'),
    (4, 3, '3rd Gear'),
    (4, 4, '4th Gear'),
    (4, 5, '5th Gear'),
    (4, 6, '6th Gear'),
    (4, 7, 'Reverse'),
    (4, 8, 'Park'),
    (9, 0, 'Off'),
    (9, 1, 'Left Indicator'),
    (9, 2, 'Right Indicator'),
    (9, 3, 'Hazard Flashers');

-- 7. Signal Receivers
INSERT INTO signal_receivers (signal_id, device_id) VALUES
    (1, 5), -- Instrument_Cluster receives EngineSpeed_RPM
    (3, 5), -- Instrument_Cluster receives EngineCoolantTemp
    (4, 1), -- Engine_Control_Module receives CurrentGear
    (4, 5), -- Instrument_Cluster receives CurrentGear
    (8, 5); -- Instrument_Cluster receives HeadlightsOn

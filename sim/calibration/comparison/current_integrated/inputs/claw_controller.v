`timescale 1ns / 1ps
module claw_controller #(
    parameter integer CLK_HZ = 50000000,
    parameter integer BAUD = 115200,
    parameter integer RESPONSE_MS = 200,
    parameter integer POLL_MS = 50,
    parameter integer MOTION_MS = 60000,
    parameter integer BASE_SETTLE_MS = 1000,
    parameter integer GRIP_SETTLE_MS = 500,
    parameter integer STARTUP_WAIT_MS = 5000,
    parameter integer SERVO_STEP_MS = 20,
    // Mechanical calibration of the 180-degree gripper servo:
    // 0 degrees is closed and 35 degrees is open.
    parameter integer GRIPPER_CLOSED_ANGLE = 0,
    parameter integer GRIPPER_OPEN_ANGLE = 35
)(
    input wire clk,
    input wire reset_n,
    input wire pc_rx,
    output wire pc_tx,
    input wire motor1_rx,
    output wire motor1_tx,
    input wire motor2_rx,
    output wire motor2_tx,
    output reg [8:0] base_angle,
    output reg [8:0] gripper_angle
);
    `include "coordinate_defaults.vh"
    `include "automatic_coordinates.vh"
    localparam [5:0] IDLE = 0, SEQ_SEND = 1, SEQ_WAIT = 2,
        MOVE_SEND = 3, MOVE_WAIT = 4, POLL_DELAY = 5, POLL_SEND = 6,
        POLL_WAIT = 7, POSITION_SEND = 8, POSITION_WAIT = 9,
        SERVO_WAIT = 10, LIST_SEND = 11, LIST_WAIT = 12,
        STOP_DRAIN = 13, STOP_WAIT = 14, ZERO_SEND = 15, ZERO_WAIT = 16,
        INFO_SEND = 19, INFO_WAIT = 20,
        MOVE_CHECK_SEND = 21, MOVE_CHECK_WAIT = 22,
        HOME_INFO_SEND = 23, HOME_INFO_WAIT = 24, HOME_SEND = 25,
        HOME_WAIT = 26, HOME_DELAY = 27, HOME_POLL_SEND = 28, HOME_POLL_WAIT = 29,
        HOME_STOP_DRAIN = 30, HOME_STOP_WAIT = 31,
        AUTO_MAP_WAIT = 32, AUTO_DISPATCH = 33, AUTO_REPORT_WAIT = 34;
    localparam [1:0] OP_READY = 0, OP_READ = 1, OP_SAVE = 2, OP_DIAG = 3;
    localparam integer MS_TICKS = CLK_HZ / 1000;
    reg [5:0] state;
    reg auto_active;
    reg [1:0] auto_kind;
    reg [2:0] auto_step;
    reg [2:0] auto_group;
    reg [4:0] auto_coordinate;
    reg [1:0] drop_slot [1:6];
    reg carrying;
    reg move_absolute;
    reg mapper_start;
    wire mapper_done;
    wire [31:0] mapped_motor1;
    wire [8:0] mapped_base;
    reg [31:0] target_motor1;
    reg signed [31:0] target_motor2;
    reg [8:0] target_base;
    reg signed [31:0] map_x;
    reg signed [31:0] map_y;
    reg ready;
    reg [31:0] startup_elapsed;
    reg startup_complete;
    reg startup_unlock_blocked;
    reg [1:0] operation;
    reg [2:0] phase;
    reg axis;
    reg [4:0] coordinate_index;
    reg [28:0] coordinate_valid;
    reg signed [31:0] coordinate_motor1 [0:28];
    reg signed [31:0] coordinate_motor2 [0:28];
    reg [8:0] coordinate_base [0:28];
    reg signed [31:0] actual_motor1;
    reg signed [31:0] actual_motor2;
    reg [7:0] diagnostic_home1;
    reg [7:0] diagnostic_home2;
    reg [7:0] diagnostic_drive1;
    reg [15:0] diagnostic_version;
    wire [119:0] home_parameters1;
    wire [119:0] home_parameters2;
    wire [119:0] selected_home_parameters = axis ? home_parameters2 : home_parameters1;
    reg [15:0] speed;
    reg [7:0] acceleration;
    reg move_direction;
    reg [31:0] move_pulses;
    reg [31:0] ms_divider;
    wire ms_tick = ms_divider == MS_TICKS - 1;
    reg [31:0] motion_elapsed;
    reg [31:0] delay_elapsed;
    reg stable_arrival;
    wire [31:0] motor2_move_tx_count;
    wire [31:0] motor2_last_tx_pulses;
    wire motor2_last_tx_direction;
    reg servo_is_base;
    reg [8:0] servo_target;
    reg homing;
    reg stop_home_failed;
    reg [1:0] link_start;
    reg [1:0] link_cancel;
    reg [7:0] link_function;
    wire [1:0] link_busy;
    wire [1:0] link_done;
    wire [3:0] link_error1;
    wire [3:0] link_error2;
    wire signed [31:0] link_result1;
    wire signed [31:0] link_result2;
    wire selected_busy = axis ? link_busy[1] : link_busy[0];
    wire selected_done = axis ? link_done[1] : link_done[0];
    wire [3:0] selected_error = axis ? link_error2 : link_error1;
    wire signed [31:0] selected_result = axis ? link_result2 : link_result1;
    reg [1:0] stopped_mask;
    reg [1:0] stop_targets;
    reg stop_failed;
    reg [191:0] stop_message;
    wire [7:0] pc_data;
    wire pc_valid;
    wire pc_frame_error;
    wire parsed_command_valid;
    wire [4:0] parsed_command;
    wire [2:0] parsed_argument_count;
    wire signed [31:0] parsed_argument0;
    wire signed [31:0] parsed_argument1;
    wire command_valid = parsed_command_valid;
    wire [4:0] command = parsed_command;
    wire [2:0] argument_count = parsed_argument_count;
    wire signed [31:0] argument0 = parsed_argument0;
    reg report_start;
    reg [191:0] report_prefix;
    reg [2:0] report_count;
    reg [191:0] report_values;
    wire report_busy;
    wire report_done;
    reg busy_notice;
    reg [5:0] fault_state;
    reg [3:0] fault_error;
    wire query_valid = command_valid && (command == 13 || command == 23) && argument_count == 0;
    wire [191:0] status_values = {24'b0, acceleration, 16'b0, speed,
        23'b0, gripper_angle, 23'b0, base_angle, 3'b0, coordinate_valid, 31'b0, ready};
    wire [191:0] progress_values = {28'b0, fault_error, 26'b0, fault_state,
        motion_elapsed, 31'b0, axis, 29'b0, auto_step, 26'b0, state};
    wire report_available = !report_busy && !report_start;
    wire moving = (state >= MOVE_SEND && state <= POSITION_WAIT) || state == MOVE_CHECK_SEND || state == MOVE_CHECK_WAIT ||
        (state >= HOME_INFO_SEND && state <= HOME_POLL_WAIT);
    integer reset_index;
    uart_rx #(.CLK_HZ(CLK_HZ), .BAUD(BAUD)) pc_receiver(
        .clk(clk), .reset_n(reset_n), .rx(pc_rx), .data(pc_data),
        .valid(pc_valid), .frame_error(pc_frame_error)
    );
    command_parser parser(
        .clk(clk), .reset_n(reset_n), .data(pc_data), .valid(pc_valid),
        .frame_error(pc_frame_error), .command_valid(parsed_command_valid),
        .command(parsed_command), .argument_count(parsed_argument_count), .argument0(parsed_argument0),
        .argument1(parsed_argument1)
    );
    coordinate_mapper mapper(
        .clk(clk), .reset_n(reset_n), .start(mapper_start), .x_centi_cm(map_x), .y_centi_cm(map_y),
        .done(mapper_done), .motor1_pulses(mapped_motor1), .base_degrees(mapped_base)
    );
    pc_report_mux #(.CLK_HZ(CLK_HZ), .BAUD(BAUD)) reporter(
        .clk(clk), .reset_n(reset_n), .start(report_start), .prefix(report_prefix),
        .value_count(report_count), .values(report_values), .busy(report_busy),
        .query_start(query_valid), .query_prefix(command == 23 ? "PROGRESS" : "STATUS"),
        .query_values(command == 23 ? progress_values : status_values),
        .done(report_done), .tx(pc_tx)
    );
    emm_link #(.CLK_HZ(CLK_HZ), .BAUD(BAUD), .ADDRESS(8'h01), .RESPONSE_MS(RESPONSE_MS)) motor1(
        .clk(clk), .reset_n(reset_n), .request(link_start[0]), .cancel(link_cancel[0]),
        .function_code(link_function), .direction(move_direction), .absolute_position(move_absolute), .pulses(move_pulses),
        .speed(speed), .acceleration(acceleration), .home_mode(selected_home_parameters[119:112]), .rx(motor1_rx), .tx(motor1_tx),
        .busy(link_busy[0]), .done(link_done[0]), .error_code(link_error1), .result(link_result1),
        .home_parameters(home_parameters1)
    );
    emm_link #(.CLK_HZ(CLK_HZ), .BAUD(BAUD), .ADDRESS(8'h02), .RESPONSE_MS(RESPONSE_MS)) motor2(
        .clk(clk), .reset_n(reset_n), .request(link_start[1]), .cancel(link_cancel[1]),
        .function_code(link_function), .direction(move_direction), .absolute_position(move_absolute), .pulses(move_pulses),
        .speed(speed), .acceleration(acceleration), .home_mode(selected_home_parameters[119:112]), .rx(motor2_rx), .tx(motor2_tx),
        .busy(link_busy[1]), .done(link_done[1]), .error_code(link_error2), .result(link_result2),
        .home_parameters(home_parameters2), .move_tx_count(motor2_move_tx_count),
        .last_tx_pulses(motor2_last_tx_pulses), .last_tx_direction(motor2_last_tx_direction)
    );
    task message;
        input [191:0] text;
        begin
            report_prefix <= text;
            report_count <= 0;
            report_values <= 0;
            report_start <= 1;
        end
    endtask
    task report_position;
        input signed [31:0] position1;
        input signed [31:0] position2;
        begin
            report_prefix <= "POS";
            report_count <= 4;
            report_values <= {64'b0, 23'b0, gripper_angle, 23'b0, base_angle, position2, position1};
            report_start <= 1;
        end
    endtask
    function [191:0] coordinate_label;
        input [4:0] coordinate;
        reg [4:0] previous_index;
        begin
            previous_index = coordinate - 1'b1;
            coordinate_label = coordinate == 0 ? "COORD 0.0" :
                {"COORD ", (8'd48 + {5'b0, previous_index[4:2]}), 8'h2e,
                (8'd49 + {6'b0, previous_index[1:0]})};
        end
    endfunction
    task report_coordinate;
        input [4:0] coordinate;
        begin
            report_prefix <= coordinate_label(coordinate);
            report_count <= 4;
            report_values <= {64'b0, 23'b0, coordinate_base[coordinate],
                coordinate_motor2[coordinate], coordinate_motor1[coordinate],
                31'b0, coordinate_valid[coordinate]};
            report_start <= 1;
        end
    endtask
    task begin_stop;
        input [191:0] text;
        begin
            if (text != "OK STOP") begin
                fault_state <= state;
                fault_error <= text == "ERR MOTION_TIMEOUT" ? 4'd6 : selected_error;
            end
            auto_active <= 0;
            ready <= 0;
            startup_unlock_blocked <= 1;
            link_start <= 0;
            link_cancel <= 2'b11;
            stop_targets <= 2'b11;
            stop_message <= text;
            stopped_mask <= 0;
            stop_failed <= 0;
            stop_home_failed <= 0;
            busy_notice <= 0;
            state <= homing && ((state >= HOME_SEND && state <= HOME_POLL_WAIT) ||
                state == POSITION_SEND || state == POSITION_WAIT) ? HOME_STOP_DRAIN : STOP_DRAIN;
        end
    endtask
    task auto_motor;
        input selected_axis;
        input signed [31:0] target;
        begin
            axis <= selected_axis;
            move_absolute <= 1;
            move_direction <= target < 0;
            move_pulses <= target < 0 ? -target : target;
            homing <= 0;
            motion_elapsed <= 0;
            stable_arrival <= 0;
            phase <= 0;
            state <= MOVE_CHECK_SEND;
        end
    endtask
    task auto_servo;
        input selected_base;
        input [8:0] target;
        begin
            servo_is_base <= selected_base;
            servo_target <= target;
            delay_elapsed <= 0;
            state <= SERVO_WAIT;
        end
    endtask
    always @(posedge clk) begin
        if (!reset_n) ms_divider <= 0;
        else if (ms_tick) ms_divider <= 0;
        else ms_divider <= ms_divider + 1'b1;
    end
    always @(posedge clk) begin
        if (!reset_n) begin
            state <= IDLE;
            auto_active <= 0;
            auto_kind <= 0;
            auto_step <= 0;
            auto_group <= 0;
            auto_coordinate <= 0;
            carrying <= 0;
            move_absolute <= 0;
            mapper_start <= 0;
            target_motor1 <= 0;
            target_motor2 <= auto_motor2(0);
            target_base <= 135;
            map_x <= 0;
            map_y <= 0;
            for (reset_index = 1; reset_index <= 6; reset_index = reset_index + 1)
                drop_slot[reset_index] <= 0;
            ready <= 0;
            startup_elapsed <= 0;
            startup_complete <= 0;
            startup_unlock_blocked <= 0;
            operation <= OP_READ;
            phase <= 0;
            axis <= 0;
            coordinate_index <= 0;
            coordinate_valid <= DEFAULT_VALID;
            for (reset_index = 0; reset_index < 29; reset_index = reset_index + 1) begin
                coordinate_motor1[reset_index] <= default_motor1(reset_index);
                coordinate_motor2[reset_index] <= default_motor2(reset_index);
                coordinate_base[reset_index] <= default_base(reset_index);
            end
            actual_motor1 <= 0;
            actual_motor2 <= 0;
            diagnostic_home1 <= 0;
            diagnostic_home2 <= 0;
            diagnostic_drive1 <= 0;
            diagnostic_version <= 0;
            base_angle <= 45;
            gripper_angle <= GRIPPER_OPEN_ANGLE;
            speed <= 60;
            acceleration <= 100;
            move_direction <= 0;
            move_pulses <= 0;
            motion_elapsed <= 0;
            delay_elapsed <= 0;
            stable_arrival <= 0;
            servo_is_base <= 0;
            servo_target <= 45;
            homing <= 0;
            stop_home_failed <= 0;
            link_start <= 0;
            link_cancel <= 0;
            link_function <= 0;
            stopped_mask <= 0;
            stop_targets <= 2'b11;
            stop_failed <= 0;
            stop_message <= 0;
            report_start <= 0;
            report_prefix <= 0;
            report_count <= 0;
            report_values <= 0;
            busy_notice <= 0;
            fault_state <= 0;
            fault_error <= 0;
        end else begin
            link_start <= 0;
            link_cancel <= 0;
            report_start <= 0;
            mapper_start <= 0;
            if (!startup_complete && ms_tick) begin
                if (startup_elapsed >= STARTUP_WAIT_MS - 1) begin
                    startup_complete <= 1;
                    if (!startup_unlock_blocked) ready <= 1;
                end else startup_elapsed <= startup_elapsed + 1'b1;
            end
            if (moving && ms_tick) motion_elapsed <= motion_elapsed + 1'b1;
            case (state)
                IDLE: begin
                    if (busy_notice && report_available) begin
                        message("ERR BUSY");
                        busy_notice <= 0;
                    end
                    if (command_valid && !query_valid) begin
                        if (!report_available || busy_notice) busy_notice <= 1;
                        else if (command == 0 || (command == 20 && argument_count != 2) ||
                            (command != 20 &&
                            ((command == 2 || command == 3 || command == 4 || command == 5 || command == 6 || command == 7 || command == 10 || command == 11 || command == 15 || command == 19) ? argument_count != 1 : argument_count != 0))) message("ERR SYNTAX");
                        else if (carrying && (command == 2 || command == 3 || command == 4 || command == 5 || command == 19 || command == 20)) message("ERR EXPECT_DROP");
                        else case (command)
                            1: if (!startup_complete) message("ERR STARTUP_WAIT");
                                else begin
                                    ready <= 1;
                                    message("OK READY");
                                end
                            9, 14: begin
                                operation <= command == 14 ? OP_DIAG : OP_READ;
                                phase <= command == 9 ? 4 : 0;
                                axis <= 0;
                                state <= SEQ_SEND;
                            end
                            2, 3: if (!ready) message("ERR NOT_READY");
                                else if (argument0 == 0) message("ERR RANGE");
                                else begin
                                    axis <= command == 3;
                                    move_absolute <= 0;
                                    homing <= 0;
                                    move_direction <= argument0 < 0;
                                    move_pulses <= argument0 < 0 ? -argument0 : argument0;
                                    motion_elapsed <= 0;
                                    stable_arrival <= 0;
                                    phase <= 0;
                                    state <= MOVE_CHECK_SEND;
                                end
                            4, 5: if (!ready) message("ERR NOT_READY");
                                else if (command == 4 ? (argument0 < 0 || argument0 > 270) :
                                    (argument0 < GRIPPER_CLOSED_ANGLE || argument0 > GRIPPER_OPEN_ANGLE)) message("ERR RANGE");
                                else begin
                                    servo_target <= argument0[8:0];
                                    servo_is_base <= command == 4;
                                    delay_elapsed <= 0;
                                    state <= SERVO_WAIT;
                                end
                            6: if (!ready) message("ERR NOT_READY");
                                else if (argument0 < 0 || argument0 > 28) message("ERR RANGE");
                                else begin
                                    coordinate_index <= argument0[4:0];
                                    operation <= OP_SAVE;
                                    phase <= 0;
                                    axis <= 0;
                                    state <= SEQ_SEND;
                                end
                            7: if (argument0 < 0 || argument0 > 28) message("ERR RANGE");
                                else report_coordinate(argument0[4:0]);
                            8: begin
                                coordinate_index <= 0;
                                state <= LIST_SEND;
                            end
                            10: if (argument0 < 1 || argument0 > 3000) message("ERR RANGE");
                                else begin
                                    speed <= argument0[15:0];
                                    message("OK SPEED");
                                end
                            11: if (argument0 < 1 || argument0 > 255) message("ERR RANGE");
                                else begin
                                    acceleration <= argument0[7:0];
                                    message("OK ACC");
                                end
                            12: begin_stop("OK STOP");
                            13: begin
                                report_prefix <= "STATUS";
                                report_count <= 6;
                                report_values <= {24'b0, acceleration, 16'b0, speed,
                                    23'b0, gripper_angle, 23'b0, base_angle,
                                    3'b0, coordinate_valid, 31'b0, ready};
                                report_start <= 1;
                            end
                            15: if (argument0 != 1 && argument0 != 2) message("ERR RANGE");
                                else begin
                                    axis <= argument0 == 2;
                                    phase <= 0;
                                    state <= INFO_SEND;
                                end
                            16: begin
                                report_prefix <= "LIFT";
                                report_count <= 6;
                                report_values <= {64'b0,
                                    31'b0, motor2_last_tx_direction, motor2_last_tx_pulses,
                                    motor2_move_tx_count, 32'd5};
                                report_start <= 1;
                            end
                            17: begin
                                report_prefix <= "HLOG";
                                report_count <= 6;
                                report_values <= {motor2_move_tx_count, 160'b0};
                                report_start <= 1;
                            end
                            18: if (ready) message("ERR NOT_LOCKED");
                                else begin
                                    axis <= 0;
                                    phase <= 0;
                                    state <= ZERO_SEND;
                                end
                            19: if (!ready) message("ERR NOT_READY");
                                else if (argument0 != 1 && argument0 != 2) message("ERR RANGE");
                                else begin
                                    axis <= argument0 == 2;
                                    homing <= 1;
                                    motion_elapsed <= 0;
                                    stable_arrival <= 0;
                                    phase <= 0;
                                    state <= HOME_INFO_SEND;
                                end
                            20: if (!ready) message("ERR NOT_READY");
                                else begin
                                    map_x <= argument0;
                                    map_y <= parsed_argument1;
                                    mapper_start <= 1;
                                    auto_active <= 1;
                                    auto_kind <= 0;
                                    auto_step <= 0;
                                    state <= AUTO_MAP_WAIT;
                                end
                            21: if (!ready) message("ERR NOT_READY");
                                else if (!carrying) message("ERR EXPECT_PICK");
                                else begin
                                    auto_group <= argument0[2:0];
                                    auto_coordinate <= argument0 * 4 + drop_slot[argument0[2:0]] + 1;
                                    auto_active <= 1;
                                    auto_kind <= 1;
                                    auto_step <= 0;
                                    state <= AUTO_DISPATCH;
                                end
                            22: if (!ready) message("ERR NOT_READY");
                                else begin
                                    auto_coordinate <= 0;
                                    auto_active <= 1;
                                    auto_kind <= 2;
                                    auto_step <= 0;
                                    state <= AUTO_DISPATCH;
                                end
                            default: message("ERR SYNTAX");
                        endcase
                    end
                end
                SEQ_SEND: if (!selected_busy) begin
                    link_function <= phase < 2 ? 8'h3b : (phase < 4 ? 8'h3a : 8'h36);
                    link_start <= axis ? 2'b10 : 2'b01;
                    state <= SEQ_WAIT;
                end
                SEQ_WAIT: if (selected_done) begin
                    if (selected_error != 0) begin
                        ready <= 0;
                        message("ERR MOTOR_LINK");
                        state <= IDLE;
                    end else if (operation != OP_DIAG && phase < 2 && selected_result[5:4] != 0) begin
                        ready <= 0;
                        message("ERR MOTOR_PROTECT");
                        state <= IDLE;
                    end else if (operation != OP_DIAG && phase >= 2 && phase < 4 &&
                        (!selected_result[0] || selected_result[3:2] != 0 || (operation == OP_SAVE && !selected_result[1]))) begin
                        ready <= 0;
                        message("ERR DRIVE_STATE");
                        state <= IDLE;
                    end else if (operation == OP_DIAG) begin
                        case (phase)
                            0: diagnostic_home1 <= selected_result[7:0];
                            1: diagnostic_home2 <= selected_result[7:0];
                            2: diagnostic_drive1 <= selected_result[7:0];
                        endcase
                        if (phase == 3) begin
                            report_prefix <= "DIAG";
                            report_count <= 4;
                            report_values <= {64'b0, selected_result, 24'b0, diagnostic_drive1,
                                24'b0, diagnostic_home2, 24'b0, diagnostic_home1};
                            report_start <= 1;
                            state <= IDLE;
                        end else begin
                            phase <= phase + 1'b1;
                            axis <= !phase[0];
                            state <= SEQ_SEND;
                        end
                    end else begin
                        if (phase == 4) actual_motor1 <= selected_result;
                        if (phase == 5) begin
                            actual_motor2 <= selected_result;
                            state <= IDLE;
                            if (operation == OP_READY) begin
                                ready <= 1;
                                message("OK READY");
                            end else if (operation == OP_SAVE) begin
                                coordinate_motor1[coordinate_index] <= actual_motor1;
                                coordinate_motor2[coordinate_index] <= selected_result;
                                coordinate_base[coordinate_index] <= base_angle;
                                coordinate_valid[coordinate_index] <= 1;
                                report_prefix <= coordinate_label(coordinate_index);
                                report_count <= 4;
                                report_values <= {64'b0, 23'b0, base_angle, selected_result, actual_motor1,
                                    32'd1};
                                report_start <= 1;
                            end else report_position(actual_motor1, selected_result);
                        end else begin
                            phase <= phase + 1'b1;
                            axis <= !phase[0];
                            state <= SEQ_SEND;
                        end
                    end
                end
                MOVE_CHECK_SEND: if (!selected_busy) begin
                    link_function <= phase == 0 ? 8'h3b : 8'h3a;
                    link_start <= axis ? 2'b10 : 2'b01;
                    state <= MOVE_CHECK_WAIT;
                end
                MOVE_CHECK_WAIT: if (selected_done) begin
                    if (selected_error != 0) begin_stop("ERR MOTOR_LINK");
                    else if (phase == 0 && selected_result[5:4] != 0) begin_stop("ERR MOTOR_PROTECT");
                    else if (phase == 0 && homing && selected_result[2]) begin_stop("ERR HOME_STATE");
                    else if (phase == 1 && (!selected_result[0] || selected_result[3:2] != 0)) begin_stop("ERR DRIVE_STATE");
                    else if (phase == 0) begin
                        phase <= 1;
                        state <= MOVE_CHECK_SEND;
                    end else state <= homing ? HOME_SEND : MOVE_SEND;
                end
                MOVE_SEND: if (!selected_busy) begin
                    link_function <= 8'hfd;
                    link_start <= axis ? 2'b10 : 2'b01;
                    state <= MOVE_WAIT;
                end
                MOVE_WAIT: if (selected_done) begin
                    if (selected_error != 0) begin_stop("ERR MOVE_ACK");
                    else begin
                        delay_elapsed <= 0;
                        state <= POLL_DELAY;
                    end
                end
                POLL_DELAY: if (ms_tick) begin
                    if (delay_elapsed >= POLL_MS - 1) state <= POLL_SEND;
                    else delay_elapsed <= delay_elapsed + 1'b1;
                end
                POLL_SEND: if (!selected_busy) begin
                    link_function <= 8'h3a;
                    link_start <= axis ? 2'b10 : 2'b01;
                    state <= POLL_WAIT;
                end
                POLL_WAIT: if (selected_done) begin
                    if (selected_error != 0) begin_stop("ERR MOTOR_LINK");
                    else if (!selected_result[0] || selected_result[3:2] != 0) begin_stop("ERR DRIVE_STATE");
                    else if (selected_result[1] && stable_arrival) state <= POSITION_SEND;
                    else begin
                        stable_arrival <= selected_result[1];
                        delay_elapsed <= 0;
                        state <= POLL_DELAY;
                    end
                end
                POSITION_SEND: if (!selected_busy) begin
                    link_function <= 8'h36;
                    link_start <= axis ? 2'b10 : 2'b01;
                    state <= POSITION_WAIT;
                end
                POSITION_WAIT: if (selected_done) begin
                    if (selected_error != 0) begin_stop("ERR MOTOR_LINK");
                    else begin
                        if (axis) actual_motor2 <= selected_result;
                        else actual_motor1 <= selected_result;
                        if (auto_active) begin
                            auto_step <= auto_step + 1'b1;
                        end else if (homing) message(axis ? "OK HOME2" : "OK HOME1");
                        else report_position(axis ? actual_motor1 : selected_result, axis ? selected_result : actual_motor2);
                        homing <= 0;
                        state <= auto_active ? AUTO_DISPATCH : IDLE;
                    end
                end
                SERVO_WAIT: if (ms_tick) begin
                    if ((servo_is_base ? base_angle : gripper_angle) != servo_target) begin
                        if (delay_elapsed >= SERVO_STEP_MS - 1) begin
                            if (servo_is_base) base_angle <= base_angle < servo_target ? base_angle + 1'b1 : base_angle - 1'b1;
                            else gripper_angle <= gripper_angle < servo_target ? gripper_angle + 1'b1 : gripper_angle - 1'b1;
                            delay_elapsed <= 0;
                        end else delay_elapsed <= delay_elapsed + 1'b1;
                    end else if (delay_elapsed >= (servo_is_base ? BASE_SETTLE_MS : GRIP_SETTLE_MS) - 1) begin
                        if (auto_active) auto_step <= auto_step + 1'b1;
                        else message(servo_is_base ? "OK BASE" : "OK GRIP");
                        state <= auto_active ? AUTO_DISPATCH : IDLE;
                    end else delay_elapsed <= delay_elapsed + 1'b1;
                end
                AUTO_MAP_WAIT: if (mapper_done) begin
                    target_motor1 <= mapped_motor1;
                    target_motor2 <= auto_motor2(0);
                    target_base <= mapped_base;
                    state <= AUTO_DISPATCH;
                end
                AUTO_DISPATCH: begin
                    if (auto_kind == 0) begin
                        case (auto_step)
                            0: auto_motor(1'b1, -32'sd1800);
                            1: auto_servo(1'b1, target_base);
                            2: auto_motor(1'b0, target_motor1);
                            3: auto_motor(1'b1, target_motor2);
                             4: auto_servo(1'b0, GRIPPER_CLOSED_ANGLE);
                            5: if (report_available) begin
                                message("0 OK");
                                state <= AUTO_REPORT_WAIT;
                            end
                            default: begin_stop("ERR AUTO_STATE");
                        endcase
                    end else begin
                        case (auto_step)
                            0: auto_motor(1'b1, -32'sd1800);
                            1: auto_motor(1'b0, auto_motor1(auto_coordinate));
                            2: auto_servo(1'b1, auto_base(auto_coordinate));
                            3: auto_motor(1'b1, auto_motor2(auto_coordinate));
                             4: auto_servo(1'b0, GRIPPER_OPEN_ANGLE);
                            5: if (report_available) begin
                                if (auto_kind == 2) message("OVER OK");
                                else message({8'd48 + {5'b0, auto_group}, " OK"});
                                state <= AUTO_REPORT_WAIT;
                            end
                            default: begin_stop("ERR AUTO_STATE");
                        endcase
                    end
                end
                AUTO_REPORT_WAIT: if (report_done) begin
                    carrying <= auto_kind == 0;
                    if (auto_kind == 1) drop_slot[auto_group] <= drop_slot[auto_group] + 1'b1;
                    if (auto_kind == 2)
                        for (reset_index = 1; reset_index <= 6; reset_index = reset_index + 1)
                            drop_slot[reset_index] <= 0;
                    auto_active <= 0;
                    state <= IDLE;
                end
                LIST_SEND: if (report_available) begin
                    report_coordinate(coordinate_index);
                    state <= LIST_WAIT;
                end
                LIST_WAIT: if (report_done) begin
                    if (coordinate_index == 28) state <= IDLE;
                    else begin
                        coordinate_index <= coordinate_index + 1'b1;
                        state <= LIST_SEND;
                    end
                end
                ZERO_SEND: if (!selected_busy) begin
                    case (phase)
                        0: link_function <= 8'h22;
                        1: link_function <= 8'h3b;
                        2: link_function <= 8'h3a;
                        default: link_function <= 8'h93;
                    endcase
                    link_start <= 2'b01;
                    state <= ZERO_WAIT;
                end
                ZERO_WAIT: if (selected_done) begin
                    if (selected_error != 0) begin
                        message("ERR MOTOR_LINK");
                        state <= IDLE;
                    end else if (phase == 0 && home_parameters1[119:112] > 1) begin
                        message("ERR HOME_MODE");
                        state <= IDLE;
                    end else if (phase == 1 && (selected_result[5:4] != 0 || selected_result[2] || selected_result[1:0] != 2'b11)) begin
                        message("ERR HOME_STATE");
                        state <= IDLE;
                    end else if (phase == 2 && selected_result[3:0] != 4'h3) begin
                        message("ERR DRIVE_STATE");
                        state <= IDLE;
                    end else if (phase == 3) begin
                        coordinate_valid <= 0;
                        message("OK M1ZERO");
                        state <= IDLE;
                    end else begin
                        phase <= phase + 1'b1;
                        state <= ZERO_SEND;
                    end
                end
                INFO_SEND: if (!selected_busy) begin
                    link_function <= phase == 0 ? 8'h1f : 8'h22;
                    link_start <= axis ? 2'b10 : 2'b01;
                    state <= INFO_WAIT;
                end
                INFO_WAIT: if (selected_done) begin
                    if (selected_error != 0) begin
                        message("ERR MOTOR_LINK");
                        state <= IDLE;
                    end else if (phase == 0) begin
                        diagnostic_version <= selected_result[31:16];
                        phase <= 1;
                        state <= INFO_SEND;
                    end else begin
                        report_prefix <= axis ? "MOTOR2_CFG" : "MOTOR1_CFG";
                        report_count <= 6;
                        report_values <= {24'b0, selected_home_parameters[7:0], selected_home_parameters[87:56],
                            16'b0, selected_home_parameters[103:88], 24'b0, selected_home_parameters[111:104],
                            24'b0, selected_home_parameters[119:112], 16'b0, diagnostic_version};
                        report_start <= 1;
                        state <= IDLE;
                    end
                end
                HOME_INFO_SEND: if (!selected_busy) begin
                    link_function <= 8'h22;
                    link_start <= axis ? 2'b10 : 2'b01;
                    state <= HOME_INFO_WAIT;
                end
                HOME_INFO_WAIT: if (selected_done) begin
                    if (selected_error != 0) begin_stop("ERR MOTOR_LINK");
                    else if (selected_home_parameters[119:112] > 3) begin
                        message("ERR HOME_MODE");
                        homing <= 0;
                        state <= IDLE;
                    end else begin
                        phase <= 0;
                        state <= MOVE_CHECK_SEND;
                    end
                end
                HOME_SEND: if (!selected_busy) begin
                    link_function <= 8'h9a;
                    link_start <= axis ? 2'b10 : 2'b01;
                    state <= HOME_WAIT;
                end
                HOME_WAIT: if (selected_done) begin
                    if (selected_error != 0) begin_stop("ERR HOME_ACK");
                    else begin
                        delay_elapsed <= 0;
                        state <= HOME_DELAY;
                    end
                end
                HOME_DELAY: if (ms_tick) begin
                    if (delay_elapsed >= POLL_MS - 1) state <= HOME_POLL_SEND;
                    else delay_elapsed <= delay_elapsed + 1'b1;
                end
                HOME_POLL_SEND: if (!selected_busy) begin
                    link_function <= 8'h3b;
                    link_start <= axis ? 2'b10 : 2'b01;
                    state <= HOME_POLL_WAIT;
                end
                HOME_POLL_WAIT: if (selected_done) begin
                    if (selected_error != 0) begin_stop("ERR MOTOR_LINK");
                    else if (selected_result[5:4] != 0) begin_stop("ERR MOTOR_PROTECT");
                    else if (selected_result[3]) begin_stop("ERR HOME_FAILED");
                    else if (selected_result[1:0] != 2'b11) begin_stop("ERR HOME_STATE");
                    else if (!selected_result[2] && stable_arrival) state <= POSITION_SEND;
                    else begin
                        stable_arrival <= !selected_result[2];
                        delay_elapsed <= 0;
                        state <= HOME_DELAY;
                    end
                end
                HOME_STOP_DRAIN: if (link_busy == 0 && link_start == 0) begin
                    link_function <= 8'h9c;
                    link_start <= axis ? 2'b10 : 2'b01;
                    state <= HOME_STOP_WAIT;
                end
                HOME_STOP_WAIT: if (selected_done) begin
                    stop_home_failed <= selected_error != 0;
                    state <= STOP_DRAIN;
                end
                STOP_DRAIN: if (link_busy == 0 && link_start == 0) begin
                    link_function <= 8'hfe;
                    link_start <= stop_targets;
                    stopped_mask <= 0;
                    stop_failed <= stop_home_failed;
                    state <= STOP_WAIT;
                end
                STOP_WAIT: begin
                    stopped_mask <= stopped_mask | link_done;
                    if ((link_done[0] && link_error1 != 0) || (link_done[1] && link_error2 != 0)) stop_failed <= 1;
                    if (stopped_mask == stop_targets && report_available) begin
                        message(stop_failed ? "ERR STOP_LINK" : stop_message);
                        state <= IDLE;
                    end
                end
            endcase
            if (state != IDLE && command_valid && !query_valid && !(command == 12 && argument_count == 0)) busy_notice <= 1;
            if (moving && motion_elapsed >= MOTION_MS) begin_stop("ERR MOTION_TIMEOUT");
            if (command_valid && command == 12 && argument_count == 0 && state != STOP_DRAIN && state != STOP_WAIT && state != HOME_STOP_DRAIN && state != HOME_STOP_WAIT) begin_stop("OK STOP");
            if (gripper_angle > GRIPPER_OPEN_ANGLE) gripper_angle <= GRIPPER_OPEN_ANGLE;
        end
    end
endmodule

`timescale 1ns / 1ps
module motor_model #(
    parameter [7:0] ADDRESS = 1,
    parameter integer INITIAL_POSITION = 0
)(
    input wire clk,
    input wire reset_n,
    input wire rx,
    output wire tx,
    input wire [7:0] home_status,
    input wire drop_replies,
    input wire stall
);
    wire [7:0] data;
    wire valid;
    reg [7:0] request_bytes [0:12];
    reg [3:0] received_count = 0;
    reg [3:0] request_length = 3;
    reg [7:0] response_bytes [0:17];
    reg [4:0] response_length;
    reg [4:0] response_index;
    reg response_pending;
    reg [1:0] response_state;
    reg tx_start;
    reg [7:0] tx_data;
    wire tx_done;
    reg signed [31:0] position;
    reg [31:0] magnitude;
    reg signed [63:0] delta_position;
    reg [3:0] polls_left;
    integer command_count = 0;
    integer move_count = 0;
    integer stop_count = 0;
    integer poll_count = 0;
    integer zero_count = 0;
    integer home_count = 0;
    integer home_interrupt_count = 0;
    reg [7:0] home_ack = 8'h02;
    reg fail_home = 0;
    reg hold_home = 0;
    reg hold_arrival = 0;
    reg home_active;
    reg [3:0] home_polls_left;
    reg [7:0] last_home_mode;
    reg [15:0] last_move_speed;
    reg [7:0] zero_ack = 8'h02;
    reg last_move_direction;
    reg [31:0] last_move_pulses;
    uart_rx #(.CLK_HZ(1000000), .BAUD(100000)) receiver(
        .clk(clk), .reset_n(reset_n), .rx(rx), .data(data), .valid(valid), .frame_error()
    );
    uart_tx #(.CLK_HZ(1000000), .BAUD(100000)) transmitter(
        .clk(clk), .reset_n(reset_n), .start(tx_start), .data(tx_data), .tx(tx), .busy(), .done(tx_done)
    );
    always @(posedge clk) begin
        if (!reset_n) begin
            received_count <= 0;
            request_length <= 3;
            response_length <= 0;
            response_index <= 0;
            response_pending <= 0;
            response_state <= 0;
            tx_start <= 0;
            tx_data <= 0;
            position <= INITIAL_POSITION;
            polls_left <= 0;
            command_count <= 0;
            move_count <= 0;
            stop_count <= 0;
            poll_count <= 0;
            zero_count <= 0;
            home_count <= 0;
            home_interrupt_count <= 0;
            home_active <= 0;
            home_polls_left <= 0;
            last_home_mode <= 0;
            last_move_speed <= 0;
            last_move_direction <= 0;
            last_move_pulses <= 0;
        end else begin
            tx_start <= 0;
            if (valid) begin
                request_bytes[received_count] <= data;
                if (received_count == 0 && data != ADDRESS) $fatal(1, "wrong motor address");
                if (received_count == 1) begin
                    case (data)
                        8'hfd: request_length <= 13;
                        8'hfe, 8'h93, 8'h9a: request_length <= 5;
                        8'h9c: request_length <= 4;
                        8'h1f, 8'h22, 8'h36, 8'h3a, 8'h3b: request_length <= 3;
                        default: $fatal(1, "unexpected motor function, including forbidden homing: %h", data);
                    endcase
                end
                if (received_count == request_length - 1 && received_count >= 2) begin
                    if (data != 8'h6b) $fatal(1, "bad request checksum");
                    command_count <= command_count + 1;
                    received_count <= 0;
                    request_length <= 3;
                    response_bytes[0] <= ADDRESS;
                    response_bytes[1] <= request_bytes[1];
                    response_bytes[2] <= 8'h02;
                    response_bytes[3] <= 8'h6b;
                    response_length <= 4;
                    case (request_bytes[1])
                        8'h1f: begin
                            response_bytes[2] <= 0;
                            response_bytes[3] <= 200 + ADDRESS;
                            response_bytes[4] <= 8'h42;
                            response_bytes[5] <= 8'h13;
                            response_bytes[6] <= 8'h6b;
                            response_length <= 7;
                        end
                        8'h22: begin
                            response_bytes[2] <= ADDRESS - 1;
                            response_bytes[3] <= ADDRESS == 2;
                            response_bytes[4] <= 0;
                            response_bytes[5] <= 20 * ADDRESS;
                            response_bytes[6] <= 0;
                            response_bytes[7] <= ADDRESS - 1;
                            response_bytes[8] <= 8'h27;
                            response_bytes[9] <= 8'h10;
                            response_bytes[10] <= 0;
                            response_bytes[11] <= 30;
                            response_bytes[12] <= 8'h03;
                            response_bytes[13] <= 8'he8;
                            response_bytes[14] <= 0;
                            response_bytes[15] <= 60;
                            response_bytes[16] <= 1;
                            response_bytes[17] <= 8'h6b;
                            response_length <= 18;
                        end
                        8'hfd: begin
                            $display("TRACE MOTOR %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                                request_bytes[0], request_bytes[1], request_bytes[2],
                                request_bytes[3], request_bytes[4], request_bytes[5],
                                request_bytes[6], request_bytes[7], request_bytes[8],
                                request_bytes[9], request_bytes[10], request_bytes[11], data);
                            last_move_direction <= request_bytes[2] != 0;
                            last_move_pulses <= {request_bytes[6], request_bytes[7], request_bytes[8], request_bytes[9]};
                            if ((request_bytes[10] != 1 && request_bytes[10] != 2) || request_bytes[11] != 0) $fatal(1, "incorrect move mode");
                            last_move_speed <= {request_bytes[3], request_bytes[4]};
                            if (request_bytes[5] != 100) $fatal(1, "incorrect default acceleration");
                            delta_position = {request_bytes[6], request_bytes[7], request_bytes[8], request_bytes[9]};
                            delta_position = delta_position * 65536 / 3200;
                            if (request_bytes[10] == 1) position <= request_bytes[2] != 0 ? -delta_position : delta_position;
                            else if (request_bytes[2] != 0) position <= position - delta_position;
                            else position <= position + delta_position;
                            move_count <= move_count + 1;
                            polls_left <= 3;
                        end
                        8'hfe: begin
                            if (request_bytes[2] != 8'h98 || request_bytes[3] != 0) $fatal(1, "bad stop frame");
                            stop_count <= stop_count + 1;
                            polls_left <= 0;
                        end
                        8'h9a: begin
                            if (request_bytes[2] != ADDRESS - 1 || request_bytes[3] != 0)
                                $fatal(1, "incorrect saved home mode or synchronization flag");
                            response_bytes[2] <= home_ack;
                            last_home_mode <= request_bytes[2];
                            if (home_ack == 8'h02) begin
                                home_count <= home_count + 1;
                                home_active <= 1;
                                home_polls_left <= 3;
                            end
                        end
                        8'h9c: begin
                            if (request_bytes[2] != 8'h48) $fatal(1, "bad homing interrupt frame");
                            home_interrupt_count <= home_interrupt_count + 1;
                            home_active <= 0;
                            home_polls_left <= 0;
                        end
                        8'h3b: begin
                            response_bytes[2] <= home_status;
                            if (home_active) begin
                                if (fail_home) response_bytes[2] <= 8'h0b;
                                else if (home_polls_left != 0 || hold_home) begin
                                    response_bytes[2] <= 8'h07;
                                    if (home_polls_left != 0) home_polls_left <= home_polls_left - 1'b1;
                                end else begin
                                    response_bytes[2] <= 8'h03;
                                    position <= 0;
                                    home_active <= 0;
                                end
                            end
                        end
                        8'h93: begin
                            if (ADDRESS != 1 || request_bytes[2] != 8'h88 || request_bytes[3] != 1)
                                $fatal(1, "incorrect persistent single-turn zero command");
                            response_bytes[2] <= zero_ack;
                            if (zero_ack == 8'h02) zero_count <= zero_count + 1;
                        end
                        8'h3a: begin
                            poll_count <= poll_count + 1;
                            response_bytes[2] <= stall ? 8'h0f : ((polls_left != 0 || hold_arrival) ? 8'h01 : 8'h03);
                            if (polls_left != 0) polls_left <= polls_left - 1'b1;
                        end
                        8'h36: begin
                            magnitude = position < 0 ? -position : position;
                            response_bytes[2] <= position < 0 ? 1 : 0;
                            response_bytes[3] <= magnitude[31:24];
                            response_bytes[4] <= magnitude[23:16];
                            response_bytes[5] <= magnitude[15:8];
                            response_bytes[6] <= magnitude[7:0];
                            response_bytes[7] <= 8'h6b;
                            response_length <= 8;
                        end
                    endcase
                    if (!drop_replies) response_pending <= 1;
                end else received_count <= received_count + 1'b1;
            end
            case (response_state)
                0: if (response_pending) begin
                    response_pending <= 0;
                    response_index <= 0;
                    response_state <= 1;
                end
                1: begin
                    tx_data <= response_bytes[response_index];
                    tx_start <= 1;
                    response_state <= 2;
                end
                2: if (tx_done) begin
                    if (response_index == response_length - 1) response_state <= 0;
                    else begin
                        response_index <= response_index + 1'b1;
                        response_state <= 1;
                    end
                end
            endcase
        end
    end
endmodule

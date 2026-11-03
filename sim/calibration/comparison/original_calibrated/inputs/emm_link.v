`timescale 1ns / 1ps
module emm_link #(
    parameter integer CLK_HZ = 50000000,
    parameter integer BAUD = 115200,
    parameter [7:0] ADDRESS = 8'h01,
    parameter integer RESPONSE_MS = 200
)(
    input wire clk,
    input wire reset_n,
    input wire request,
    input wire cancel,
    input wire [7:0] function_code,
    input wire direction,
    input wire absolute_position,
    input wire [31:0] pulses,
    input wire [15:0] speed,
    input wire [7:0] acceleration,
    input wire [7:0] home_mode,
    input wire rx,
    output wire tx,
    output reg busy,
    output reg done,
    output reg [3:0] error_code,
    output reg signed [31:0] result,
    output reg [119:0] home_parameters,
    output reg [31:0] move_tx_count,
    output reg [31:0] last_tx_pulses,
    output reg last_tx_direction
);
    localparam integer RESPONSE_TICKS = (CLK_HZ / 1000) * RESPONSE_MS;
    reg [103:0] packet;
    reg [3:0] packet_length;
    reg [3:0] packet_index;
    reg [1:0] send_state;
    reg send_complete;
    reg [7:0] expected_function;
    reg [4:0] expected_length;
    reg [4:0] receive_index;
    reg [119:0] receive_payload;
    reg response_received;
    reg cancelled;
    reg [31:0] timeout_count;
    reg uart_start;
    reg [31:0] pending_pulses;
    reg pending_direction;
    reg [7:0] uart_data;
    wire uart_done;
    wire [7:0] received_data;
    wire received_valid;
    wire received_error;
    uart_tx #(.CLK_HZ(CLK_HZ), .BAUD(BAUD)) transmitter(
        .clk(clk), .reset_n(reset_n), .start(uart_start), .data(uart_data),
        .tx(tx), .busy(), .done(uart_done)
    );
    uart_rx #(.CLK_HZ(CLK_HZ), .BAUD(BAUD)) receiver(
        .clk(clk), .reset_n(reset_n), .rx(rx), .data(received_data),
        .valid(received_valid), .frame_error(received_error)
    );
    always @(posedge clk) begin
        if (!reset_n) begin
            busy <= 0;
            done <= 0;
            error_code <= 0;
            result <= 0;
            home_parameters <= 0;
            move_tx_count <= 0;
            last_tx_pulses <= 0;
            last_tx_direction <= 0;
            pending_pulses <= 0;
            pending_direction <= 0;
            packet <= 0;
            packet_length <= 0;
            packet_index <= 0;
            send_state <= 0;
            send_complete <= 0;
            expected_function <= 0;
            expected_length <= 0;
            receive_index <= 0;
            receive_payload <= 0;
            response_received <= 0;
            cancelled <= 0;
            timeout_count <= 0;
            uart_start <= 0;
            uart_data <= 0;
        end else begin
            done <= 0;
            uart_start <= 0;
            if (!busy) begin
                if (request) begin
                    busy <= 1;
                    error_code <= 0;
                    result <= 0;
                    packet_index <= 0;
                    send_state <= 0;
                    send_complete <= 0;
                    receive_index <= 0;
                    receive_payload <= 0;
                    response_received <= 0;
                    cancelled <= cancel;
                    timeout_count <= 0;
                    expected_function <= function_code;
                    expected_length <= function_code == 8'h22 ? 18 :
                        (function_code == 8'h1f ? 7 : (function_code == 8'h36 ? 8 : 4));
                    case (function_code)
                        8'hfd: begin
                            pending_pulses <= pulses;
                            pending_direction <= direction;
                            packet <= {ADDRESS, 8'hfd, 7'b0, direction, speed, acceleration, pulses,
                                (absolute_position ? 8'h01 : 8'h02), 8'h00, 8'h6b};
                            packet_length <= 13;
                        end
                        8'hfe: begin
                            packet <= {ADDRESS, 8'hfe, 8'h98, 8'h00, 8'h6b, 64'b0};
                            packet_length <= 5;
                        end
                        default: begin
                            packet <= {ADDRESS, function_code, 8'h6b, 80'b0};
                            packet_length <= 3;
                        end
                        8'h93: begin
                            packet <= {ADDRESS, 8'h93, 8'h88, 8'h01, 8'h6b, 64'b0};
                            packet_length <= 5;
                        end
                        8'h9a: begin
                            packet <= {ADDRESS, 8'h9a, home_mode, 8'h00, 8'h6b, 64'b0};
                            packet_length <= 5;
                        end
                        8'h9c: begin
                            packet <= {ADDRESS, 8'h9c, 8'h48, 8'h6b, 72'b0};
                            packet_length <= 4;
                        end
                    endcase
                end
            end else begin
                if (cancel) cancelled <= 1;
                if (timeout_count < RESPONSE_TICKS) timeout_count <= timeout_count + 1'b1;
                if (!send_complete) begin
                    case (send_state)
                        0: begin
                            uart_data <= packet[103:96];
                            uart_start <= 1;
                            send_state <= 1;
                        end
                        1: if (uart_done) begin
                            if (packet_index == packet_length - 1) begin
                                send_complete <= 1;
                                if (expected_function == 8'hfd) begin
                                    move_tx_count <= move_tx_count + 1'b1;
                                    last_tx_pulses <= pending_pulses;
                                    last_tx_direction <= pending_direction;
                                end
                            end
                            else begin
                                packet <= {packet[95:0], 8'b0};
                                packet_index <= packet_index + 1'b1;
                                send_state <= 0;
                            end
                        end
                    endcase
                end
                if (received_error) receive_index <= 0;
                else if (received_valid && !response_received) begin
                    if (receive_index == 0) begin
                        if (received_data == ADDRESS) receive_index <= 1;
                    end else if (receive_index == 1) begin
                        if (received_data == expected_function) begin
                            receive_index <= 2;
                            receive_payload <= 0;
                        end else receive_index <= received_data == ADDRESS ? 1 : 0;
                    end else if (receive_index == expected_length - 1) begin
                        receive_index <= 0;
                        if (received_data != 8'h6b) begin
                            error_code <= 3;
                            response_received <= 1;
                        end else if (expected_function == 8'h22) begin
                            home_parameters <= receive_payload;
                            response_received <= 1;
                        end else if (expected_function == 8'h1f) begin
                            result <= receive_payload[31:0];
                            response_received <= 1;
                        end else if (expected_length == 8) begin
                            if (receive_payload[39:32] > 1 || receive_payload[31]) error_code <= 4;
                            else result <= receive_payload[32] ? -$signed(receive_payload[31:0]) : $signed(receive_payload[31:0]);
                            response_received <= 1;
                        end else if (expected_function == 8'hfd || expected_function == 8'hfe || expected_function == 8'h93 || expected_function == 8'h9a || expected_function == 8'h9c) begin
                            if (receive_payload[7:0] != 8'h9f) begin
                                result <= {24'b0, receive_payload[7:0]};
                                error_code <= receive_payload[7:0] == 8'h02 ? 0 : 1;
                                response_received <= 1;
                            end
                        end else begin
                            result <= {24'b0, receive_payload[7:0]};
                            response_received <= 1;
                        end
                    end else begin
                        receive_payload <= {receive_payload[111:0], received_data};
                        receive_index <= receive_index + 1'b1;
                    end
                end
                if (send_complete && (cancelled || cancel || response_received || timeout_count == RESPONSE_TICKS)) begin
                    busy <= 0;
                    done <= 1;
                    if (cancelled || cancel) error_code <= 5;
                    else if (!response_received) error_code <= 2;
                end
            end
        end
    end
endmodule

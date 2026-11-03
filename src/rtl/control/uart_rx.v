`timescale 1ns / 1ps
module uart_rx #(
    parameter integer CLK_HZ = 50000000,
    parameter integer BAUD = 115200
)(
    input wire clk,
    input wire reset_n,
    input wire rx,
    output reg [7:0] data,
    output reg valid,
    output reg frame_error
);
    localparam integer BIT_TICKS = (CLK_HZ + BAUD / 2) / BAUD;
    (* ASYNC_REG = "TRUE" *) reg rx_meta = 1'b1;
    (* ASYNC_REG = "TRUE" *) reg rx_sync = 1'b1;
    reg [1:0] state;
    reg [31:0] tick_count;
    reg [2:0] bit_index;
    reg [7:0] shift_data;
    always @(posedge clk) begin
        rx_meta <= rx;
        rx_sync <= rx_meta;
        if (!reset_n) begin
            state <= 0;
            tick_count <= 0;
            bit_index <= 0;
            shift_data <= 0;
            data <= 0;
            valid <= 0;
            frame_error <= 0;
        end else begin
            valid <= 0;
            frame_error <= 0;
            case (state)
                0: if (!rx_sync) begin
                    tick_count <= 0;
                    state <= 1;
                end
                1: if (tick_count == BIT_TICKS / 2 - 1) begin
                    tick_count <= 0;
                    if (!rx_sync) begin
                        bit_index <= 0;
                        state <= 2;
                    end else state <= 0;
                end else tick_count <= tick_count + 1'b1;
                2: if (tick_count == BIT_TICKS - 1) begin
                    tick_count <= 0;
                    shift_data[bit_index] <= rx_sync;
                    if (bit_index == 7) state <= 3;
                    else bit_index <= bit_index + 1'b1;
                end else tick_count <= tick_count + 1'b1;
                3: if (tick_count == BIT_TICKS - 1) begin
                    tick_count <= 0;
                    state <= 0;
                    if (rx_sync) begin
                        data <= shift_data;
                        valid <= 1;
                    end else frame_error <= 1;
                end else tick_count <= tick_count + 1'b1;
            endcase
        end
    end
endmodule

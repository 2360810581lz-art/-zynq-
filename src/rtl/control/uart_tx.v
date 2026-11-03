`timescale 1ns / 1ps
module uart_tx #(
    parameter integer CLK_HZ = 50000000,
    parameter integer BAUD = 115200
)(
    input wire clk,
    input wire reset_n,
    input wire start,
    input wire [7:0] data,
    output reg tx,
    output reg busy,
    output reg done
);
    localparam integer BIT_TICKS = (CLK_HZ + BAUD / 2) / BAUD;
    reg [31:0] tick_count;
    reg [3:0] bit_index;
    reg [9:0] frame;
    always @(posedge clk) begin
        if (!reset_n) begin
            tx <= 1'b1;
            busy <= 1'b0;
            done <= 1'b0;
            tick_count <= 0;
            bit_index <= 0;
            frame <= 10'h3ff;
        end else begin
            done <= 1'b0;
            if (!busy) begin
                tx <= 1'b1;
                if (start) begin
                    frame <= {1'b1, data, 1'b0};
                    tx <= 1'b0;
                    busy <= 1'b1;
                    tick_count <= 0;
                    bit_index <= 0;
                end
            end else if (tick_count == BIT_TICKS - 1) begin
                tick_count <= 0;
                if (bit_index == 9) begin
                    tx <= 1'b1;
                    busy <= 1'b0;
                    done <= 1'b1;
                end else begin
                    bit_index <= bit_index + 1'b1;
                    frame <= {1'b1, frame[9:1]};
                    tx <= frame[1];
                end
            end else tick_count <= tick_count + 1'b1;
        end
    end
endmodule

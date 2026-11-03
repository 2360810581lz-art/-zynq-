`timescale 1ns / 1ps
module serial_report #(
    parameter integer CLK_HZ = 50000000,
    parameter integer BAUD = 115200
)(
    input wire clk,
    input wire reset_n,
    input wire start,
    input wire [191:0] prefix,
    input wire [2:0] value_count,
    input wire [191:0] values,
    output reg busy,
    output reg done,
    output wire tx
);
    reg [191:0] prefix_shift;
    reg [4:0] prefix_remaining;
    reg [191:0] held_values;
    reg [2:0] held_count;
    reg [2:0] field_index;
    reg negative;
    reg [3:0] digit_index;
    reg [31:0] magnitude;
    reg [39:0] decimal_digits;
    reg [39:0] adjusted_digits;
    reg [5:0] conversion_step;
    reg [3:0] state;
    reg tx_start;
    reg [7:0] tx_data;
    wire tx_done;
    integer decimal_index;
    function [4:0] text_length;
        input [191:0] text;
        integer byte_index;
        begin
            text_length = 0;
            for (byte_index = 0; byte_index < 24; byte_index = byte_index + 1)
                if (text[byte_index * 8 +: 8] != 0) text_length = text_length + 1'b1;
        end
    endfunction
    always @(*) begin
        adjusted_digits = decimal_digits;
        for (decimal_index = 0; decimal_index < 10; decimal_index = decimal_index + 1)
            if (decimal_digits[decimal_index * 4 +: 4] >= 5)
                adjusted_digits[decimal_index * 4 +: 4] = decimal_digits[decimal_index * 4 +: 4] + 4'd3;
    end
    uart_tx #(.CLK_HZ(CLK_HZ), .BAUD(BAUD)) transmitter(
        .clk(clk), .reset_n(reset_n), .start(tx_start), .data(tx_data),
        .tx(tx), .busy(), .done(tx_done)
    );
    always @(posedge clk) begin
        if (!reset_n) begin
            state <= 0;
            busy <= 0;
            done <= 0;
            tx_start <= 0;
            tx_data <= 0;
            prefix_shift <= 0;
            prefix_remaining <= 0;
            held_values <= 0;
            held_count <= 0;
            field_index <= 0;
            negative <= 0;
            digit_index <= 0;
            magnitude <= 0;
            decimal_digits <= 0;
            conversion_step <= 0;
        end else begin
            tx_start <= 0;
            done <= 0;
            case (state)
                0: if (start) begin
                    prefix_shift <= prefix << ((24 - text_length(prefix)) * 8);
                    prefix_remaining <= text_length(prefix);
                    held_values <= values;
                    held_count <= value_count;
                    field_index <= 0;
                    busy <= 1;
                    state <= 1;
                end
                1: if (prefix_remaining == 0) state <= 3;
                    else begin
                        tx_data <= prefix_shift[191:184];
                        tx_start <= 1;
                        state <= 2;
                    end
                2: if (tx_done) begin
                    prefix_shift <= {prefix_shift[183:0], 8'b0};
                    prefix_remaining <= prefix_remaining - 1'b1;
                    state <= 1;
                end
                3: if (field_index == held_count) state <= 11;
                    else begin
                        negative <= held_values[31];
                        magnitude <= held_values[31] ? -held_values[31:0] : held_values[31:0];
                        decimal_digits <= 0;
                        conversion_step <= 0;
                        digit_index <= 0;
                        state <= 4;
                    end
                4: begin
                    decimal_digits <= {adjusted_digits[38:0], magnitude[31]};
                    magnitude <= {magnitude[30:0], 1'b0};
                    if (conversion_step == 31) state <= 5;
                    else conversion_step <= conversion_step + 1'b1;
                end
                5: begin
                    tx_data <= " ";
                    tx_start <= 1;
                    state <= 6;
                end
                6: if (tx_done) state <= 7;
                7: begin
                    tx_data <= negative ? "-" : "+";
                    tx_start <= 1;
                    state <= 8;
                end
                8: if (tx_done) state <= 9;
                9: begin
                    tx_data <= 8'd48 + {4'b0, decimal_digits[39:36]};
                    tx_start <= 1;
                    state <= 10;
                end
                10: if (tx_done) begin
                    decimal_digits <= {decimal_digits[35:0], 4'b0};
                    if (digit_index == 9) begin
                        held_values <= {32'b0, held_values[191:32]};
                        field_index <= field_index + 1'b1;
                        state <= 3;
                    end else begin
                        digit_index <= digit_index + 1'b1;
                        state <= 9;
                    end
                end
                11: begin
                    tx_data <= 8'h0d;
                    tx_start <= 1;
                    state <= 12;
                end
                12: if (tx_done) state <= 13;
                13: begin
                    tx_data <= 8'h0a;
                    tx_start <= 1;
                    state <= 14;
                end
                14: if (tx_done) begin
                    busy <= 0;
                    done <= 1;
                    state <= 0;
                end
                default: state <= 0;
            endcase
        end
    end
endmodule

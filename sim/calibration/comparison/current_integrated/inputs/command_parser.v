`timescale 1ns / 1ps
module manual_command_parser(
    input wire clk,
    input wire reset_n,
    input wire [7:0] data,
    input wire valid,
    input wire frame_error,
    output reg command_valid,
    output reg [4:0] command,
    output reg [2:0] argument_count,
    output reg signed [31:0] argument0
);
    reg [63:0] word_buffer;
    reg [3:0] word_length;
    reg argument_phase;
    reg invalid_line;
    reg number_started;
    reg number_digit;
    reg number_negative;
    reg [31:0] number_value;
    reg number_dot;
    reg [31:0] coordinate_group;
    reg [2:0] count_buffer;
    reg signed [31:0] first_argument;
    reg [7:0] line_length;
    wire delimiter = data == 8'h20 || data == 8'h09;
    wire line_end = data == 8'h0d || data == 8'h0a;
    wire [7:0] letter = (data >= "a" && data <= "z") ? data - 8'd32 : data;
    wire [35:0] next_number = {4'b0, number_value} * 36'd10 + {28'b0, data} - 36'd48;
    wire coordinate_command = decode(word_buffer) == 6 || decode(word_buffer) == 7;
    wire signed [31:0] completed_number = coordinate_command ?
        ((coordinate_group > 6 || (coordinate_group != 0 && number_value == 0) || number_value > 4) ? 32'd29 :
        (coordinate_group * 4 + number_value)) :
        (number_negative ? -$signed(number_value) : $signed(number_value));
    function [4:0] decode;
        input [63:0] name;
        begin
            case (name)
                "READY": decode = 1;
                "M1": decode = 2;
                "M2": decode = 3;
                "BASE": decode = 4;
                "GRIP": decode = 5;
                "SAVE": decode = 6;
                "SHOW": decode = 7;
                "LIST": decode = 8;
                "READ": decode = 9;
                "SPEED": decode = 10;
                "ACC": decode = 11;
                "STOP": decode = 12;
                "STATUS": decode = 13;
                "DIAG": decode = 14;
                "MINFO": decode = 15;
                "LIFT": decode = 16;
                "HLOG": decode = 17;
                "M1ZERO": decode = 18;
                "HOME": decode = 19;
                "1", "2", "3", "4", "5", "6": decode = 21;
                "OVER": decode = 22;
                "PROGRESS": decode = 23;
                default: decode = 0;
            endcase
        end
    endfunction
    always @(posedge clk) begin
        if (!reset_n) begin
            word_buffer <= 0;
            word_length <= 0;
            argument_phase <= 0;
            invalid_line <= 0;
            number_started <= 0;
            number_digit <= 0;
            number_negative <= 0;
            number_value <= 0;
            number_dot <= 0;
            coordinate_group <= 0;
            count_buffer <= 0;
            first_argument <= 0;
            line_length <= 0;
            command_valid <= 0;
            command <= 0;
            argument_count <= 0;
            argument0 <= 0;
        end else begin
            command_valid <= 0;
            if (frame_error) invalid_line <= 1;
            if (valid) begin
                if (line_end) begin
                    if (word_length != 0 || invalid_line) begin
                        command_valid <= 1;
                        command <= (invalid_line || (number_started && (!number_digit || (coordinate_command && !number_dot)))) ? 0 : decode(word_buffer);
                        argument_count <= count_buffer + (number_started ? 3'd1 : 3'd0);
                        argument0 <= decode(word_buffer) == 21 ? word_buffer[7:0] - 8'd48 : (count_buffer == 0 && number_digit) ?
                            completed_number : first_argument;
                    end
                    word_buffer <= 0;
                    word_length <= 0;
                    argument_phase <= 0;
                    invalid_line <= 0;
                    number_started <= 0;
                    number_digit <= 0;
                    number_negative <= 0;
                    number_value <= 0;
                    number_dot <= 0;
                    coordinate_group <= 0;
                    count_buffer <= 0;
                    first_argument <= 0;
                    line_length <= 0;
                end else begin
                    if (line_length == 127) invalid_line <= 1;
                    else line_length <= line_length + 1'b1;
                    if (!argument_phase) begin
                        if (delimiter) begin
                            if (word_length != 0) argument_phase <= 1;
                        end else if (word_length == 8) invalid_line <= 1;
                        else begin
                            word_buffer <= {word_buffer[55:0], letter};
                            word_length <= word_length + 1'b1;
                        end
                    end else if (delimiter) begin
                        if (number_started) begin
                            if (!number_digit || count_buffer != 0 || (coordinate_command && !number_dot)) invalid_line <= 1;
                            if (count_buffer == 0) first_argument <= completed_number;
                            count_buffer <= 1;
                            number_started <= 0;
                            number_digit <= 0;
                            number_negative <= 0;
                            number_value <= 0;
                            number_dot <= 0;
                            coordinate_group <= 0;
                        end
                    end else if (data >= "0" && data <= "9") begin
                        number_started <= 1;
                        number_digit <= 1;
                        if (number_dot && number_digit) invalid_line <= 1;
                        if (next_number > 36'd2147483647) invalid_line <= 1;
                        else number_value <= next_number[31:0];
                    end else if (data == "." && coordinate_command && number_digit && !number_dot && !number_negative) begin
                        number_dot <= 1;
                        coordinate_group <= number_value;
                        number_value <= 0;
                        number_digit <= 0;
                    end else if ((data == "-" || data == "+") && !number_started) begin
                        number_started <= 1;
                        number_negative <= data == "-";
                        if (coordinate_command) invalid_line <= 1;
                    end else invalid_line <= 1;
                end
            end
        end
    end
endmodule

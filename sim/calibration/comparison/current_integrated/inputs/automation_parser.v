`timescale 1ns / 1ps
module command_parser(
    input wire clk,
    input wire reset_n,
    input wire [7:0] data,
    input wire valid,
    input wire frame_error,
    output wire command_valid,
    output wire [4:0] command,
    output wire [2:0] argument_count,
    output wire signed [31:0] argument0,
    output wire signed [31:0] argument1
);
    wire manual_valid;
    wire [4:0] manual_command;
    wire [2:0] manual_count;
    wire signed [31:0] manual_argument;
    reg started;
    reg coordinate_line;
    reg invalid_line;
    reg [2:0] phase;
    reg negative;
    reg sign_seen;
    reg digit_seen;
    reg dot_seen;
    reg [1:0] fraction_digits;
    reg [31:0] magnitude;
    reg signed [31:0] first_coordinate;
    reg signed [31:0] second_coordinate;
    reg coordinate_valid;
    reg coordinate_good;
    reg signed [31:0] output_x;
    reg signed [31:0] output_y;
    wire space = data == " " || data == 9;
    wire line_end = data == 13 || data == 10;
    wire [35:0] next_magnitude = {4'b0, magnitude} * 36'd10 + data - 36'd48;
    wire [63:0] scaled_magnitude = !dot_seen ? {32'b0, magnitude} * 64'd100 :
        (fraction_digits == 1 ? {32'b0, magnitude} * 64'd10 : {32'b0, magnitude});
    wire signed [31:0] coordinate_value = negative ? -$signed(scaled_magnitude[31:0]) : $signed(scaled_magnitude[31:0]);
    manual_command_parser manual(
        .clk(clk), .reset_n(reset_n), .data(data), .valid(valid), .frame_error(frame_error),
        .command_valid(manual_valid), .command(manual_command),
        .argument_count(manual_count), .argument0(manual_argument)
    );
    assign command_valid = coordinate_valid || manual_valid;
    assign command = coordinate_valid ? (coordinate_good ? 5'd20 : 5'd0) : manual_command;
    assign argument_count = coordinate_valid ? 3'd2 : manual_count;
    assign argument0 = coordinate_valid ? output_x : manual_argument;
    assign argument1 = output_y;
    always @(posedge clk) begin
        if (!reset_n) begin
            started <= 0;
            coordinate_line <= 0;
            invalid_line <= 0;
            phase <= 0;
            negative <= 0;
            sign_seen <= 0;
            digit_seen <= 0;
            dot_seen <= 0;
            fraction_digits <= 0;
            magnitude <= 0;
            first_coordinate <= 0;
            second_coordinate <= 0;
            coordinate_valid <= 0;
            coordinate_good <= 0;
            output_x <= 0;
            output_y <= 0;
        end else begin
            coordinate_valid <= 0;
            if (frame_error) invalid_line <= 1;
            if (valid) begin
                if (line_end) begin
                    coordinate_valid <= coordinate_line;
                    coordinate_good <= !invalid_line && !frame_error && phase == 3;
                    output_x <= first_coordinate;
                    output_y <= second_coordinate;
                    started <= 0;
                    coordinate_line <= 0;
                    invalid_line <= 0;
                    phase <= 0;
                    negative <= 0;
                    sign_seen <= 0;
                    digit_seen <= 0;
                    dot_seen <= 0;
                    fraction_digits <= 0;
                    magnitude <= 0;
                end else if (!started && !space) begin
                    started <= 1;
                    coordinate_line <= data == "(";
                end else if (coordinate_line) begin
                    if ((phase == 0 || phase == 1) && data >= "0" && data <= "9") begin
                        digit_seen <= 1;
                        if ((dot_seen && fraction_digits == 2) || next_magnitude > 36'd100000000) invalid_line <= 1;
                        else magnitude <= next_magnitude[31:0];
                        if (dot_seen && fraction_digits < 2) fraction_digits <= fraction_digits + 1'b1;
                    end else if ((phase == 0 || phase == 1) && (data == "-" || data == "+") && !sign_seen && !digit_seen && !dot_seen) begin
                        negative <= data == "-";
                        sign_seen <= 1;
                    end else if ((phase == 0 || phase == 1) && data == "." && !dot_seen && digit_seen) begin
                        dot_seen <= 1;
                    end else if ((phase == 0 && data == ",") || (phase == 1 && data == ")")) begin
                        if (!digit_seen || (dot_seen && fraction_digits == 0) || scaled_magnitude > 100000000) invalid_line <= 1;
                        if (phase == 0) first_coordinate <= coordinate_value;
                        else second_coordinate <= coordinate_value;
                        phase <= phase == 0 ? 1 : 3;
                        magnitude <= 0;
                        negative <= 0;
                        sign_seen <= 0;
                        digit_seen <= 0;
                        dot_seen <= 0;
                        fraction_digits <= 0;
                    end else if (!space || ((phase == 0 || phase == 1) && (digit_seen || sign_seen || dot_seen))) invalid_line <= 1;
                end
            end
        end
    end
endmodule

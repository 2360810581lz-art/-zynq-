`timescale 1ns / 1ps
module coordinate_mapper(
    input wire clk,
    input wire reset_n,
    input wire start,
    input wire signed [31:0] x_centi_cm,
    input wire signed [31:0] y_centi_cm,
    output reg done,
    output reg [31:0] motor1_pulses,
    output reg [8:0] base_degrees
);
    `include "automatic_coordinates.vh"
    reg [2:0] state;
    reg signed [63:0] weight_first;
    reg signed [63:0] weight_second;
    reg [4:0] first_index;
    reg [4:0] second_index;
    reg signed [63:0] first_motor_product;
    reg signed [63:0] second_motor_product;
    reg signed [63:0] first_base_product;
    reg signed [63:0] second_base_product;
    reg signed [63:0] motor_sum;
    reg signed [63:0] base_sum;
    reg [21:0] clipped_motor;
    reg [18:0] clipped_base;
    reg [21:0] rounded_motor;
    reg [18:0] rounded_base;
    wire signed [63:0] coordinate_x = {{32{x_centi_cm[31]}}, x_centi_cm};
    wire signed [63:0] coordinate_y = {{32{y_centi_cm[31]}}, y_centi_cm};
    wire signed [63:0] absolute_x = coordinate_x < 0 ? -coordinate_x : coordinate_x;
    wire signed [63:0] absolute_y = coordinate_y < 0 ? -coordinate_y : coordinate_y;
    always @(posedge clk) begin
        if (!reset_n) begin
            state <= 0;
            done <= 0;
            motor1_pulses <= 0;
            base_degrees <= 0;
            weight_first <= 0;
            weight_second <= 0;
            first_index <= 0;
            second_index <= 0;
            first_motor_product <= 0;
            second_motor_product <= 0;
            first_base_product <= 0;
            second_base_product <= 0;
            motor_sum <= 0;
            base_sum <= 0;
            rounded_motor <= 0;
            rounded_base <= 0;
            clipped_motor <= 0;
            clipped_base <= 0;
        end else begin
            done <= 0;
            case (state)
                0: if (start) begin
                    if (absolute_y >= absolute_x) begin
                        first_index <= coordinate_y >= 0 ? 1 : 2;
                        second_index <= coordinate_y >= 0 ? 3 : 4;
                        weight_first <= absolute_y - coordinate_x;
                        weight_second <= absolute_y + coordinate_x;
                    end else begin
                        first_index <= coordinate_x >= 0 ? 3 : 1;
                        second_index <= coordinate_x >= 0 ? 4 : 2;
                        weight_first <= absolute_x + coordinate_y;
                        weight_second <= absolute_x - coordinate_y;
                    end
                    state <= 1;
                end
                1: begin
                    first_motor_product <= weight_first * (auto_motor1(first_index) - auto_motor1(0));
                    second_motor_product <= weight_second * (auto_motor1(second_index) - auto_motor1(0));
                    first_base_product <= weight_first * (auto_base(first_index) - auto_base(0));
                    second_base_product <= weight_second * (auto_base(second_index) - auto_base(0));
                    state <= 2;
                end
                2: begin
                    motor_sum <= 64'sd1000 * auto_motor1(0) + first_motor_product + second_motor_product;
                    base_sum <= 64'sd1000 * auto_base(0) + first_base_product + second_base_product;
                    state <= 3;
                end
                3: begin
                    clipped_motor <= motor_sum < 0 ? 0 : (motor_sum > 4000000 ? 4000000 : motor_sum[21:0]);
                    clipped_base <= base_sum < 0 ? 0 : (base_sum > 270000 ? 270000 : base_sum[18:0]);
                    state <= 4;
                end
                4: begin
                    rounded_motor <= clipped_motor + 22'd500;
                    rounded_base <= clipped_base + 19'd500;
                    state <= 5;
                end
                5: begin
                    motor1_pulses <= rounded_motor / 22'd1000;
                    base_degrees <= rounded_base / 19'd1000;
                    done <= 1;
                    state <= 0;
                end
                default: state <= 0;
            endcase
        end
    end
endmodule

`timescale 1ns / 1ps
module servo_pwm #(
    parameter integer CLK_HZ = 50000000,
    parameter integer FULL_ANGLE = 180,
    parameter integer MIN_ANGLE = 0,
    parameter integer LIMIT_ANGLE = 35,
    parameter integer BOOT_ANGLE = 35,
    parameter integer MIN_US = 500,
    parameter integer MAX_US = 2500,
    parameter integer PERIOD_US = 20000
)(
    input wire clk,
    input wire reset_n,
    input wire [8:0] angle,
    output reg pwm
);
    localparam integer US_TICKS = CLK_HZ / 1000000;
    localparam integer SAFE_BOOT = (BOOT_ANGLE > LIMIT_ANGLE) ? LIMIT_ANGLE :
        (BOOT_ANGLE < MIN_ANGLE ? MIN_ANGLE : BOOT_ANGLE);
    localparam integer BOOT_US = MIN_US + (SAFE_BOOT * (MAX_US - MIN_US) + FULL_ANGLE / 2) / FULL_ANGLE;
    // Use a fixed-point reciprocal instead of a run-time divider.  The
    // original constant division created a long angle->pulse combinational
    // path (and failed timing at 100 MHz).  Ten fractional bits keep the
    // pulse width error below one microsecond for the 180-degree gripper
    // and the 270-degree base servo.
    localparam integer SCALE_BITS = 10;
    localparam integer ANGLE_SCALE = (((MAX_US - MIN_US) * (1 << SCALE_BITS)) +
        FULL_ANGLE / 2) / FULL_ANGLE;
    reg [31:0] us_divider;
    reg [15:0] frame_us;
    reg [15:0] active_us;
    reg [15:0] requested_us_reg;
    wire [31:0] safe_angle = (angle > LIMIT_ANGLE) ? LIMIT_ANGLE :
        (angle < MIN_ANGLE ? MIN_ANGLE : angle);
    wire [23:0] scaled_product = safe_angle * ANGLE_SCALE;
    wire [15:0] calculated_us = MIN_US +
        ((scaled_product + (1 << (SCALE_BITS - 1))) >> SCALE_BITS);
    always @(posedge clk) begin
        if (!reset_n) pwm <= 0;
        else pwm <= frame_us < active_us;
    end
    always @(posedge clk) begin
        if (!reset_n) begin
            us_divider <= 0;
            frame_us <= 0;
            active_us <= BOOT_US;
            requested_us_reg <= BOOT_US;
        end else if (us_divider == US_TICKS - 1) begin
            us_divider <= 0;
            if (frame_us == PERIOD_US - 1) begin
                frame_us <= 0;
                active_us <= requested_us_reg;
            end else frame_us <= frame_us + 1'b1;
            requested_us_reg <= calculated_us;
        end else us_divider <= us_divider + 1'b1;
    end
endmodule

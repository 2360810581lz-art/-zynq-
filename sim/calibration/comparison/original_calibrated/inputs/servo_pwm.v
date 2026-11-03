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
    reg [31:0] us_divider;
    reg [15:0] frame_us;
    reg [15:0] active_us;
    wire [31:0] safe_angle = (angle > LIMIT_ANGLE) ? LIMIT_ANGLE :
        (angle < MIN_ANGLE ? MIN_ANGLE : angle);
    wire [31:0] requested_us = MIN_US + (safe_angle * (MAX_US - MIN_US) + FULL_ANGLE / 2) / FULL_ANGLE;
    always @(posedge clk) begin
        if (!reset_n) pwm <= 0;
        else pwm <= frame_us < active_us;
    end
    always @(posedge clk) begin
        if (!reset_n) begin
            us_divider <= 0;
            frame_us <= 0;
            active_us <= BOOT_US;
        end else if (us_divider == US_TICKS - 1) begin
            us_divider <= 0;
            if (frame_us == PERIOD_US - 1) begin
                frame_us <= 0;
                active_us <= requested_us[15:0];
            end else frame_us <= frame_us + 1'b1;
        end else us_divider <= us_divider + 1'b1;
    end
endmodule

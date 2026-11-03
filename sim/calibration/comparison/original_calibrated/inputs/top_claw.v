`timescale 1ns / 1ps
module top_claw #(
    parameter integer CLK_HZ = 50000000,
    // 180-degree gripper calibration: 0 degrees closed, 35 degrees open.
    parameter integer GRIPPER_CLOSED_ANGLE = 0,
    parameter integer GRIPPER_OPEN_ANGLE = 35
)(
    input wire clk_50m,
    input wire pc_rx,
    output wire pc_tx,
    input wire motor1_rx,
    output wire motor1_tx,
    input wire motor2_rx,
    output wire motor2_tx,
    output wire base_pwm,
    output wire gripper_pwm
);
    reg [15:0] reset_count = 0;
    wire reset_n = &reset_count;
    wire [8:0] base_angle;
    wire [8:0] gripper_angle;
    always @(posedge clk_50m)
        if (!reset_n) reset_count <= reset_count + 1'b1;
    claw_controller #(.CLK_HZ(CLK_HZ),
        .GRIPPER_CLOSED_ANGLE(GRIPPER_CLOSED_ANGLE),
        .GRIPPER_OPEN_ANGLE(GRIPPER_OPEN_ANGLE)) controller(
        .clk(clk_50m), .reset_n(reset_n), .pc_rx(pc_rx), .pc_tx(pc_tx),
        .motor1_rx(motor1_rx), .motor1_tx(motor1_tx),
        .motor2_rx(motor2_rx), .motor2_tx(motor2_tx),
        .base_angle(base_angle), .gripper_angle(gripper_angle)
    );
    servo_pwm #(.CLK_HZ(CLK_HZ), .FULL_ANGLE(270), .MIN_ANGLE(0), .LIMIT_ANGLE(270), .BOOT_ANGLE(45)) base_servo(
        .clk(clk_50m), .reset_n(reset_n), .angle(base_angle), .pwm(base_pwm)
    );
    servo_pwm #(.CLK_HZ(CLK_HZ), .FULL_ANGLE(180), .MIN_ANGLE(GRIPPER_CLOSED_ANGLE),
        .LIMIT_ANGLE(GRIPPER_OPEN_ANGLE), .BOOT_ANGLE(GRIPPER_OPEN_ANGLE)) gripper_servo(
        .clk(clk_50m), .reset_n(reset_n), .angle(gripper_angle), .pwm(gripper_pwm)
    );
endmodule

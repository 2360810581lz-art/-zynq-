`timescale 1ns / 1ps
module tb_automatic;
    reg clk = 0;
    always #500 clk = !clk;
    reg reset_n = 0;
    reg pc_rx = 1;
    wire pc_tx, motor1_tx, motor1_rx, motor2_tx, motor2_rx;
    wire [8:0] base_angle, gripper_angle;
    reg drop_motor1 = 0;
    reg stall_motor2 = 0;
    wire [7:0] reply_data;
    wire reply_valid;
    reg [1023:0] line_buffer = 0;
    reg [1023:0] last_line = 0;
    integer line_count = 0;
    integer assertions = 0;
    integer baseline;
    integer move_baseline;
    integer group_number;
    integer slot_number;
    integer counter_index;
    reg [1023:0] group_text;
    reg [1023:0] group_reply;
    `include "automatic_coordinates.vh"
    claw_controller #(.CLK_HZ(1000000), .BAUD(100000), .RESPONSE_MS(20),
        .POLL_MS(1), .MOTION_MS(100), .BASE_SETTLE_MS(3), .GRIP_SETTLE_MS(3),
        .STARTUP_WAIT_MS(20), .SERVO_STEP_MS(1)) dut(
        .clk(clk), .reset_n(reset_n), .pc_rx(pc_rx), .pc_tx(pc_tx),
        .motor1_rx(motor1_rx), .motor1_tx(motor1_tx),
        .motor2_rx(motor2_rx), .motor2_tx(motor2_tx),
        .base_angle(base_angle), .gripper_angle(gripper_angle));
    motor_model #(.ADDRESS(1)) motor1(.clk(clk), .reset_n(reset_n),
        .rx(motor1_tx), .tx(motor1_rx), .home_status(8'h03), .drop_replies(drop_motor1), .stall(1'b0));
    motor_model #(.ADDRESS(2)) motor2(.clk(clk), .reset_n(reset_n),
        .rx(motor2_tx), .tx(motor2_rx), .home_status(8'h03), .drop_replies(1'b0), .stall(stall_motor2));
    uart_rx #(.CLK_HZ(1000000), .BAUD(100000)) receiver(.clk(clk), .reset_n(reset_n),
        .rx(pc_tx), .data(reply_data), .valid(reply_valid), .frame_error());
    always @(posedge clk) begin
        if (reply_valid) begin
            if (reply_data == 10) begin
                last_line <= line_buffer;
                line_buffer <= 0;
                line_count <= line_count + 1;
                $display("PC: %0s", line_buffer);
            end else if (reply_data != 13) line_buffer <= {line_buffer[1015:0], reply_data};
        end
        if (reset_n) begin
            if (gripper_angle > 35) $fatal(1, "unsafe gripper output");
            if (dut.auto_active && dut.link_start != 0 && dut.link_function == 8'hfd) begin
                if (!dut.move_absolute || dut.speed != 60 || dut.acceleration != 100)
                    $fatal(1, "auto motor mode/speed/acceleration");
                case (dut.auto_step)
                    0: if (dut.axis != 1 || !dut.move_direction || dut.move_pulses != 1800)
                        $fatal(1, "lift must target absolute -1800");
                    1: if (dut.auto_kind == 0 || dut.axis != 0 || motor2.position != -36864)
                        $fatal(1, "drop must lift before extending");
                    2: if (dut.auto_kind != 0 || dut.axis != 0 || motor2.position != -36864 || base_angle != dut.target_base)
                        $fatal(1, "pick must rotate before extending at lift height");
                    3: if (dut.axis != 1 || base_angle != (dut.auto_kind == 0 ? dut.target_base : auto_base(dut.auto_coordinate)))
                        $fatal(1, "descent before rotation completed");
                    default: $fatal(1, "motor command at wrong auto step");
                endcase
                assertions = assertions + 1;
            end
            if (dut.auto_active && dut.state == 10 && dut.auto_step == 4 && gripper_angle != dut.servo_target) begin
                if (motor2.position != (dut.auto_kind == 1 ? 51200 : 24576))
                    $fatal(1, "gripper changes before target height");
            end
        end
    end
    task check;
        input condition;
        input [511:0] description;
        begin
            if (!condition) $fatal(1, "%0s", description);
            assertions = assertions + 1;
        end
    endtask
    task send_byte;
        input [7:0] value;
        integer bit_number;
        begin
            @(negedge clk);
            pc_rx = 0;
            repeat (10) @(negedge clk);
            for (bit_number = 0; bit_number < 8; bit_number = bit_number + 1) begin
                pc_rx = value[bit_number];
                repeat (10) @(negedge clk);
            end
            pc_rx = 1;
            repeat (10) @(negedge clk);
        end
    endtask
    task send_line;
        input [1023:0] text;
        integer byte_number;
        begin
            for (byte_number = 127; byte_number >= 0; byte_number = byte_number - 1)
                if (text[byte_number * 8 +: 8] != 0) send_byte(text[byte_number * 8 +: 8]);
            send_byte(13);
            send_byte(10);
        end
    endtask
    task expect_line;
        input integer previous_count;
        input [1023:0] expected;
        integer ticks;
        begin
            ticks = 0;
            while (line_count <= previous_count && ticks < 800000) begin
                @(negedge clk);
                ticks = ticks + 1;
            end
            if (line_count <= previous_count || last_line != expected)
                $fatal(1, "expected %0s got %0s state=%0d step=%0d", expected, last_line, dut.state, dut.auto_step);
            assertions = assertions + 1;
            repeat (50) @(negedge clk);
        end
    endtask
    task issue;
        input [1023:0] text;
        input [1023:0] expected;
        integer previous_count;
        begin
            previous_count = line_count;
            send_line(text);
            expect_line(previous_count, expected);
        end
    endtask
    task place;
        input integer destination;
        input integer expected_slot;
        integer coordinate;
        begin
            group_text = 8'd48 + destination;
            group_reply = {(8'd48 + destination[7:0]), " OK"};
            coordinate = destination * 4 + expected_slot;
            issue(group_text, group_reply);
            check(!dut.carrying && gripper_angle == 35, "release not complete at ACK");
            check(motor1.position == auto_motor1(coordinate) * 65536 / 3200 &&
                motor2.position == 51200 && base_angle == auto_base(coordinate), "wrong drop coordinate");
        end
    endtask

    reg [8:0] previous_base = 9'h1ff, previous_grip = 9'h1ff;
    always @(negedge clk) if (reset_n) begin
        if (base_angle != previous_base || gripper_angle != previous_grip) begin
            $display("TRACE SERVO base=%0d grip=%0d", base_angle, gripper_angle);
            previous_base = base_angle;
            previous_grip = gripper_angle;
        end
    end
    initial begin
        repeat (20) @(negedge clk);
        reset_n = 1;
        issue("(0,0)", "ERR NOT_READY");
        repeat (21000) @(negedge clk);
        check(motor1.command_count == 0 && motor2.command_count == 0 && gripper_angle == 35, "startup motion/default");
        issue("2", "ERR EXPECT_PICK");
        issue("(1.234,0)", "ERR SYNTAX");
        issue("(1,)", "ERR SYNTAX");
        issue("(1,2)junk", "ERR SYNTAX");
        issue("(99999999999999999,0)", "ERR SYNTAX");
        issue("(1000001,0)", "ERR SYNTAX");
        issue("2 1", "ERR SYNTAX");
        issue("7", "ERR SYNTAX");
        issue("(0.00,+0.00)", "0 OK");
        check(dut.carrying && motor1.position == 22528 && motor2.position == 24576 && base_angle == 135 && gripper_angle == 0,
            "wrong center pick or premature ACK");
        issue("(1,1)", "ERR EXPECT_DROP");
        place(2, 1);
        issue("(-5,5)", "0 OK");
        check(motor1.position == 4096 && base_angle == 100, "wrong calibrated pick");
        place(3, 1);
        issue("(8,4)", "0 OK");
        check(motor1.position == 11878 && base_angle == 184, "out-of-range coordinate was not extrapolated");
        place(2, 2);
        issue("(0,-20)", "0 OK");
        check(motor1.position == 81920, "motor1 upper saturation");
        place(2, 3);
        issue("(0,20)", "0 OK");
        check(motor1.position == 0, "motor1 lower saturation");
        place(2, 4);
        issue("(100,0)", "0 OK");
        check(base_angle == 270, "base upper saturation");
        place(2, 1);
        check(dut.drop_slot[2] == 1 && dut.drop_slot[3] == 1, "independent wrapping counters");
        issue("OVER", "OVER OK");
        check(motor1.position == 22528 && motor2.position == 24576 && base_angle == 135 && gripper_angle == 35,
            "OVER did not return to 0.0");
        for (counter_index = 1; counter_index <= 6; counter_index = counter_index + 1)
            check(dut.drop_slot[counter_index] == 0, "OVER did not reset counter");
        for (group_number = 1; group_number <= 6; group_number = group_number + 1)
            for (slot_number = 1; slot_number <= 4; slot_number = slot_number + 1) begin
                issue("(1.25,-2.50)", "0 OK");
                check(dut.map_x == 125 && dut.map_y == -250, "decimal coordinates parsed incorrectly");
                place(group_number, slot_number);
            end
        check(motor1.home_count == 0 && motor2.home_count == 0 &&
            motor1.zero_count == 0 && motor2.zero_count == 0, "origin changed");
        $display("PASS original_motion_trace assertions=%0d", assertions);
        $finish;
    end
    initial begin
        #(64'd30000000000);
        $fatal(1, "trace watchdog");
    end
endmodule

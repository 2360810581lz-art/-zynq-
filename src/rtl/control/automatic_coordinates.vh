function signed [31:0] auto_motor1;
    input [4:0] coordinate;
    begin
        case (coordinate)
            0: auto_motor1 = 32'sd1100;
            1: auto_motor1 = 32'sd200;
            2: auto_motor1 = 32'sd2500;
            3: auto_motor1 = 32'sd200;
            4: auto_motor1 = 32'sd2500;
            5: auto_motor1 = 32'sd1000;
            6: auto_motor1 = 32'sd1000;
            7: auto_motor1 = 32'sd300;
            8: auto_motor1 = 32'sd200;
            9: auto_motor1 = 32'sd1500;
            10: auto_motor1 = 32'sd2200;
            11: auto_motor1 = 32'sd1000;
            12: auto_motor1 = 32'sd1400;
            13: auto_motor1 = 32'sd2800;
            14: auto_motor1 = 32'sd3600;
            15: auto_motor1 = 32'sd2600;
            16: auto_motor1 = 32'sd3300;
            17: auto_motor1 = 32'sd300;
            18: auto_motor1 = 32'sd200;
            19: auto_motor1 = 32'sd1000;
            20: auto_motor1 = 32'sd1000;
            21: auto_motor1 = 32'sd1000;
            22: auto_motor1 = 32'sd1400;
            23: auto_motor1 = 32'sd1600;
            24: auto_motor1 = 32'sd2000;
            25: auto_motor1 = 32'sd2400;
            26: auto_motor1 = 32'sd3000;
            27: auto_motor1 = 32'sd2800;
            28: auto_motor1 = 32'sd3600;
            default: auto_motor1 = 0;
        endcase
    end
endfunction
function signed [31:0] auto_motor2;
    input [4:0] coordinate;
    begin
        case (coordinate)
            0: auto_motor2 = 32'sd1200;
            1: auto_motor2 = 32'sd1200;
            2: auto_motor2 = 32'sd1200;
            3: auto_motor2 = 32'sd1200;
            4: auto_motor2 = 32'sd1200;
            5: auto_motor2 = 32'sd2500;
            6: auto_motor2 = 32'sd2500;
            7: auto_motor2 = 32'sd2500;
            8: auto_motor2 = 32'sd2500;
            9: auto_motor2 = 32'sd2500;
            10: auto_motor2 = 32'sd2500;
            11: auto_motor2 = 32'sd2500;
            12: auto_motor2 = 32'sd2500;
            13: auto_motor2 = 32'sd2500;
            14: auto_motor2 = 32'sd2500;
            15: auto_motor2 = 32'sd2500;
            16: auto_motor2 = 32'sd2500;
            17: auto_motor2 = 32'sd2500;
            18: auto_motor2 = 32'sd2500;
            19: auto_motor2 = 32'sd2500;
            20: auto_motor2 = 32'sd2500;
            21: auto_motor2 = 32'sd2500;
            22: auto_motor2 = 32'sd2500;
            23: auto_motor2 = 32'sd2500;
            24: auto_motor2 = 32'sd2500;
            25: auto_motor2 = 32'sd2500;
            26: auto_motor2 = 32'sd2500;
            27: auto_motor2 = 32'sd2500;
            28: auto_motor2 = 32'sd2500;
            default: auto_motor2 = 0;
        endcase
    end
endfunction
function signed [31:0] auto_base;
    input [4:0] coordinate;
    begin
        case (coordinate)
            0: auto_base = 32'sd135;
            1: auto_base = 32'sd100;
            2: auto_base = 32'sd117;
            3: auto_base = 32'sd170;
            4: auto_base = 32'sd153;
            5: auto_base = 32'sd38;
            6: auto_base = 32'sd57;
            7: auto_base = 32'sd37;
            8: auto_base = 32'sd60;
            9: auto_base = 32'sd75;
            10: auto_base = 32'sd88;
            11: auto_base = 32'sd85;
            12: auto_base = 32'sd95;
            13: auto_base = 32'sd100;
            14: auto_base = 32'sd103;
            15: auto_base = 32'sd103;
            16: auto_base = 32'sd108;
            17: auto_base = 32'sd233;
            18: auto_base = 32'sd210;
            19: auto_base = 32'sd233;
            20: auto_base = 32'sd213;
            21: auto_base = 32'sd185;
            22: auto_base = 32'sd175;
            23: auto_base = 32'sd195;
            24: auto_base = 32'sd185;
            25: auto_base = 32'sd170;
            26: auto_base = 32'sd161;
            27: auto_base = 32'sd175;
            28: auto_base = 32'sd170;
            default: auto_base = 0;
        endcase
    end
endfunction

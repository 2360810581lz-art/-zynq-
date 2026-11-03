localparam [28:0] DEFAULT_VALID = 29'b00000000000000000000000000000;
function signed [31:0] default_motor1;
    input integer coordinate;
    begin
        case (coordinate)
            default: default_motor1 = 0;
        endcase
    end
endfunction
function signed [31:0] default_motor2;
    input integer coordinate;
    begin
        case (coordinate)
            default: default_motor2 = 0;
        endcase
    end
endfunction
function [8:0] default_base;
    input integer coordinate;
    begin
        case (coordinate)
            default: default_base = 45;
        endcase
    end
endfunction

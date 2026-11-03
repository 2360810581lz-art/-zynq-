`timescale 1ns/1ps
// Keep completion reports intact while allowing read-only queries during motion.
// One pending foreground report; repeated queries coalesce to the latest snapshot.
module pc_report_mux #(
    parameter integer CLK_HZ = 50000000,
    parameter integer BAUD = 115200
)(
    input wire clk, reset_n,
    input wire start,
    input wire [191:0] prefix,
    input wire [2:0] value_count,
    input wire [191:0] values,
    input wire query_start,
    input wire [191:0] query_prefix,
    input wire [191:0] query_values,
    output wire busy, done, tx
);
    reg main_pending, query_pending;
    reg [191:0] main_prefix, main_values, held_query_prefix, held_query_values;
    reg [2:0] main_count;
    reg tx_start, active_query;
    reg [191:0] tx_prefix, tx_values;
    reg [2:0] tx_count;
    wire tx_busy, tx_done;
    assign busy = main_pending || start || tx_start || tx_busy;
    // A query's LF must never advance AUTO_REPORT_WAIT or LIST_WAIT.
    assign done = tx_done && !active_query;
    serial_report #(.CLK_HZ(CLK_HZ), .BAUD(BAUD)) reporter(
        .clk(clk), .reset_n(reset_n), .start(tx_start), .prefix(tx_prefix),
        .value_count(tx_count), .values(tx_values), .busy(tx_busy),
        .done(tx_done), .tx(tx));
    always @(posedge clk) begin
        if (!reset_n) begin
            main_pending <= 0; query_pending <= 0;
            main_prefix <= 0; main_values <= 0; main_count <= 0;
            held_query_prefix <= 0; held_query_values <= 0;
            tx_start <= 0; active_query <= 0;
            tx_prefix <= 0; tx_values <= 0; tx_count <= 0;
        end else begin
            tx_start <= 0;
            if (start) begin
                main_prefix <= prefix; main_values <= values;
                main_count <= value_count; main_pending <= 1;
            end
            if (query_start) begin
                held_query_prefix <= query_prefix; held_query_values <= query_values;
                query_pending <= 1;
            end
            if (!tx_busy && !tx_start && !start) begin
                if (main_pending) begin
                    tx_prefix <= main_prefix; tx_values <= main_values;
                    tx_count <= main_count; active_query <= 0;
                    main_pending <= 0; tx_start <= 1;
                end else if (query_pending) begin
                    tx_prefix <= held_query_prefix; tx_values <= held_query_values;
                    tx_count <= 6; active_query <= 1;
                    query_pending <= query_start; tx_start <= 1;
                end
            end
        end
    end
endmodule

// delay test
`define DELAY_TEST 1'b1
    `ifdef DELAY_TEST
        `define RAN_DELAY 1'b1
        // `define VAR_DELAY 4'b101
    `endif


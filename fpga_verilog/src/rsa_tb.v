`timescale 1ns / 1ps

module rsa_tb;

    logic clk;
    logic rst;
    logic start;
    logic [15:0] plaintext;
    logic [15:0] public_exp;
    logic [15:0] modulus;
    logic [15:0] ciphertext;
    logic done;

    // Instantiate DUT
    rsa_encryptor dut (
        .clk(clk),
        .rst(rst),
        .start(start),
        .plaintext(plaintext),
        .public_exp(public_exp),
        .modulus(modulus),
        .ciphertext(ciphertext),
        .done(done)
    );

    // Clock generation
    always #5 clk = ~clk;

    initial begin
        // Initialize
        clk = 0;
        rst = 1;
        start = 0;
        plaintext = 16'd0;
        public_exp = 16'd0;
        modulus = 16'd0;

        // Hold reset
        #20;
        rst = 0;

        // Apply inputs
        plaintext = 16'd7;      // M
        public_exp = 16'd5;     // e
        modulus = 16'd187;      // n

        #10;
        start = 1;              // Start encryption
        #10;
        start = 0;              // Let FSM take over

        // Wait for done
        wait (done == 1);
        $display("Plaintext: %0d", plaintext);
        $display("Ciphertext: %0d", ciphertext); // should be 168

        if (ciphertext == 16'd168)
            $display("RSA encryption test PASSED");
        else
            $display("RSA encryption test FAILED");

        $finish;
    end

endmodule

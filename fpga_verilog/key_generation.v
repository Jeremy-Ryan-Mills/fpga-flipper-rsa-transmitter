module key_generation(
    input clk,
    input reset,
    output reg [63:0] p,         // First prime number
    output reg [63:0] q,         // Second prime number
    output reg [127:0] phi       // Value of (p-1) * (q-1)
);

    // Internal signals
    reg [63:0] candidate_p;      // Candidate for prime p
    reg [63:0] candidate_q;      // Candidate for prime q
    reg prime_p;                 // Flag indicating if candidate_p is prime
    reg prime_q;                 // Flag indicating if candidate_q is prime

    // Random number generator instances
    wire [63:0] rand_p_wire;
    wire [63:0] rand_q_wire;

    random_number_generator rand_gen_p (
        .clk(clk),
        .reset(reset),
        .rand(rand_p_wire)
    );

    random_number_generator rand_gen_q (
        .clk(clk),
        .reset(reset),
        .rand(rand_q_wire)
    );

    // Primality check modules for p and q
    wire [63:0] a_p;              // Random base for Fermat test (p)
    wire [63:0] a_q;              // Random base for Fermat test (q)
    assign a_p = (candidate_p > 2) ? 2 : 1;  // Simple base selection
    assign a_q = (candidate_q > 2) ? 2 : 1;  // Simple base selection

    is_prime_fermat prime_check_p (
        .n(candidate_p),
        .a(a_p),
        .prime(prime_p)
    );

    is_prime_fermat prime_check_q (
        .n(candidate_q),
        .a(a_q),
        .prime(prime_q)
    );

    // State machine for generating primes
    reg [1:0] state;
    localparam IDLE = 2'b00, GENERATE_P = 2'b01, GENERATE_Q = 2'b10, COMPLETE = 2'b11;

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            state <= IDLE;
            p <= 0;
            q <= 0;
            phi <= 0;
            candidate_p <= 0;
            candidate_q <= 0;
        end else begin
            case (state)
                IDLE: begin
                    state <= GENERATE_P;
                end
                GENERATE_P: begin
                    if (prime_p) begin
                        p <= candidate_p;
                        state <= GENERATE_Q;
                    end else begin
                        candidate_p <= rand_p_wire;
                    end
                end
                GENERATE_Q: begin
                    if (prime_q) begin
                        q <= candidate_q;
                        state <= COMPLETE;
                    end else begin
                        candidate_q <= rand_q_wire;
                    end
                end
                COMPLETE: begin
                    phi <= (p - 1) * (q - 1);
                end
            endcase
        end
    end
endmodule


// Generates a random 64 bit integer using LFSR
module random_number_generator (
    input clk,              // Clock input
    input reset,            
    output reg [63:0] rand 
);

    reg [63:0] lfsr;  // 64-bit LFSR register

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            lfsr <= 64'habcdefabcdef1234; // Initial seed value
        end else begin
            // LFSR feedback equation for generating random bits
            lfsr <= {lfsr[62:0], lfsr[63] ^ lfsr[60]}; // Example feedback taps
        end
    end

    always @(posedge clk) begin
        rand <= lfsr;
    end
endmodule


// Determines whether n is prime given a random base a using Fermat's Little Theorem
module is_prime_fermat(
	input [63:0] n,
	input [63:0] a,
	output reg prime // 1 if n is prime, 0 if not
);

	reg [63:0] result;
	
	// Instantiate mod_exp
	mod_exp mod_exp_inst (
		.base(a),
		.exponent(n - 1),
		.mod(n),
		.result(result)
	);
	
	always @(*) begin
		if (result == 1) begin
			prime = 1;
		end else begin
			prime = 0;
		end
	end
endmodule
		
	

module key_generation(
	input clk,
	input reset,
	output [63:0] p,
	output [63:0] q,
	output [127:0] phi
);
	
	reg [63:0] rand;
	reg [63:0] a;
	reg prime;
	
	random_number_generator rand_gen(
		.clk(clk),
		.reset(reset)
		.rand(rand)
	);
	
	// Function to generate random a in range (1 < a < p-1)
	function [63:0] generate_a(input [63:0] p);
		reg [63:0] a;
		begin
			// Generate random value in range 1 to p-1
			random_number_generator(rand_gen, clk, reset, a);
			if (a >= p) begin
				generate_a = a - p;
			end else if (a == 0) begin
				generate_a = 1;
			end else begin
				generate_a = a;
			end
		end
	endfunction

	
	/*
	always @(*) begin
		while (~p_bit_reg) begin
			random_number_generator(clk, reset, rand);
			is_prime_fermat(rand, a, prime);
			if (prime) begin
				p_bit_reg = 1;
				p = rand;
			end
		end
		
		while (~q_bit_reg) begin
			random_number_generator(clk, reset, rand);
			is_prime_fermat(rand, a, prime);
			if (prime) begin
				q_bit_reg = 1;
				q = rand;
			end
		end
	*/
		reg [63:0] rand;
	reg [63:0] a;
	reg prime;
	
	random_number_generator rand_gen(
		.clk(clk),
		.reset(reset)
		.rand(rand)
	);
	
	always @(posedge clk or posedge reset) begin
		if (reset) begin
			p <= 0;
			q <= 0;
			phi <= 0;
		end else begin
			if (!p) begin
				while (!prime) begin
					random_number_generator(clk, reset, rand);
					is_prime_fermat(rand, a, prime);
				end
				p <= rand;
			end
		
			if (!q) begin
				while (!prime) begin
					random_number_generator(clk, reset, rand);
					is_prime_fermat(rand, a, prime);
				end
				q <= rand;
			end
			
			phi <= (p - 1) * (q - 1);
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
	output reg prime, // 1 if n is prime, 0 if not
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
		
	

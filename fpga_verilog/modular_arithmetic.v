// Modular multiplication with 32-bit inputs
module mod_mult(input [127:0] a, b, input [127:0] mod, output reg [31:0] result);
	reg [255:0] temp;
	always @(*) begin
		temp = a * b;
		result = temp % mod;
	end
endmodule


// Modular exponentiation with 32-bit inputs with successive squaring 
module mod_exp(input [127:0] base, exp, mod, output reg [127:0] result);
	reg [127:0] temp_result, base_temp;
	reg [127:0] exp_temp;
	always @(*) begin
		temp_result = 1;
		base_temp = base % mod;
		exp_temp = exp;
		while(exp_temp > 0) begin
			if(exp_temp[0] == 1) begin // If the current exp is odd
				temp_result = (temp_result * base_temp) % mod;
			end
			// If the current exp is even, square the base and halve the exp
			base_temp = (base_temp * base_temp) % mod;
			exp_temp = exp_temp >> 1;
		end
		result = temp_result;
	end
endmodule


// Montgomery reduction for value A mod N
module montgomery_reduction(
	input [127:0] A,
	input [127:0] N,
	input [127:0] N_prime, // -N^-1 mod R
	output reg [127:0] result
);
	// R = 2^127 for 128 bit-width
	reg [127:0] m;
	reg [255:0] t;
	
	always @(*) begin 
		t = A;
		
		// Compute m = (A * N_prime) mod R
		m = (A * N_prime) & ((1 << 127) - 1); // Truncating to lower 512 bits
		
		// Compute t = (t + m * N) / R
		t = t + (m * N);
		t = t >> 127; // Divide by R == right shift by 512 bits
		
		if (t >= N)
			result = t - N;
		else
			result = t;
	end
endmodule


// Computes N', where N * N' = -1 (mod R) using Extended Euclidean Algorithm
module compute_n_prime #(parameter WIDTH = 128)(
	input [WIDTH-1:0] N,
	output reg [WIDTH-1:0] N_prime
);
	reg [WIDTH-1:0] R;
	reg [WIDTH-1:0] x, y, temp;
	integer i;
	
	initial begin
		R = 1 << WIDTH; // R = 2^WIDTH
		x = 0;
		y = 1;
		temp = R;
		
		for (i = 0; i < WIDTH; i = i + 1) begin
			if (temp[0] == 1) begin
				temp = temp + N;
				y = y + x;
			end
			temp = temp >> 1;
			x = x >> 1;
		end
		
		N_prime = (~x) + 1;
	end
endmodule
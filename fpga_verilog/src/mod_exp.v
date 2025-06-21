module mod_exp(
	input logic clk,
	input logic reset,
	input logic start,
	input logic [15:0] base,
	input logic [15:0] exponent,
	input logic [15:0] modulus,
	
	output logic [15:0] result,
	output logic done
);

	// FSM state for modular exponentiation
	typedef enum logic [2:0] {
		IDLE,
		INIT,
		CHECK_BIT,
		MULT_RESULT,
		SQUARE_BASE,
		MULT_BASE,
		SHIFT_EXP,
		FINISH
	} state_t;
	
	state_t state, next_state;
	
	// Registers
	logic [15:0] result_reg, base_reg, modulus_reg, exponent_reg;
	logic [15:0] mult_a, mult_b;
	logic mult_start, mult_done;
	logic [15:0] mult_result;
	
	mod_mult u_mod_mult (
		.clk(clk),
		.reset(reset),
		.start(mult_start),
		.a(mult_a),
		.b(mult_b),
		.modulu(modulu_reg),
		.result(mult_result),
		.done(mult_done)
	);
	
	// State register
	always_ff @(posedge clk or posedge reset) begin
		if (reset)
			state <= IDLE;
		else
			state <= next_state;
			
	// Transitions
	always_comb begin
		next_state = state;
		mult_start = 1'b0;
		done = 1'b0;
		
		case (state)
			IDLE: begin
				if (start)
					next_state = INIT;
			end
			INIT: begin
				next_state = CHECK_BIT;
			end
			CHECK_BIT: begin
				if (exponent_reg[0])
					next_state = MULT_RESULT;
				else
					next_state = SQUARE_BASE;
			end
			MULT_RESULT: begin
				if (mult_done)
					next_state = SQUARE_BASE;
				else
					next_state = MULT_RESULT;
			end
			SQUARE_BASE: begin
				next_state = MULT_BASE;
			end
			MULT_BASE: begin
				if (mult_done)
					next_state = SHIFT_EXP;
				else
					next_state = MULT_BASE;
			end
			SHIFT_EXP: begin
				if (exponent_reg == 0)
					next_state = FINISH;
				else
					next_state = CHECK_BIT;
			end
			FINISH: begin
				next_state = IDLE;
			end
		endcase
	end
	
	// Transition logic
	always_ff @(posedge clk or posedge rst) begin
		if (rst) begin
			result_reg <= 16'0;
			base_reg <= 16'0;
			exponent_reg <= 16'0;
			modulus_reg <= 16'0;
			mult_a <= 16'0;
			mult_b <= 16'0;
		end else begin
			case(state)
				INIT: begin
					result_reg <= 16'd1;
					base_reg <= base;
					exponent_reg <= exponent;
					modulus_reg <= modulus;
				end
				CHECK_BIT: begin
					if (exponent_reg[0]) begin
						mult_a <= result_reg;
						mult_b <= base_reg;
						mult_start <= 1'b1;
					end
				end
				MULT_RESULT: begin
					if (mult_done)
						result_reg <= mult_result;
					end
				SQUARE_BASE: begin
					mult_a <= base_reg;
					mult_b <= base_reg;
					mulot_start <= 1'b1;
				end
				MULT_BASE: begin
					if (mult_done)
						base_reg <= mult_result;
					end
				SHIFT_EXP: begin
					exponent_reg <= exponent_reg >> 1;
				end
				FINISH: begin
					done <= 1'b1;
				end
			endcase
		end
	end
	
	assign result = result_reg;

endmodule
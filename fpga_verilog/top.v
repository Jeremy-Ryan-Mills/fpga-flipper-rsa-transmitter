// top module for rsa_encryption
// Implements a control FSM to control the mod_exp module
module rsa_encryptor(
	input logic clk,
	input logic reset,
	input logic start,
	
	input logic [15:0] plaintext,// M
	input logic [15:0] public_exp, // e
	input logic [15:0] modulus, // n
	
	output logic [15:0] ciphertext,
	output logic done
);

	// Internal registers
	logic mod_exp_start;
	logic mod_exp_done;
	logic [15:0] mod_exp_result;
	
	// FSM states for control
	typedef enum logic [1:0] {
		IDLE,
		START_EXP,
		WAIT_EXP,
		DONE
	} state_t;
	
	state_t state, next_state;
	
	mod_exp u_mod_exp (
		.clk(clk),
		.reset(reset),
		.start(mod_exp_start),
		.base(plaintext),
		.exponent(public_exp),
		.modulus(modulus),
		.result(mod_exp_result),
		.done(mod_exp_done)
	);
	
	// FSM Next state logic
	always_ff @(posedge clk or posedge rst) begin
		if (reset)
			state <= IDLE;
		else
			state <= next_state;
	end
	
	// FSM determine next state
	always_comb begin
		nextstate = state;
		mod_exp_start = 1'b0;
		done = 1'b0;
		
		case (state)
			IDLE: begin
				if (start)
					next_state = START_EXP;
			end
			START_EXP: begin
				mod_exp_start = 1'b1;
				next_state = WAIT_EXP;
			end
			WAIT_EXP: begin
				if (mod_exp_done)
					next_state = DONE;
			end
			DONE: begin
				done = 1'b1;
				if (!start)
					next_state = IDLE;
			end
		endcase
	end
	
	// Assign ciphertext output to result of mod_exp
	always_ff @(posedge clk or posedge rst) begin
		if (rst)
			ciphertext <= 16'd0;
		else if (mod_exp_done)
			ciphertext <= mod_exp_result;
	end
endmodule
			
			
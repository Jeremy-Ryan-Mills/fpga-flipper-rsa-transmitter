module rsa_encrypt(
	input clk,
	input reset,
	input [127:0] plaintext,
	input [63:0] e,
	input [127:0] n,
	output reg [127:0] encryptedtext
);

	reg [127:0] temp;
	wire [127:0] mod_exp_result;
	
	mod_exp mod_exp_inst(
		.base(plaintext),
		.exponent(e),
		.modulus(n),
		.result(mod_exp_result)
	);
	
	always @(posedge clk or posedge reset) begin
		if (reset) begin
			encryptedtext <= 128'b0;
		end else begin
			encryptedtext <= mod_exp_result;
		end
	end
endmodule



module rsa_decrypt(
	input clk,
	input reset,
	input [127:0] encryptedtext,
	input [63:0] d,
	input [127:0] n,
	output reg [127:0] decryptedtext
);
	reg[127:0] temp;
	wire [127:0] mod_exp_result;
	
	mod_exp mod_exp_inst(
		.base(encryptedtext),
		.exponent(d),
		.modulus(n),
		.result(mod_exp_result)
	);
	
	always @(posedge clk or posedge reset) begin
		if (reset) begin
			decryptedtext <= 128'b0;
		end else begin
			decryptedtext <= mod_exp_result;
		end
	end
endmodule


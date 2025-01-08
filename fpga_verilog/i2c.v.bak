// Receives i2c communication and gets the message
module i2c_receiver(
	input clk,
	input reset,
	input i2c_sda, // Data line
	input i2c_scl, // Clock line
	output reg [127:0] message,
	output reg message_ready
	output reg [7:0] slave_address, // Should be FPGA address
	output reg read_write, // Read/Write 
	outpute reg ack_received
);

	reg [7:0] byte_buffer;
	reg [6:0] bit_count;
	reg [3:0] byte_count;
	reg [127:0] msg_temp;
	reg [7:0] addr_buffer;
	reg address_received;
	
	always @(posedge clk or posedge reset) begin
		if (reset) begin
			bit_count <= 0;
			byte_count <= 0;
			byte_buffer <= 0;
			msg_temp <= 0;
			message <= 0;
			message_ready <= 0;
			slave_address <= 0;
			read_write <= 0;
			ack_received <= 0;
			address_received <= 0;
			
		end else begin
			if (i2c_scl == 1) begin
			
				byte_buffer[bit_count] <= i2c_sda;
				
				if (bit_count == 7) begin // One byte received
					// If no address received, first byte is the address
					if (address_received == 0) begin
						addr_buffer <= byte_buffer;
						slave_address <= addr_buffer[7:1];
						read_write <= addr_buffer[0];
						address_received <= 1;
						ack_received <= i2c_sda;
						
					end else begin // Add bytes into the message
						msg_temp <= {msg_temp[119:0], byte_buffer};
						byte_count <= byte_count + 1;
					end
					
					bit_count <= 0; // Reset for next byte
				end
			end
			
			// If 16 bytes worth of message have been received
			if (byte_count == 15) begin
				message <= msg_temp;
				message_ready <= 1;
			end
		end
	end
endmodule


// Transmits a message across sda and scl
module i2c_transmitter(
    input clk,             // System clock
    input reset,           // Reset signal
    inout sda,             // I2C data line (bi-directional)
    input scl,             // I2C clock line
    input [6:0] address,   // 7-bit I2C address
    input [127:0] send_data,  // Data to send
    input send,            // Trigger signal to start sending
    input rw,              // Read/Write flag (0 for write, 1 for read)
    output reg ack_received // Acknowledgment from receiver
);

    reg [7:0] data_byte;    // A byte of data to be sent
    reg [3:0] bit_count;    // Bit counter for sending data
    reg [7:0] byte_count;   // Byte counter
    reg transmitting;       // Flag to indicate if transmission is in progress
    reg sda_reg;            // Register to hold the value of SDA pin

    // Set the direction of the SDA pin (output or input)
    assign sda = (transmitting && !bit_count) ? sda_reg : 1'bz;  // During transmission, drive SDA line
    assign ack_received = (bit_count == 8) && transmitting;  // Acknowledge received at the end of byte transmission

    // State machine to handle the transmission process
    always @(posedge clk or posedge reset) begin
        if (reset) begin
            transmitting <= 1'b0;
            byte_count <= 0;
            bit_count <= 0;
            ack_received <= 1'b0;
        end
        else begin
            if (send && !transmitting) begin
                transmitting <= 1'b1;
                byte_count <= 0;
                bit_count <= 0;
                data_byte <= {address, rw};  // Send address + R/W bit as the first byte
            end

            if (transmitting) begin
                if (bit_count < 8) begin
                    // Shift data byte bit by bit
                    sda_reg <= data_byte[7];      // Output the MSB of the byte
                    data_byte <= data_byte << 1;  // Shift the byte for the next bit
                    bit_count <= bit_count + 1;
                end
                else if (bit_count == 8) begin
                    // After sending 8 bits (1 byte), check for acknowledgment
                    // Acknowledge signal is typically sent on the 9th clock pulse
                    if (sda == 1'b0) begin // If receiver pulls SDA low, we got ACK
                        ack_received <= 1'b1;
                    end
                    else begin
                        ack_received <= 1'b0;
                    end

                    if (byte_count < 16) begin
                        byte_count <= byte_count + 1;
                        data_byte <= send_data[127 - byte_count*8 : 120 - byte_count*8];
                        bit_count <= 0; // Reset bit counter for next byte
                    end
                    else begin
                        // All bytes sent, end transmission
                        transmitting <= 1'b0;
                    end
                end
            end
        end
    end
endmodule

					
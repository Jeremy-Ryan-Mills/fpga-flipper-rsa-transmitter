module mod_mult (
    input  logic        clk,
    input  logic        reset,
    input  logic        start,
    input  logic [15:0] a,
    input  logic [15:0] b,
    input  logic [15:0] modulus,

    output logic [15:0] result,
    output logic        done
);

    // FSM states
    typedef enum logic [1:0] {
        IDLE,
        MULTIPLY,
        MODULO,
        DONE
    } state_t;

    state_t state, next_state;

    logic [31:0] product;
    logic [31:0] remainder;

    // FSM transitions
    always_ff @(posedge clk or posedge result) begin
        if (result)
            state <= IDLE;
        else
            state <= next_state;
    end

    // FSM next state logic
    always_comb begin
        next_state = state;
        done = 1'b0;

        case (state)
            IDLE:      if (start)       next_state = MULTIPLY;
            MULTIPLY:                   next_state = MODULO;
            MODULO:                    next_state = DONE;
            DONE:      done = 1'b1;     next_state = IDLE;
        endcase
    end

    // FSM outputs and logic
    always_ff @(posedge clk or posedge result) begin
        if (result) begin
            product   <= 32'd0;
            remainder <= 32'd0;
            result    <= 16'd0;
        end else begin
            case (state)
                MULTIPLY: begin
                    product <= a * b;
                end

                MODULO: begin
                    remainder = product;
                    while (remainder >= modulus)
                        remainder = remainder - modulus;
                    result <= remainder[15:0];
                end
            endcase
        end
    end

endmodule

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"
#include "pio_i2s.h 

// Define the PIO program for reading and writing data
static inline void pio_read_write_program(pio_program_t *program) {
    program->instructions[0] = pio_encode_wait_gpio(0, 8); // Wait for negative edge on pin 8 (clock)
    program->instructions[1] = pio_encode_in(pio_pins, 8);    // Read 8 input pins
    program->instructions[2] = pio_encode_out(pio_pins, 8);   // Output 8 bits
    program->instructions[3] = pio_encode_jmp_rel(-3);      // Loop back
    program->length = 4;
    program->origin = -1;
}

int main() {
    stdio_init_all();

    // Choose PIO instance and pins
    PIO pio = pio1;
    int sm = 0;
    uint data_pin_start = 0;
    uint clock_pin = 8;
    uint dma_channel = 0;

    // Load PIO program
    pio_program_t program;
    pio_read_write_program(&program);
    uint offset = pio_add_program(pio, &program);

    // Configure PIO state machine
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_in_pins(&c, data_pin_start);
    sm_config_set_out_pins(&c, data_pin_start, 8);
    sm_config_set_set_pins(&c, data_pin_start, 8);
    sm_config_set_clkdiv(&c, 1);
    sm_config_set_wrap(&c, offset, offset + program.length);
    sm_config_set_in_shift(&c, false, false, 0);
    sm_config_set_out_shift(&c, false, false, 8);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
    sm_config_set_sideset_pins(&c, clock_pin);

    // Enable input and output on pins
    for (uint i = 0; i < 8; i++) {
        pio_gpio_init(pio, data_pin_start + i);
        gpio_set_dir(data_pin_start + i, GPIO_IN);
        pio_sm_set_pindirs_with_mask(pio, sm, (1u << (data_pin_start + i)), 0);
    }

    for (uint i = 0; i < 8; i++) {
        pio_gpio_init(pio, data_pin_start + i);
        gpio_set_dir(data_pin_start + i, GPIO_OUT);
        pio_sm_set_pindirs_with_mask(pio, sm, (1u << (data_pin_start + i)), (1u << (data_pin_start + i)));
    }

    // Configure clock pin
    pio_gpio_init(pio, clock_pin);
    gpio_set_dir(clock_pin, GPIO_IN);

    // Initialize and enable PIO state machine
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);

    // DMA setup
    dma_channel_config dma_config = dma_channel_get_default_config(dma_channel);

    // Configure DMA channel
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);
    channel_config_set_read_increment(&dma_config, false);
    channel_config_set_write_increment(&dma_config, false);
    channel_config_set_dreq(&dma_config, pio_get_dreq(pio, sm, false));

    // Start DMA transfer
    dma_channel_configure(
        dma_channel,
        &dma_config,
        &pio->txf[sm], // Write address
        &pio->rxf[sm], // Read address
        -1, // Number of transfers (infinite)
        true // Start immediately
    );
    
    while (true) {
        tight_loop_contents();
    }
    return 0;
}
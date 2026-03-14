#pragma once

// STM32F4 boards do not implement the H7 audio pipeline, but health packets
// still expose a sound output level. Keep the stubbed sound state in the sound
// subsystem rather than the config header.
uint16_t sound_output_level = 0U;

void sound_tick(void) {
}

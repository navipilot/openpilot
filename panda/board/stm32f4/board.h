#pragma once

#include "board/boards/board_declarations.h"
#include "board/boards/unused_funcs.h"
#include "board/stm32f4/lladc.h"
#include "board/drivers/harness.h"
#include "board/drivers/fan.h"
#include "board/stm32f4/llfan.h"
#include "board/stm32f4/clock_source.h"
#include "board/boards/dos.h"

void sound_tick(void) {}
uint16_t sound_output_level = 0U;

void detect_board_type(void) {
  if (!detect_with_pull(GPIOB, 1, PULL_UP) && !detect_with_pull(GPIOB, 7, PULL_UP)) {
    hw_type = HW_TYPE_DOS;
    current_board = &board_dos;
  } else {
    hw_type = HW_TYPE_UNKNOWN;
  }
  set_gpio_alternate(GPIOA, 13, GPIO_AF0_SWJ);
}

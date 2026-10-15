#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#define STM32F413xx
#define STM32F4
#define NUM_INTERRUPTS 102U
#define APB2_TIMER_FREQ 96U
#include "board/stm32f4/inc/stm32f4xx.h"
#include "board/stm32f4/inc/stm32f4xx_hal_gpio_ex.h"
#include "board/drivers/drivers.h"

static TIM_TypeDef timer1, timer8;
static GPIO_TypeDef gpioa, gpiob;
#undef TIM1
#undef TIM8
#undef GPIOA
#undef GPIOB
#define TIM1 (&timer1)
#define TIM8 (&timer8)
#define GPIOA (&gpioa)
#define GPIOB (&gpiob)
#define NVIC_DisableIRQ(irq) ((void)(irq))

static uint8_t pin15_af;
static void set_gpio_alternate(GPIO_TypeDef *gpio, uint8_t pin, uint8_t af) {
  if ((gpio == GPIOB) && (pin == 15U)) pin15_af = af;
}
void register_set(volatile uint32_t *addr, uint32_t val, uint32_t mask) {
  *addr = (*addr & ~mask) | val;
}
void register_set_bits(volatile uint32_t *addr, uint32_t val) {
  *addr |= val;
}

#ifdef TEST_H7_CLOCK
#include "board/drivers/clock_source.h"
#else
#include "board/stm32f4/clock_source.h"
#endif

int main(void) {
  clock_source_init(false);
  assert(TIM1->PSC == 9599U);
  assert(TIM1->ARR == 499U);
  assert(TIM1->CCR1 == 20U);
  assert(TIM1->CCR2 == 20U);
#ifdef TEST_H7_CLOCK
  assert(pin15_af == GPIO_AF3_TIM8);
  assert(TIM1->CCR4 == 250U);
  assert(TIM8->CCR3 == 20U);
  assert(TIM8->SMCR == (0b0100U << TIM_SMCR_SMS_Pos));
#else
  assert(pin15_af == GPIO_AF1_TIM1);
  assert(TIM1->CCR3 == 20U);
  assert(TIM8->CR1 == 0U);
#endif
  clock_source_set_timer_params(0x0304U, 0x0532U);
  assert(TIM1->CCR1 == 30U);
  assert(TIM1->CCR2 == 40U);
  assert(TIM1->ARR == 499U);
#ifdef TEST_H7_CLOCK
  assert(TIM8->CCR3 == 50U);
  assert(TIM1->CCR4 == 250U);
#else
  assert(TIM1->CCR3 == 50U);
#endif
  return 0;
}

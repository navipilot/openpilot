#pragma once

typedef struct {
  ADC_TypeDef *adc;
  uint8_t channel;
} adc_signal_t;

#define ADC_CHANNEL_DEFAULT(a, c) {.adc = (a), .channel = (c)}

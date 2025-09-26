#include <string.h>
#include <stdio.h>
#include <stdint.h>

extern unsigned int _sbss, _ebss, _sidata, _sdata, _edata;
static void wdt_disable(void);
#define REG(x) (*(volatile uint32_t *)(x))
#define BIT(x) ((uint32_t)1U << (x))
#define C3_LOWPOWER             0x60008000
#define C3_TIMERGRP0            0x6001F000
#define C3_TIMERGRP1            0x60020000

#define LED_GPIO               2 // GPIO pin for the inbuilt LED
#define GPIO_OUT_W1TS_REG      0x3FF44008  // GPIO register for setting the output high
#define GPIO_OUT_W1TC_REG      0x3FF4400C  // GPIO register for setting the output low
#define GPIO_ENABLE_W1TS_REG   0x3FF44024  // GPIO register for enabling the output

volatile uint32_t *gpio_enable_reg = (volatile uint32_t *)GPIO_ENABLE_W1TS_REG;
volatile uint32_t *gpio_out_w1ts_reg = (volatile uint32_t *)GPIO_OUT_W1TS_REG;
volatile uint32_t *gpio_out_w1tc_reg = (volatile uint32_t *)GPIO_OUT_W1TC_REG;

void intialize_led(void)
{
    *gpio_enable_reg = 1 << LED_GPIO; // Enable GPIO2 as output
}

void delay(long long delayed) {
    for (volatile long long i = 0; i < delayed; ++i)
        for (volatile int k = 0; k < 1000; ++k)
            ;
}

void toggle_led(void)
{
    *gpio_out_w1ts_reg = 1 << LED_GPIO; // Set the GPIO pin low
    delay(200);
    *gpio_out_w1tc_reg = 1 << LED_GPIO; // Set the GPIO pin high
    delay(6000);
}

int main( void ) {

    intialize_led();
    while ( 1 ) {
      toggle_led();
    }
    return 0;
}

const int size_sdata = sizeof( _sdata );

// Application entry point / startup logic.
void __attribute__( ( noreturn ) ) call_start_cpu0() {
  // Clear BSS.
  memset( &_sbss, 0, ( &_ebss - &_sbss ) * sizeof( _sbss ) );
  // Copy initialized data.
  // to be fixed
//  memset( &_sdata, &_sidata, ( &_edata - &_sdata ) * sizeof( _sdata ) );
//  for (unsigned int *dst = &_sdata, *src = &_sidata; dst < &_edata;) *dst++ = *src++;

  // Done, branch to main
  wdt_disable();
  main();
  // (Should never be reached)
  while( 1 ) {}
}

static inline void wdt_disable(void)
{
    REG(C3_LOWPOWER + 0xA8) = 0x50D83AA1;   // Disable Write Protection.
    // REG(C3_RTCCNTL)[36] &= BIT(31);    // Disable RTC WDT
    REG(C3_LOWPOWER + 0x90) = 0; // Disable RTC WDT
    REG(C3_LOWPOWER + 0x8C) = 0;
//  REG(C3_LOWPOWER + 0xA8) = 0xABCDEF00;   // Enable Write Protection.

    // bootloader_super_wdt_auto_feed()
    REG(C3_LOWPOWER + 0xB0) = 0x8F1D312A;   // Disable Write Protection.
    REG(C3_LOWPOWER + 0xAC) |= BIT(31); // Auto feed
//  REG(C3_LOWPOWER + 0xB4) = 0; // RTC_CNTL_SW_CPU_STALL_REG
//  REG(C3_LOWPOWER + 0xB0) = 0xABCDEF00;   // Enable Write Protection.

    REG(C3_TIMERGRP0 + 0x64) = 0x50D83AA1;  // Disable Write Protection.
    REG(C3_TIMERGRP0 + 0xFC) &= ~BIT(29);
    REG(C3_TIMERGRP0 + 0x48) = 0; // Disable TG0 WDT
//  REG(C3_TIMERGRP0 + 0x64) = 0xABCDEF00;  // Enable Write Protection.

    REG(C3_TIMERGRP1 + 0x64) = 0x50D83AA1;  // Disable Write Protection.
    REG(C3_TIMERGRP1 + 0xFC) &= ~BIT(29);
    REG(C3_TIMERGRP1 + 0x48) = 0; // Disable TG1 WDT
//  REG(C3_TIMERGRP1 + 0x64) = 0xABCDEF00;  // Enable Write Protection.
}

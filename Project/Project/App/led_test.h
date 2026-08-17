#ifndef __LED_TEST_H__
#define __LED_TEST_H__

// 配置 PB9 为 LED 输出（默认熄灭）
void pb9_led_init(void);

// 点亮 / 熄灭 LED
void pb9_led_on(void);
void pb9_led_off(void);

// 翻转 LED（PB9）
void pb9_led_toggle(void);

#endif  /* __LED_TEST_H__ */

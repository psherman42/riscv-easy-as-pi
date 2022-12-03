#ifndef GPIO_H__INCLUDED
#define GPIO_H__INCLUDED


void gpio_pin_en (uint8_t g, uint8_t x);
void gpio_pin_sel (uint8_t g, uint8_t x);

//void gpio_init (void);
//void gpio_high (unsigned int ra);
//void gpio_low (unsigned int ra);
//unsigned int gpio_read (unsigned int ra);


enum gpio_dir_t { GPIO_IN, GPIO_OUT };
enum gpio_iof_t { GPIO_GPIO, GPIO_UART, GPIO_SPI, GPIO_I2C, GPIO_PWM };

void gpio_init (void);

uint8_t gpio_read (uint8_t g);
void gpio_high (uint8_t g);
void gpio_low (uint8_t g);
void gpio_dir (uint8_t g, enum gpio_dir_t x);
void gpio_iof (uint8_t g, enum gpio_iof_t x);

#endif//GPIO_H__INCLUDED

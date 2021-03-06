/*
 *
 */
#ifndef __MJD_H__
#define __MJD_H__

#include <float.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

// This header file is generated by 'make menuconfig' and derived from the config file 'sdkconfig'
#include "sdkconfig.h"

#include "esp_event_loop.h"
#include "esp_clk.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_spi_flash.h"
#include "esp_system.h"


#include "cJSON.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/rmt.h"
#include "driver/uart.h"
#include "esp_http_client.h"
#include "mbedtls/base64.h"
#include "nvs_flash.h"
#include "soc/rmt_reg.h"
#include "soc/soc.h"

#ifdef __cplusplus
extern "C" {
#endif

/**********
 *  Definitions for errors
 *  @deprecated Use esp_err_t instead of mjd_err_t
 *  @deprecated Use ESP_OK instead of MJD_OK
 *  @deprecated Use ESP_FAIL instead of MJD_ERROR
 *  @deprecated Use ESP_ERR_* instead of MJD_ERR_* if a matching error code exists (e.g. MJD_ERR_INVALID_ARG => ESP_ERR_INVALID_ARG)
 */
/////typedef int32_t mjd_err_t;
/////#define MJD_OK     (0)
/////#define MJD_ERROR  (-1)
#define MJD_ERR_CHECKSUM            (0x101)
#define MJD_ERR_INVALID_ARG         (0x102)
#define MJD_ERR_INVALID_DATA        (0x103)
#define MJD_ERR_INVALID_RESPONSE    (0x104)
#define MJD_ERR_INVALID_STATE       (0x105)
#define MJD_ERR_NOT_FOUND           (0x106)
#define MJD_ERR_NOT_SUPPORTED       (0x107)
#define MJD_ERR_REGEXP              (0x108)
#define MJD_ERR_TIMEOUT             (0x109)
#define MJD_ERR_IO                  (0x110)

#define MJD_ERR_ESP_GPIO            (0x201)
#define MJD_ERR_ESP_RMT             (0x203)
#define MJD_ERR_ESP_RTOS            (0x204)
#define MJD_ERR_ESP_SNTP            (0x205)
#define MJD_ERR_ESP_WIFI            (0x206)

#define MJD_ERR_LWIP                (0x301)
#define MJD_ERR_NETCONN             (0x302)

/**********
 * C Language: utilities, stdlib, etc.
 */
/*
 * ARRAY_SIZE - get the number of elements in array @arr
 * @arr: array to be sized
 */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/**********
 * BOOLEAN: printf
 */
#define MJDBOOLEANFMT "%s"
#define MJDBOOLEAN2STR(a) (a ? "true" : "false")

/**********
 * INTEGERS: BINARY SEARCH
 */
int mjd_compare_ints(const void * a, const void * b);

/**********
 * BYTES/WORDS and BINARY REPRESENTATION
 *
 * @doc Packed BCD = 4 bits per digit. This encodes two decimal digits within a single byte by taking advantage of the fact that four bits are enough to represent the range 0 to 9.
 * @doc https://en.wikipedia.org/wiki/Binary-coded_decimal
 *
 */
#define MJD_HIBYTE(x) ((uint8_t)((uint16_t)(x) >> 8))
#define MJD_LOBYTE(x) ((uint8_t)(x))
#define MJD_HIWORD(x) ((uint16_t)((uint32_t)(x) >> 16))
#define MJD_LOWORD(x) ((uint16_t)((uint32_t)(x)))

/*
 * Convert uint8_t to binary coded decimal
 */
uint8_t mjd_byte_to_bcd(uint8_t val);

/*
 * Convert binary coded decimal to uint8_t
 * @doc 4 bits for each decimal digit.
 */
uint8_t mjd_bcd_to_byte(uint8_t val);

esp_err_t mjd_byte_to_binary_string(uint8_t input_byte, char * output_string);
esp_err_t mjd_word_to_binary_string(uint16_t input_word, char * output_string);

/**********
 * STRINGS
 */
bool mjd_string_starts_with(const char *str, const char *pre);
bool mjd_string_ends_with(const char *str, const char *post);
char * mjd_string_repeat(const char * s, int n);
void mjd_string_prepend(char* param_ptr_string, const char* param_ptr_part);

/**********
 * HEX STRINGS
 */

/*
 * @example [0 0x00, 1 0x01, 14 0x0e, 15 0x0f, 240 0xf0, 241 0xf1, 254 0xfe, 255 0xff] => "00010E0FF0F1FEFF"
 * @dep param_ptr_output must point to an array of chars TWICE the size of the param_ptr_input buffer PLUS 1
 */
esp_err_t mjd_uint8s_to_hexstring(const uint8_t * param_ptr_input, size_t param_len_input, char * param_ptr_output);

/*
 * @example "00010E0FF0F1FEFF" = [0 0x00, 1 0x01, 14 0x0e, 15 0x0f, 240 0xf0, 241 0xf1, 254 0xfe, 255 0xff] => "00010E0FF0F1FEFF"
 * @dep param_ptr_output must point to an array of uint8 at least HALF the size of the param_ptr_input buffer PLUS 1
 */
esp_err_t mjd_hexstring_to_uint8s(const char * param_ptr_input, size_t param_len_input, uint8_t * param_ptr_output);

/*
 * @example "ABC" => "414243"
 * @dep param_ptr_output must point to an array of chars TWICE the size of the param_ptr_input buffer PLUS 1
 */
esp_err_t mjd_string_to_hexstring(const char * param_ptr_input, size_t param_len_input, char * param_ptr_output);

/*
 * @example "414243" => "ABC"
 * @dep param_ptr_output must point to an array of chars at least HALF the size of the param_ptr_input buffer PLUS 1
 */
esp_err_t mjd_hexstring_to_string(const char * param_ptr_input, size_t param_len_input, char * param_ptr_output);

/**********
 * CRYPTO
 */
esp_err_t mjd_crypto_xor_cipher(const uint8_t param_key, uint8_t* param_ptr_values, const size_t param_values_len);

/**********
 * DATE TIME
 * @doc unsigned int (uint32_t on ESP32) Maximum value: 4294967295
 */
#define SECONDS_PER_DAY               86400
#define SECONDS_FROM_1970_TO_2000 946684800

/*
 * @brief Enforce a delay in milliseconds
 *
 *  @param param_millisec delay in ms
 *
 *  @important For small delays (<= 500 millisec) use ets_delay_us(). This is a blocking func.
 *  @important For long delays  (>  500 millisec) use vTaskDelay(). Also avoidd that the CPU Watchdog is triggered (x seconds).
 *
 *  @important RTOS func vTaskDelay() is not accurate
 *                  1. Lowest allowed delay is 10 milliseconds;
 *                  2. Does not work accurately for values < 500millisec;
 *
 */
void mjd_delay_millisec(uint32_t param_millisec);

uint32_t mjd_seconds_to_milliseconds(uint32_t seconds);
uint32_t mjd_seconds_to_microseconds(uint32_t seconds);
void mjd_log_time();
void mjd_set_timezone_utc();
void mjd_set_timezone_amsterdam();

/*
 * @brief
 *
 * @example char current_time[20];
 *          mjd_get_current_time_yyyymmddhhmmss(current_time);
 *
 */
void mjd_get_current_time_yyyymmddhhmmss(char *ptr_buffer);

/**********
 * Network helpers
 */

// Mac Address helper for printf
#define MJDMACFMT "%02X:%02X:%02X:%02X:%02X:%02X"
#define MJDMAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]

/**********
 * RTOS vTaskDelay() parameter
 *
 * @important Do NOT use vTaskDelay() for time periods smnaller than 10 millisec (use ets_delay_us() instead!)
 */
#define RTOS_DELAY_0             (0)
#define RTOS_DELAY_1MILLISEC     (   1 / portTICK_PERIOD_MS)
#define RTOS_DELAY_5MILLISEC     (   5 / portTICK_PERIOD_MS)
#define RTOS_DELAY_10MILLISEC    (  10 / portTICK_PERIOD_MS)
#define RTOS_DELAY_25MILLISEC    (  25 / portTICK_PERIOD_MS)
#define RTOS_DELAY_50MILLISEC    (  50 / portTICK_PERIOD_MS)
#define RTOS_DELAY_75MILLISEC    (  75 / portTICK_PERIOD_MS)
#define RTOS_DELAY_100MILLISEC   ( 100 / portTICK_PERIOD_MS)
#define RTOS_DELAY_125MILLISEC   ( 125 / portTICK_PERIOD_MS)
#define RTOS_DELAY_150MILLISEC   ( 150 / portTICK_PERIOD_MS)
#define RTOS_DELAY_200MILLISEC   ( 200 / portTICK_PERIOD_MS)
#define RTOS_DELAY_250MILLISEC   ( 250 / portTICK_PERIOD_MS)
#define RTOS_DELAY_500MILLISEC   ( 500 / portTICK_PERIOD_MS)
#define RTOS_DELAY_1SEC          ( 1 * 1000 / portTICK_PERIOD_MS)
#define RTOS_DELAY_2SEC          ( 2 * 1000 / portTICK_PERIOD_MS)
#define RTOS_DELAY_3SEC          ( 3 * 1000 / portTICK_PERIOD_MS)
#define RTOS_DELAY_5SEC          ( 5 * 1000 / portTICK_PERIOD_MS)
#define RTOS_DELAY_6SEC          ( 6 * 1000 / portTICK_PERIOD_MS)
#define RTOS_DELAY_10SEC         (10 * 1000 / portTICK_PERIOD_MS)
#define RTOS_DELAY_15SEC         (15 * 1000 / portTICK_PERIOD_MS)
#define RTOS_DELAY_30SEC         (30 * 1000 / portTICK_PERIOD_MS)
#define RTOS_DELAY_1MINUTE       ( 1 * 60 * 1000 / portTICK_PERIOD_MS)
#define RTOS_DELAY_5MINUTES      ( 5 * 60 * 1000 / portTICK_PERIOD_MS)
#define RTOS_DELAY_15MINUTES     (15 * 60 * 1000 / portTICK_PERIOD_MS)
#define RTOS_DELAY_MAX           (portMAX_DELAY)

// Use this priority to create all your RTOS tasks so all tasks are ginve equal cputime using the Round Robin method
//      @doc https://www.freertos.org/RTOS-task-priority.html
#define RTOS_TASK_PRIORITY_NORMAL (5)

void mjd_rtos_wait_forever();

/**********
 * ESP32 SYSTEM
 */
void mjd_log_clanguage_details();
void mjd_log_chip_info();
void mjd_log_wakeup_details();

typedef struct {
    uint32_t free_esp_heap;
    uint32_t free_rtos_stack;
} mjd_meminfo_t;

/**********
 * ESP32 SYSTEM & RTOS: Memory
 *
 * @doc uxTaskGetStackHighWaterMark()
 *   Set xTask to NULL to check the stack of the calling RTOS task.
 *   High water mark is the minimum free stack space there has been (***in bytes rather than words as found in vanilla FreeRTOS***)
 *   since the task started. The smaller the returned number the closer the task has come to overflowing its stack.
 *
 * @doc esp_get_free_heap_size() returns available heap size, in bytes.
 *   Note that the returned value may be larger than the maximum contiguous block which can be allocated.
 */
esp_err_t mjd_get_memory_statistics(mjd_meminfo_t* data);
esp_err_t mjd_log_memory_statistics();

/**********
 * ESP32: BOOT INFO, DEEP SLEEP and WAKE UP
 */
uint32_t mjd_increment_mcu_boot_count();
void mjd_log_mcu_boot_count();
uint32_t mjd_get_mcu_boot_count();
void mjd_log_wakeup_details();

/**********
 * ESP32 cJSON
 */
/*
 * cJSON_PrintPreallocated(cJSON *item, char *buffer, const int length, const cJSON_bool format)
 *     @note cJSON is not always 100% accurate in estimating how much memory it will use, so be safe and allocate 5 bytes more than you actually need.
 */
#define MJD_CJSON_PRINT_FORMATTED true
#define MJD_CJSON_PRINT_UNFORMATTED false


/**********
 * ESP32: LED
 */
#define HUZZAH32_GPIO_NUM_LED (GPIO_NUM_13)
#define HUZZAH32_GPIO_BITPIN_LED (1ULL<<HUZZAH32_GPIO_NUM_LED)

typedef enum {
    LED_WIRING_TYPE_LED_LOW_SIDE = 1,  /*!< PCB: GPIO .. Resistor .. LED-DIODE|> .. GND (MCU Adafruit HUZZAH32) */
    LED_WIRING_TYPE_LED_HIGH_SIDE = 2, /*!< PCB: GPIO .. |<LED-DIODE .. Resistor .. VCC (MCU Lolin32Lite, LOLIN D32) */
} mjd_led_wiring_type_t;

typedef struct {
    uint32_t is_init;                  /*!< Helper to know if an element was initialized, or not. Mark 1. */
    uint64_t gpio_num;                 /*!< GPIO num pin */
    mjd_led_wiring_type_t wiring_type; /*!< Wiring Type */
} mjd_led_config_t;

#define MJD_LED_CONFIG_DEFAULT() { \
    .is_init = false, \
    .gpio_num = GPIO_NUM_MAX, \
    .wiring_type = LED_WIRING_TYPE_LED_LOW_SIDE, \
}

void mjd_led_config(const mjd_led_config_t *param_led_config);
void mjd_led_on(int param_gpio_nr);
void mjd_led_off(int param_gpio_nr);
void mjd_led_blink_times(int param_gpio_nr, int param_count);
void mjd_led_mark_error(int param_gpio_nr);

#ifdef __cplusplus
}
#endif

#endif /* __MJD_H__ */

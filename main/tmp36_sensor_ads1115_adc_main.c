#include "mjd.h"
#include "mjd_ads1115.h"
#include "mjd_tmp36.h"

/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * KConfig: LED
 */
static const int MY_LED_ON_DEVBOARD_GPIO_NUM = CONFIG_MY_LED_ON_DEVBOARD_GPIO_NUM;
static const int MY_LED_ON_DEVBOARD_WIRING_TYPE = CONFIG_MY_LED_ON_DEVBOARD_WIRING_TYPE;

static const int MY_ADS1115_I2C_SLAVE_ADDRESS = CONFIG_MY_ADS1115_I2C_SLAVE_ADDRESS;
static const int MY_ADS1115_I2C_MASTER_PORT_NUM = CONFIG_MY_ADS1115_I2C_MASTER_PORT_NUM;
static const int MY_ADS1115_I2C_SCL_GPIO_NUM = CONFIG_MY_ADS1115_I2C_SCL_GPIO_NUM;
static const int MY_ADS1115_I2C_SDA_GPIO_NUM = CONFIG_MY_ADS1115_I2C_SDA_GPIO_NUM;
static const int MY_ADS1115_ALERT_READY_GPIO_NUM = CONFIG_MY_ADS1115_ALERT_READY_GPIO_NUM;
static const int MY_ADS1115_DATA_RATE = CONFIG_MY_ADS1115_DATA_RATE;

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_8K (8192)

/*
 * Project Globs
 */

/*
 * INIT ONCE
 */

/*
 * TASK
 */
void peripheral_task(void *pvParameter) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval;

    mjd_led_config_t led_config =
        { 0 };
    led_config.gpio_num = MY_LED_ON_DEVBOARD_GPIO_NUM; // (Huzzah32 #13) (Lolin32lite #22)
    led_config.wiring_type = MY_LED_ON_DEVBOARD_WIRING_TYPE; // (Huzzah32 1=GND) (Lolin32lite 2=VCC)
    mjd_led_config(&led_config);

    /*
     * DEV INIT: ADS1115
     * @doc The mjd_tmp36 component does not require init.
     */
    ESP_LOGI(TAG, "do mjd_ads1115_init()");

    // @important Do not use ={} or ={0}
    mjd_ads1115_config_t ads1115_config = MJD_ADS1115_CONFIG_DEFAULT()
    ;
    ads1115_config.i2c_slave_addr = MY_ADS1115_I2C_SLAVE_ADDRESS;
    ads1115_config.i2c_port_num = MY_ADS1115_I2C_MASTER_PORT_NUM;
    ads1115_config.i2c_scl_gpio_num = MY_ADS1115_I2C_SCL_GPIO_NUM;
    ads1115_config.i2c_sda_gpio_num = MY_ADS1115_I2C_SDA_GPIO_NUM;
    // @tip Omit this line if you do not want to use the ALERT READY PIN (the component will wait a specified milisec-time depending on params).
    ads1115_config.alert_ready_gpio_num = MY_ADS1115_ALERT_READY_GPIO_NUM;
    ads1115_config.data_rate = MY_ADS1115_DATA_RATE;
    ads1115_config.pga = MJD_ADS1115_PGA_2_048; // Maximum voltage of TMP36 is 1.75V for 125 Degrees Celsius

    f_retval = mjd_ads1115_init(&ads1115_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_ads1115_init() err %i %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * LOG DEVICE PARAMS: ADS1115, TMP36
     */
    mjd_ads1115_log_device_parameters(&ads1115_config);
    mjd_tmp36_log_config();

    /*
     * LOOP
     */
    mjd_ads1115_data_t ads1115_data =
        { 0 };
    mjd_tmp36_data_t tmp36_data =
        { 0 };
    uint32_t nbr_of_adc_errors = 0;
    float sum_degrees_celsius = 0;
    float min_degrees_celsius = FLT_MAX;
    float max_degrees_celsius = FLT_MIN;
    mjd_log_time();
    const uint32_t NBR_OF_RUNS = 1000;
    ESP_LOGI(TAG, "TMP36 Measurements via the ADS1115 ADC");
    ESP_LOGI(TAG, "LOOP: NBR_OF_RUNS %u", NBR_OF_RUNS);
    for (uint32_t j = 1; j <= NBR_OF_RUNS; ++j) {
        mjd_led_on(MY_LED_ON_DEVBOARD_GPIO_NUM);

        ESP_LOGI(TAG, "***Measurement#%u", j);

        // A0
        f_retval = mjd_ads1115_cmd_get_single_conversion(&ads1115_config, &ads1115_data);
        if (f_retval != ESP_OK) {
            mjd_led_mark_error(MY_LED_ON_DEVBOARD_GPIO_NUM);
            ++nbr_of_adc_errors;
            ESP_LOGE(TAG, "%s(). Cannot Get Single Conversion | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
            // CONTINUE
            continue;
        }
        tmp36_data.in_volts = ads1115_data.volt_value;
        mjd_tmp36_convert_volts_to_degrees_celsius(&tmp36_data);

        ESP_LOGI(TAG, "  TMP36: %.3f Degrees Celsius | ADC Pin A0: raw_value (signed int16): %5i | volt_value (float): %.3f",
                 tmp36_data.out_degrees_celsius, ads1115_data.raw_value, ads1115_data.volt_value);

        sum_degrees_celsius += tmp36_data.out_degrees_celsius;
        if (tmp36_data.out_degrees_celsius < min_degrees_celsius) {
            min_degrees_celsius = tmp36_data.out_degrees_celsius;
        }
        if (tmp36_data.out_degrees_celsius > max_degrees_celsius) {
            max_degrees_celsius = tmp36_data.out_degrees_celsius;
        }

        mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);

        // @optional A visual delay between reading loop items, is sometimes easier when debugging
        /////vTaskDelay(RTOS_DELAY_1SEC);

    }
    ESP_LOGI(TAG, "REPORT:");
    ESP_LOGI(TAG, "  NBR_OF_RUNS:       %u", NBR_OF_RUNS);
    ESP_LOGI(TAG, "  nbr_of_adc_errors: %u", nbr_of_adc_errors);
    ESP_LOGI(TAG, "  TMP36 Temperature Readings:");
    ESP_LOGI(TAG, "    AVG degrees_celsius  min_degrees_celsius  max_degrees_celsius");
    ESP_LOGI(TAG, "    -------------------  -------------------  -------------------");
    ESP_LOGI(TAG, "    %19.3f  %19.3f  %19.3f", sum_degrees_celsius / NBR_OF_RUNS, min_degrees_celsius, max_degrees_celsius);

    /*
     * DEVICE DE-INIT
     * @doc The mjd_tmp36 component does not require deinit.
     */
    f_retval = mjd_ads1115_deinit(&ads1115_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_ads1115_deinit() | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    cleanup: ;

    mjd_log_time();

    // HALT (end of task)
    mjd_rtos_wait_forever();
}

/*
 * MAIN
 */
void app_main() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    BaseType_t xReturned;

    /* SOC init */
    ESP_LOGI(TAG, "@doc exec nvs_flash_init() - mandatory for Wifi to work later on");
    nvs_flash_init();

    /********************************************************************************
     * MY STANDARD Init
     *
     */
    mjd_log_chip_info();
    mjd_log_memory_statistics();
    mjd_set_timezone_utc();
    mjd_log_time();
    ESP_LOGI(TAG,
            "@tip You can also change the log level to DEBUG for more detailed logging and to get insights in what the component is actually doing.");
    ESP_LOGI(TAG, "@doc Wait 2 seconds after power-on (start logic analyzer, let peripherals become active, ...)");
    vTaskDelay(RTOS_DELAY_2SEC);

    /*
     * Sensor Task
     */
    xReturned = xTaskCreatePinnedToCore(&peripheral_task, "peripheral_task (name)", MYAPP_RTOS_TASK_STACK_SIZE_8K, NULL, RTOS_TASK_PRIORITY_NORMAL,
    NULL,
    APP_CPU_NUM);
    if (xReturned == pdPASS) {
        ESP_LOGI(TAG, "OK Task has been created, and is running right now");
    }

    ESP_LOGI(TAG, "app_main() END");
}

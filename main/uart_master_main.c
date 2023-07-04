#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#include "esp_log.h"

#define TAG "MASTER"

/**
 * - Port: UART1
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below
 */

#define PIN_TXD  (33)
#define PIN_RXD  (39)
#define ECHO_TEST_RTS  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS  (UART_PIN_NO_CHANGE)

#define BUF_SIZE (1024)

static void master_read_write_task(void *arg)
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config =
    {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_driver_install(UART_NUM_2, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_2, &uart_config);
    uart_set_pin(UART_NUM_2, PIN_TXD, PIN_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS);

    // Configure a temporary buffer for the incoming data
    uint8_t *data_from_master = (uint8_t *) malloc(BUF_SIZE);
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    char string[BUF_SIZE]= "Hello world from master!";
    memcpy(data_from_master, string, BUF_SIZE);

#ifdef WRITE_FROM_TERMINAL
    char read_char = '\0';
    while(1) {
        size_t nmbr_bytes_read = fread(&read_char, 1, 1, stdin);
        if(nmbr_bytes_read)
        {
            if (read_char != '\0')
            {
                // Write data read from terminal to Slave
                uart_write_bytes(UART_NUM_2, (const char *) &read_char, 1);
                read_char = '\0';
            }else
            {
                vTaskDelay(10/portTICK_PERIOD_MS);
            }
        }else
        {
            vTaskDelay(10/portTICK_PERIOD_MS);
        }
    }
#else
    while (1)
    {
        // Write data back to the UART
        uart_write_bytes(UART_NUM_2, (const char *) data_from_master, BUF_SIZE);

        // Read data from the UART
        memset(data,0, BUF_SIZE);
        uart_read_bytes(UART_NUM_2, data, BUF_SIZE, 200 / portTICK_RATE_MS);
        ESP_LOGI(TAG, "Received from SLAVE: %s", data);

        vTaskDelay(100/portTICK_PERIOD_MS);
    }
#endif // WRITE_FROM_TERMINAL
}

void app_main(void)
{
    xTaskCreate(master_read_write_task, "UART_MASTER_TASK", 5*1024, NULL, 10, NULL);
}

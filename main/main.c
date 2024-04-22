/*
 * LED blink with FreeRTOS
 */
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>
#include <string.h>
#include "pico/stdlib.h"
#include <stdio.h>
#include "hc06.h"
#include "hardware/adc.h"
#include <math.h>
#include <stdlib.h>

const int BTN_Confirm = 2; // z or enter
const int BTN_Cancel = 3; // x or shift
const int BTN_Menu = 4; // c or ctrl
const int BTN_Fullscreen = 5; // f4
const int BTN_Quit = 6; // hold esc
const int BTN_Quit = 7; // extra

// hc-06
const int HC_TX = 8;
const int HC_RX = 9;

const int LED_Connected = 10; // bluetooth connected

const int JOY_X_PIN = 26;
const int JOY_Y_PIN = 27;
const int DEAD_ZONE = 30;

QueueHandle_t xQueueAdc;
QueueHandle_t xQueueBTN;

typedef struct adc {
    int axis;
    int val;
} adc_t;

void button_callback(uint gpio, uint32_t events) {
    int pressed_btn = 0;
    if (gpio == BTN_Confirm && events == GPIO_IRQ_EDGE_FALL) {
        pressed_btn = BTN_Confirm;
    } else if (gpio == BTN_Cancel && events == GPIO_IRQ_EDGE_FALL) {
        pressed_btn = BTN_Cancel;
    } else if (gpio == BTN_Menu && events == GPIO_IRQ_EDGE_FALL) {
        pressed_btn = BTN_Menu;
    } else if (gpio == BTN_Fullscreen && events == GPIO_IRQ_EDGE_FALL) {
        pressed_btn = BTN_Fullscreen;
    } else if (gpio == BTN_Quit && events == GPIO_IRQ_EDGE_FALL) {
        pressed_btn = BTN_Quit;
    }
    xQueueSendFromISR(xQueueBTN, &pressed_btn, NULL);
}

void joy_x_task(void *p) {
    adc_t data;
    adc_init();

    while (1) {
        adc_gpio_init(JOY_X_PIN);
        adc_select_input(0);
        data.axis = 0;
        int val = adc_read();
        val -= 2048;
        val /= 8;
        if (abs(val) < DEAD_ZONE) {
            val = 0;
        }
        data.val = val;
        xQueueSend(xQueueAdc, &data, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void joy_y_task(void *p) {
    adc_t data;
    adc_init();

    while (1) {
        adc_gpio_init(JOY_Y_PIN);
        adc_select_input(1);
        data.axis = 1;
        int val = adc_read();
        val -= 2048;
        val /= 8;
        if (abs(val) < DEAD_ZONE) {
            val = 0;
        }
        data.val = val;
        xQueueSend(xQueueAdc, &data, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void uart_task(void *p) {
    adc_t data;

    while (1) {
        xQueueReceive(xQueueAdc, &data, portMAX_DELAY);
        int msb = data.val >> 8;
        int lsb = data.val & 0xFF;

        uart_putc_raw(uart0, data.axis);
        uart_putc_raw(uart0, lsb);
        uart_putc_raw(uart0, msb);
        uart_putc_raw(uart0, -1);
    }
}

void select_output(void){
    int pressed_btn = 0;
    if(xQueueReceiveFromISR(xQueueBTN, &pressed_btn, portMAX_DELAY) == pdTRUE){
        if(pressed_btn == BTN_Confirm){

        }
    }
}

void hc06_task(void *p) {
    uart_init(HC06_UART_ID, HC06_BAUD_RATE);
    gpio_set_function(HC06_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(HC06_RX_PIN, GPIO_FUNC_UART);
    hc06_init("aps2_legal", "1234");

    while (true) {
        uart_puts(HC06_UART_ID, "OLAAA ");
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

int main() {
    stdio_init_all();

    printf("Start bluetooth task\n");

    uart_init(uart0, 115200);

    xQueueBTN = xQueueCreate(10, sizeof(int));
    xQueueAdc = xQueueCreate(32, sizeof(adc_t));

    xTaskCreate(hc06_task, "UART_Task 1", 4096, NULL, 1, NULL);
    xTaskCreate(joy_x_task, "x_task", 4096, NULL, 1, NULL);
    xTaskCreate(joy_y_task, "y_task", 4096, NULL, 1, NULL);
    xTaskCreate(uart_task, "uart_task", 4096, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}

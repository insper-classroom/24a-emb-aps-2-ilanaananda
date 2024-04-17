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

const int BTN_Confirm = 0; // z or enter
const int BTN_Cancel = 1; // x or shift
const int BTN_Menu = 2; // c or ctrl
const int BTN_Fullscreen = 3; // f4
const int BTN_Quit = 4; // hold esc
const int BTN_OnOff = 5; // power control

const int LED_Connected = 6; // bluetooth connected

#define JOY_X_PIN 26
#define JOY_Y_PIN 27
#define DEAD_ZONE 30

QueueHandle_t xQueueAdc;
QueueHandle_t xQueueBTN;

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
    } else if (gpio == BTN_OnOff && events == GPIO_IRQ_EDGE_FALL) {
        pressed_btn = BTN_OnOff;
    }
    xQueueSendFromISR(xQueueBTN, &pressed_btn, NULL);
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

    xQueueBTN = xQueueCreate(10, sizeof(int));
    xQueueAdc = xQueueCreate(10, sizeof(int));

    xTaskCreate(hc06_task, "UART_Task 1", 4096, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}

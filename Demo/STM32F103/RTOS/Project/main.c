#include "sys.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "croutine.h"

#include "bsp_usart.h"
#include "bsp_led.h"
#include "bsp_init.h"

void vTaskTaskLED0(void *pvParameters);
void vTaskTaskLED1(void *pvParameters);

int main(void)
{
    bsp_Init();//硬件初始化
    
    xTaskCreate(    vTaskTaskLED0,   /* 任务函数  */
                    "vTaskTaskLED0",     /* 任务名    */
                    512,               /* stack大小，单位word，也就是4字节 */
                    NULL,              /* 任务参数  */
                    1,                 /* 任务优先级*/
                    NULL );            /* 任务句柄  */

    xTaskCreate(    vTaskTaskLED1,   /* 任务函数  */
                    "vTaskTaskLED1",     /* 任务名    */
                    512,               /* stack大小，单位word，也就是4字节 */
                    NULL,              /* 任务参数  */
                    1,                 /* 任务优先级*/
                    NULL );            /* 任务句柄  */
    
    vTaskStartScheduler();
    
    while(1);
}

void vTaskTaskLED0(void *pvParameters)
{
    while(1)
    {
        LED0 = !LED0;
        vTaskDelay(500);
    }
}

void vTaskTaskLED1(void *pvParameters)
{
    while(1)
    {
        LED1 = !LED1;
        vTaskDelay(250);
    }
}

/*
 * Copyright (c) 2022 HiSilicon (Shanghai) Technologies CO., LIMITED.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include "iot_config.h"
#include "iot_main.h"
#include "iot_profile.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifi_connecter.h"
#include "iot_gpio_ex.h"
#include "iot_gpio.h"
#include "iot_watchdog.h"
#include "cjson_init.h"
#include "hi_stdlib.h"
#include "hcsr04.h"

#define TASK_SLEEP_1000MS (1000)

/*上报车位情况*/
void IotPublishSample(void)
{
    float distance;
    distance=GetDistance();
    IoTProfileService service;
    IoTProfileKV property;
    memset_s(&property, sizeof(property), 0, sizeof(property));
    property.type = EN_IOT_DATATYPE_STRING;
    property.key = "Space1";/*spacex表示车位x,只修改这部分即可得到不同车位的上报代码*/
    if (distance >= 20) 
    {
        property.value = "SPACE_YES";
    } 
    else 
    {
        property.value = "SPACE_NO";
    }
    memset_s(&service, sizeof(service), 0, sizeof(service));
    service.serviceID = "ParkingSpace";
    service.serviceProperty = &property;
    IoTProfilePropertyReport(CONFIG_DEVICE_ID, &service);
}

/*主函数*/
static void MqttTread(void)
{
    Hcsr04Init();
    ConnectToHotspot();
    CJsonInit();
    IoTMain();
    TaskMsleep(30000); // 30000 = 30s连接华为云
    /*主动上报 */
    while (1)
    {
        /*延时函数，每隔1s上报一次信息*/
        TaskMsleep(TASK_SLEEP_1000MS);
        /*将车位信息上报华为云*/
        IotPublishSample();
    }
}

// This is the demo entry, we create a task here, and all the works has been done in the demo_entry
#define CN_IOT_TASK_STACKSIZE  0x1000
#define CN_IOT_TASK_PRIOR 28
static void AppDemoIot(void)
{
    osThreadAttr_t attr;
    IoTWatchDogDisable();
    attr.name = "MqttTread";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = CN_IOT_TASK_STACKSIZE;
    attr.priority = CN_IOT_TASK_PRIOR;

    if (osThreadNew((osThreadFunc_t)MqttTread, NULL, &attr) == NULL)//创建线程
    {
        printf("[MqttTread] Failed to create MqttTread!\n");
    }
}

SYS_RUN(AppDemoIot);
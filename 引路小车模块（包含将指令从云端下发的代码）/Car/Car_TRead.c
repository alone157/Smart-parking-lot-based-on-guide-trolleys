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
#include "stdint.h"
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
#include "motor_control.h"
#include "gyro.h"
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <hi_time.h>
#include <hi_timer.h>
#include "iot_i2c.h"
#include "iot_errno.h"
#include "hi_errno.h"
#include "hi_i2c.h"
#include "pca9555.h"

/* attribute initiative to report */
#define TAKE_THE_INITIATIVE_TO_REPORT
/* oc request id */
#define CN_COMMAND_INDEX                    "commands/request_id="

#define TASK_SLEEP_1000MS (1000)
int start=0;

static void Car_Go(char *payload)
{
    printf("PAYLOAD:%s\r\n", payload);
    if (strstr(payload, "Space1") != NULL)/*前往车位1*/
    {
        if (strstr(payload, "go") != NULL) 
        { 
            num1();
        }
    }
    else if(strstr(payload, "Space2") != NULL)/*前往车位2*/
    {
        if (strstr(payload, "go") != NULL) 
        { 
            num2();
        }
    }
    else if (strstr(payload, "Space3") != NULL)/*前往车位3*/
    {
        if (strstr(payload, "go") != NULL) 
        { 
            num3();
        }
    }
    else if (strstr(payload, "Space4") != NULL)/*前往车位4*/
    {
        if (strstr(payload, "go") != NULL) 
        { 
            num4();
        }
    }
    else if (strstr(payload, "Space5") != NULL)/*前往车位5*/
    {
        if (strstr(payload, "go") != NULL) 
        { 
            num5();
        }
    }
    else if (strstr(payload, "Space6") != NULL)/*前往车位6*/
    {
        if (strstr(payload, "go") != NULL) 
        { 
            num6();
        }
    }
}

// /< this is the callback function, set to the mqtt, and if any messages come, it will be called
// /< The payload here is the json string
static void DemoMsgRcvCallBack(int qos, char *topic, char *payload)//下发
{
    const char *requesID;
    char *tmp;
    IoTCmdResp resp;
    printf("RCVMSG:QOS:%d TOPIC:%s PAYLOAD:%s\r\n", qos, topic, payload);
    /* app 下发的操作 */
    Car_Go(payload);
    tmp = strstr(topic, CN_COMMAND_INDEX);
    if (tmp != NULL) 
    {
        // /< now you could deal your own works here --THE COMMAND FROM THE PLATFORM
        // /< now er roport the command execute result to the platform
        requesID = tmp + strlen(CN_COMMAND_INDEX);
        resp.requestID = requesID;
        resp.respName = NULL;
        resp.retCode = 0;   ////< which means 0 success and others failed
        resp.paras = NULL;
        (void)IoTProfileCmdResp(CONFIG_DEVICE_PWD, &resp);
    }
}

/*主函数*/
static void CarTread(void)
{
    Hcsr04Init();//超声波传感器初始化
    IMUInit();//陀螺仪初始化
    ConnectToHotspot();
    CJsonInit();
    IoTMain();
    IoTSetMsgCallback(DemoMsgRcvCallBack);
    TaskMsleep(30000); // 30000 = 30s连接华为云
   
}

// This is the demo entry, we create a task here, and all the works has been done in the demo_entry
#define CN_IOT_TASK_STACKSIZE  0x1000
#define CN_IOT_TASK_PRIOR 28
static void AppDemoIot(void)
{
    osThreadAttr_t attr;
    IoTWatchDogDisable();
    attr.name = "CarTread";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = CN_IOT_TASK_STACKSIZE;
    attr.priority = CN_IOT_TASK_PRIOR;

    if (osThreadNew((osThreadFunc_t)CarTread, NULL, &attr) == NULL) 
    {
        printf("[CarTread] Failed to create CarTread!\n");
    }
}

SYS_RUN(AppDemoIot);
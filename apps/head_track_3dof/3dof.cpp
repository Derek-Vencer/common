/***************************************************************************
 *
 * Copyright 2015-2023 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 * Application layer function of head tracking 3dof with lsm6dsl sensor
 ****************************************************************************/
#include "hal_trace.h"
#include "hal_i2c.h"
#include "hal_timer.h"
#include "hal_spi.h"
#include "string.h"
#include "stdlib.h"

#include "hal_sysfreq.h"

#ifdef RTOS
#include "cmsis_os.h"
#ifdef KERNEL_RTX
#include "rt_Time.h"
#endif
#endif

#include "3dof.h"
#include "head_track_3dof.h"
// #include "bts_tws_api.h"

#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_STEREO__) || !defined(HEAD_TRACK_MULTI_CORE_COMMUNICATE)
// #include "app_bt_stream.h"
extern "C" int32_t audio_process_stereo_set_yaw(float yaw);
extern "C" int32_t audio_process_stereo_set_pitch(float pitch);
#endif

#define HEAD_TRACK_SIGNAL_ALGO          (0x01)
#define HEAD_TRACK_TIMER_INTERVAL       (10)  // 10ms-20ms, related to freq
#define HEAD_TRACK_TIMER_FREQ           (100) // 50-100hz, related to chip computing performance

static int head_track_inited = 0;
static int head_track_algo_inited = 0;
static int head_track_send_inited = 0;
// static bool is_ht_data_sending_enabled = false;

static osMutexId head_track_sensor_mutex_id = NULL;
osMutexDef(head_track_sensor_mutex);

static osMutexId pre_sensor_mutex_id = NULL;
osMutexDef(pre_sensor_mutex);

static osMutexId head_track_pose_mutex_id = NULL;
osMutexDef(head_track_pose_mutex);

static osSemaphoreId sensor_signal = NULL;
osSemaphoreDef(sensor_signal);

static struct SENSOR_IMU preImu;
void imu_data_set(float ax, float ay, float az, float gx, float gy, float gz, unsigned int timeStamp)
{
    osMutexWait(pre_sensor_mutex_id, osWaitForever);

    // Accelerometer, unit mg
    preImu.a_x = ax;
    preImu.a_y = ay;
    preImu.a_z = az;

    // Gyroscope , unit mdeg/s
    preImu.g_x = gx;
    preImu.g_y = gy;
    preImu.g_z = gz;

    // timestamp unit us;  1s = 1000ms = 1000000us
    preImu.frameTimeStamp = timeStamp;
    preImu.rflag = 1;

    osMutexRelease(pre_sensor_mutex_id);
}

void head_track_resouce_init(void)
{
    if(head_track_sensor_mutex_id == NULL) {
        head_track_sensor_mutex_id = osMutexCreate(osMutex(head_track_sensor_mutex));
        if (head_track_sensor_mutex_id == NULL) {
            HEAD_TRACK_3DOF_TRACE(1, "Failed to Create head_track_sensor_mutex_id");
        }
    }

    if(pre_sensor_mutex_id == NULL) {
        pre_sensor_mutex_id = osMutexCreate(osMutex(pre_sensor_mutex));
        if (pre_sensor_mutex_id == NULL) {
            HEAD_TRACK_3DOF_TRACE(1, "Failed to Create pre_sensor_mutex_id");
        }
    }

    if (head_track_pose_mutex_id == NULL) {
        head_track_pose_mutex_id = osMutexCreate(osMutex(head_track_pose_mutex));
        if (head_track_pose_mutex_id == NULL) {
            HEAD_TRACK_3DOF_TRACE(1, "Failed to Create head_track_pose_mutex_id");
        }
    }

    if (sensor_signal == NULL) {
        sensor_signal = osSemaphoreCreate(osSemaphore(sensor_signal), 1);
        if (sensor_signal == NULL) {
            HEAD_TRACK_3DOF_TRACE(1, "Failed to Create sensor_signal");
        }
    }

    // if(head == NULL) {
    //     head_track_window_link_init();
    //     if(head == NULL) {
    //         HEAD_TRACK_3DOF_TRACE(1, "Failed to Create head track window LinkList");
    //     }
    // }else{
    //     /* release, then init again */
    //     head_track_window_link_release();
    //     head_track_window_link_init();
    //     if(head == NULL) {
    //         HEAD_TRACK_3DOF_TRACE(1, "Failed to Create head track window LinkList");
    //     }
    // }
}

#define HEAD_TRACK_THREAD_STACK_SIZE    2048
static void head_track_task_thread(void const* argument);
static osThreadId head_track_thread_id;
osThreadDef(head_track_task_thread, osPriorityAboveNormal, 1, HEAD_TRACK_THREAD_STACK_SIZE, "head_track_thread");

#define POSE_SIZE (sizeof(struct POSE_S))
#define SENSOR_SIZE (sizeof(struct SENSOR_IMU))
static struct POSE_S pose_s;
static struct SENSOR_IMU sensor;
static struct SENSOR_IMU pre_sensor;
static void head_track_task_thread(void const*argument)
{
    osEvent evt;
    uint32_t signals = 0;

    while (1) {
        // self define
        evt = osSignalWait(0x0, osWaitForever);
        signals = evt.value.signals;

        if (evt.status == osEventSignal) {
            if (signals & HEAD_TRACK_SIGNAL_ALGO) {

                osMutexWait(pre_sensor_mutex_id, osWaitForever);
                memcpy(&pre_sensor, &preImu, SENSOR_SIZE);
                osMutexRelease(pre_sensor_mutex_id);

                //todo, imu raw data filter

                osMutexWait(head_track_sensor_mutex_id, osWaitForever);
                memcpy(&sensor, &pre_sensor, SENSOR_SIZE);
                osSemaphoreRelease(sensor_signal);
                osMutexRelease(head_track_sensor_mutex_id);
            }
        }
    }
}

#define HEAD_TRACK_ALGO_THREAD_STACK_SIZE    4096
static void head_track_algo_task_thread(void const* argument);
static osThreadId head_track_algo_thread_id;
osThreadDef(head_track_algo_task_thread, osPriorityAboveNormal, 1, HEAD_TRACK_ALGO_THREAD_STACK_SIZE, "head_track_algo_thread");

static void head_track_algo_task_thread(void const*argument)
{
    struct POSE_S algoPose;
    struct SENSOR_IMU localSensor;

    memset(&localSensor, 0, sizeof(struct SENSOR_IMU));
    memset(&algoPose, 0, sizeof(struct POSE_S));

    while (1)
    {
        // head_track_task_thread get sensor data
        osSemaphoreWait(sensor_signal, osWaitForever);
        osMutexWait(head_track_sensor_mutex_id, osWaitForever);
        memcpy(&localSensor, &sensor, SENSOR_SIZE);
        sensor.rflag = 0; // get sensor data then set 0
        osMutexRelease(head_track_sensor_mutex_id);
        if(localSensor.rflag == 1) {
#ifdef HT_ANALYSIS_ENABLE
            uint32_t start_ts = FAST_TICKS_TO_US(hal_fast_sys_timer_get());
#endif
            // compute pose
            head_track_3dof_algo_with_fix_param(&algoPose, &localSensor);
#ifdef HT_ANALYSIS_ENABLE
            uint32_t end_ts = FAST_TICKS_TO_US(hal_fast_sys_timer_get());
            HEAD_TRACK_3DOF_TRACE(0, "### ts: %u, cost:%u", end_ts, end_ts - start_ts);
#endif
        }

        // memcpy pose
        osMutexWait(head_track_pose_mutex_id, osWaitForever);
        memcpy(&pose_s, &algoPose, POSE_SIZE);
        osMutexRelease(head_track_pose_mutex_id);
    }
}

#define HEAD_TRACK_SEND_THREAD_STACK_SIZE    2048
static void head_track_send_task_thread(void const* argument);
static osThreadId head_track_send_thread_id;
osThreadDef(head_track_send_task_thread, osPriorityNormal, 1, HEAD_TRACK_SEND_THREAD_STACK_SIZE, "head_track_send_thread");

static int get_yaw = 0;
static int get_pitch = 0;
int head_track_get_yaw(void)
{
    return get_yaw;
}

int head_track_get_pitch(void)
{
    return get_pitch;
}

#ifdef HEAD_TRACK_MULTI_CORE_COMMUNICATE
#include "app_dsp_m55.h"

// Casually named/defined
uint16_t group = 0x01;
uint16_t subCode = 0x02;
uint16_t MCU_DSP_M55_3DOF_UPDATE = CORE_BRIDGE_CMD_CODE(group, subCode);
static void gaf_3dof_transmit_handler(uint8_t* ptr, uint16_t len)
{
    app_dsp_m55_bridge_send_data_without_waiting_rsp(MCU_DSP_M55_3DOF_UPDATE, ptr, len);
}

M55_CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_DSP_M55_3DOF_UPDATE,
                            "MCU_DSP_M55_3DOF_UPDATE",
                            gaf_3dof_transmit_handler,
                            NULL,
                            0,
                            NULL,
                            NULL,
                            NULL);

extern "C" uint8_t a2dp_is_run(void);
#endif

static void head_track_send_task_thread(void const*argument)
{
    struct POSE_S send_pose;
    memset(&send_pose, 0, sizeof(struct POSE_S));
    while (1)
    {
        osMutexWait(head_track_pose_mutex_id, osWaitForever);
        memcpy(&send_pose, &pose_s, POSE_SIZE);
        osMutexRelease(head_track_pose_mutex_id);
        /*
            set spatial audio with 3dof angle if necessary,
            the spatial audio APIs might be different,
            here is just a demo usage.

            also, the pose data can be sent to cell-phone or whatever, according to customer's wishes
        */
#ifdef HEAD_TRACK_MULTI_CORE_COMMUNICATE
        // multi core communicate
        if (a2dp_is_run()) {
            app_dsp_m55_bridge_send_cmd(MCU_DSP_M55_3DOF_UPDATE, \
                                            (uint8_t*)&send_pose, \
                                            sizeof(struct POSE_S));
        }
#else
        // single core process
        audio_process_stereo_set_yaw((float)send_pose.yaw);
        audio_process_stereo_set_pitch((float)send_pose.pitch);
#endif
        get_yaw = (int)(send_pose.yaw * 57.3);
        get_pitch = (int)(send_pose.pitch * 57.3);
        HEAD_TRACK_3DOF_TRACE(3,"[CXW] get_yaw:%d,get_pitch:%d",get_yaw,get_pitch);

#ifdef HT_ANALYSIS_ENABLE
        HEAD_TRACK_3DOF_TRACE(0, "#### %u\t%d\t%d\t%d", FAST_TICKS_TO_US(hal_fast_sys_timer_get()),
            (int)(send_pose.yaw * 57.3),
            (int)(send_pose.pitch * 57.3),
            (int)(send_pose.pitch * 57.3));
        hal_sysfreq_print_user_freq(); //cycle
        HEAD_TRACK_3DOF_TRACE(0, "### sysfreq calc : %d\n", hal_sys_timer_calc_cpu_freq(5, 0));
#endif
        // maybe update
        osDelay(25);
    }
}

static int head_track_task_init(void)
{
    HEAD_TRACK_3DOF_TRACE(0, "%s(true),%d \n\r", __FUNCTION__, __LINE__);

    if (!head_track_inited) {
        head_track_thread_id = osThreadCreate(osThread(head_track_task_thread), NULL);
        if (head_track_thread_id == NULL) {
            HEAD_TRACK_3DOF_TRACE(0, "create head_track_thread failed");
            return -1;
        }
        head_track_inited = 1;
    }
    return 0;
}

static int head_track_algo_task_init(void)
{
    HEAD_TRACK_3DOF_TRACE(0, "%s(true),%d \n\r", __FUNCTION__, __LINE__);

    if(!head_track_algo_inited)
    {
        head_track_algo_thread_id = osThreadCreate(osThread(head_track_algo_task_thread), NULL);
        if (head_track_algo_thread_id == NULL) {
            HEAD_TRACK_3DOF_TRACE(0, "create head_track_algo_thread failed");
            return -1;
        }

        head_track_algo_inited = 1;
    }
    return 0;
}

static int head_track_send_task_init(void)
{
    HEAD_TRACK_3DOF_TRACE(0, "%s(true),%d \n\r", __FUNCTION__, __LINE__);

    if(!head_track_send_inited)
    {
        head_track_send_thread_id = osThreadCreate(osThread(head_track_send_task_thread), NULL);
        if (head_track_send_thread_id == NULL) {
            HEAD_TRACK_3DOF_TRACE(0, "create head_track_send_thread failed");
            return -1;
        }

        head_track_send_inited = 1;
    }
    return 0;
}

void imu_timer_handler(void const *param);
osTimerDef (imu_timer, imu_timer_handler);
osTimerId imu_timer_id = NULL;

void imu_timer_handler(void const *param)
{
    osSignalSet(head_track_thread_id, HEAD_TRACK_SIGNAL_ALGO);
}

void head_angle_reset(void)
{
    head_track_3dof_reset();
}

void start_head_track_timer(void)
{
    HEAD_TRACK_3DOF_TRACE(0, "start head_track algo timer!");
    osStatus_t status = osTimerStart(imu_timer_id, HEAD_TRACK_TIMER_INTERVAL);
    if (status != osOK) {
        HEAD_TRACK_3DOF_TRACE(0, "start head_track algo timer failed!");
    }
}

void pause_head_track_timer(void)
{
    HEAD_TRACK_3DOF_TRACE(0, "pause head_track algo timer!");
    osStatus_t status = osTimerStop(imu_timer_id);
    if (status != osOK) {
        HEAD_TRACK_3DOF_TRACE(0, "pause head_track algo timer failed!");
    }
}

void ht_init(void)
{
    HEAD_TRACK_3DOF_TRACE(0, "%s init", __func__);

    memset(&preImu, 0, sizeof(struct SENSOR_IMU));

    /* Init head track task : mutx signal init  */
    head_track_resouce_init();

    /* Init head track task  */
    head_track_task_init();
    head_track_algo_task_init();
    head_track_send_task_init();

    /* Create imu timer*/
    if (imu_timer_id == NULL)
    {
        imu_timer_id = osTimerCreate(osTimer(imu_timer), osTimerPeriodic, NULL);
    }

    return;
}

void head_track_algo_start(void)
{
    pause_head_track_timer();
    head_angle_reset();
    start_head_track_timer();
}

void head_track_algo_pause(void)
{
    HEAD_TRACK_3DOF_TRACE(0, "%s ",__func__);
    head_angle_reset();
    pause_head_track_timer();
    // is_ht_data_sending_enabled = false;
}

static void head_track_send_task_deinit(void)
{
    HEAD_TRACK_3DOF_TRACE(0, "%s(true),%d \n\r", __FUNCTION__, __LINE__);
    // is_ht_data_sending_enabled = false;
    osThreadTerminate(head_track_send_thread_id);
    head_track_send_inited = 0;
}

static void head_track_algo_task_deinit(void)
{
    HEAD_TRACK_3DOF_TRACE(0, "%s(true),%d \n\r", __FUNCTION__, __LINE__);
    osThreadTerminate(head_track_algo_thread_id);
    head_track_algo_inited = 0;
}

static void head_track_task_deinit(void)
{
    HEAD_TRACK_3DOF_TRACE(0, "%s(true),%d \n\r", __FUNCTION__, __LINE__);
    osThreadTerminate(head_track_thread_id);
    head_track_inited = 0;
}

void head_track_resouce_deinit(void)
{
    // maybe delete
    head_track_sensor_mutex_id = NULL;
    pre_sensor_mutex_id = NULL;
    head_track_pose_mutex_id = NULL;
    sensor_signal = NULL;
}

void ht_deinit(void)
{
    HEAD_TRACK_3DOF_TRACE(0, "%s(true),%d \n\r", __FUNCTION__, __LINE__);
    if(imu_timer_id != NULL) {
        osTimerDelete(imu_timer_id);
        imu_timer_id = NULL;
    }

    head_track_send_task_deinit();
    head_track_algo_task_deinit();
    head_track_task_deinit();

    head_track_resouce_deinit();
    return;
}

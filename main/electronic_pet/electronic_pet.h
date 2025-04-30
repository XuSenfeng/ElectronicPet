/*
 * @Descripttion: 
 * @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Xvsenfeng helloworldjiao@163.com
 * Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
 */
#pragma once
#include "time.h"
#include <esp_timer.h>
#include <atomic>
#include <string>
#include <vector>
typedef enum {
    E_PET_TIMER_MESSAGE = 0, // 消息
    E_PET_TIMER_FUNCTION, // 函数
}e_pet_timer_type_e;

typedef struct {  
    void (*callback)(void*);  
    void* arg;  
}e_pet_timer_function;

typedef struct{
    time_t trigger_time;
    e_pet_timer_type_e type;
    int repeat_time;
    union {
        e_pet_timer_function function;
        char *message;
    };
}e_pet_timer_event_t;

class ElectronicPet {
private:
    std::mutex mutex_;
    int vigor_;      // 精神状态
    int satiety;    // 饱食度
    int happiness;  // 快乐度
    int clock_ticks_;  // 时钟
public:

    static ElectronicPet* MyPet;
    esp_timer_handle_t electromic_prt_timer_ = nullptr;
    std::vector<e_pet_timer_event_t> e_pet_timer_events; // 处理事件列表

    ElectronicPet();
    ~ElectronicPet();

    static ElectronicPet* GetInstance();

    void OnClockTimer();
    void timer_event_sort();
    void timer_add_timer_event_relative(int seconds, e_pet_timer_type_e type, void (*callback)(void*), void* arg, bool repeat);
    void timer_add_timer_event_absolute(time_t trigger_time, e_pet_timer_type_e type, void (*callback)(void*), void* arg, bool repeat);
    void timer_add_timer_event_repeat(time_t trigger_time, e_pet_timer_type_e type, void (*callback)(void*), void* arg, int repeat_time);
    void timer_event_process();

    void vigor_add(int vigor);
    void satiety_add(int satiety);
    void happiness_add(int happiness);
};

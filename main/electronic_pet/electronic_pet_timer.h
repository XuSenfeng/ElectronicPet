#pragma once

#include "time.h"
#include <esp_timer.h>
#include <atomic>
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

class ElectronicPetTimer {
private:
    int clock_ticks_;  // 时钟
    std::mutex mutex_;
    esp_timer_handle_t electromic_prt_timer_ = nullptr;
    std::vector<e_pet_timer_event_t> e_pet_timer_events; // 处理事件列表
public:
    ElectronicPetTimer();
    ~ElectronicPetTimer();
    void OnClockTimer();
    void timer_event_sort();
    void timer_add_timer_event_relative(int seconds, e_pet_timer_type_e type, void (*callback)(void*), void* arg, bool repeat);
    void timer_add_timer_event_absolute(time_t trigger_time, e_pet_timer_type_e type, void (*callback)(void*), void* arg, bool repeat);
    void timer_add_timer_event_repeat(time_t trigger_time, e_pet_timer_type_e type, void (*callback)(void*), void* arg, int repeat_time);
    void timer_event_process();
};
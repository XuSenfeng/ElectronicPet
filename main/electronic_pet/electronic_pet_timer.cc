#include "electronic_pet_timer.h"
#include "esp_log.h"
#include "electronic_pet.h"
#include <string>
#include <algorithm>

#define TAG "ElectronicPetTimer"
ElectronicPetTimer::ElectronicPetTimer() {
    clock_ticks_ = 0;
    esp_timer_create_args_t clock_timer_args = {
        .callback = [](void* arg) {
            ElectronicPetTimer*timer = (ElectronicPetTimer*)arg;
            timer->OnClockTimer();
        },
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "e_pet_timer",
        .skip_unhandled_events = true
    };
    esp_timer_create(&clock_timer_args, &electromic_prt_timer_);
    esp_timer_start_periodic(electromic_prt_timer_, 1000000); // 1 second
}

ElectronicPetTimer::~ElectronicPetTimer() {
    if (electromic_prt_timer_ != nullptr) {
        esp_timer_stop(electromic_prt_timer_);
        esp_timer_delete(electromic_prt_timer_);
    }
}

void ElectronicPetTimer::OnClockTimer() {
    std::lock_guard<std::mutex> lock(mutex_);
    timer_event_process();
}



void ElectronicPetTimer::timer_event_sort(){
    std::sort(e_pet_timer_events.begin(), e_pet_timer_events.end(), [](const e_pet_timer_event_t& a, const e_pet_timer_event_t& b) {
        return a.trigger_time < b.trigger_time;
    });
}
/// @brief 设置定时器事件(相对时间)
/// @param seconds 多长时间以后
/// @param type 设置的事件类型
/// @param callback 回调函数
/// @param arg 参数(MESAGEE类型时，传入消息字符串的地址, FUNCTION类型时，传入函数的参数)
void ElectronicPetTimer::timer_add_timer_event_relative(
    int seconds, 
    e_pet_timer_type_e type, 
    void (*callback)(void*), 
    void* arg,
    bool repeat
){
    std::lock_guard<std::mutex> lock(mutex_);
    e_pet_timer_event_t event;
    time_t current_time = time(nullptr);
    event.trigger_time = current_time + seconds;
    event.type = type;
    if (type == E_PET_TIMER_FUNCTION) {
        event.function.callback = callback;
        event.function.arg = arg;
    } else {
        event.message = (char*)arg;
    }
    if(repeat){
        event.repeat_time = seconds;
    }else{
        event.repeat_time = 0;
    }
    e_pet_timer_events.push_back(event);
    timer_event_sort();
    ESP_LOGI(TAG, "Added timer event: %lld, type: %d", event.trigger_time, type);
}

/// @brief 设置定时器事件(绝对时间)
/// @param trigger_time 
/// @param type 
/// @param callback 
/// @param arg 
void ElectronicPetTimer::timer_add_timer_event_absolute(
    time_t trigger_time, 
    e_pet_timer_type_e type, 
    void (*callback)(void*), 
    void* arg,
    bool repeat
){
    std::lock_guard<std::mutex> lock(mutex_);
    e_pet_timer_event_t event;
    event.trigger_time = trigger_time;
    event.type = type;
    if (type == E_PET_TIMER_FUNCTION) {
        event.function.callback = callback;
        event.function.arg = arg;
    } else {
        event.message = (char*)arg;
    }
    if(repeat){
        time_t current_time = time(nullptr);
        event.repeat_time = trigger_time - current_time;
    }else{
        event.repeat_time = 0;
    }
    e_pet_timer_events.push_back(event);
    timer_event_sort();
}

void ElectronicPetTimer::timer_add_timer_event_repeat(
    time_t trigger_time, 
    e_pet_timer_type_e type, 
    void (*callback)(void*), 
    void* arg,
    int repeat_time
){
    std::lock_guard<std::mutex> lock(mutex_);
    e_pet_timer_event_t event;
    event.trigger_time = trigger_time;
    event.type = type;
    if (type == E_PET_TIMER_FUNCTION) {
        event.function.callback = callback;
        event.function.arg = arg;
    } else {
        event.message = (char*)arg;
    }
    event.repeat_time = repeat_time;
    e_pet_timer_events.push_back(event);
    timer_event_sort();
}

void ElectronicPetTimer::timer_event_process(){
    time_t current_time = time(nullptr);
    for (auto it = e_pet_timer_events.begin(); it != e_pet_timer_events.end();) {
        if (it->trigger_time <= current_time) {
            if (it->type == E_PET_TIMER_FUNCTION) {
                it->function.callback(it->function.arg);
            } else {
                ESP_LOGI(TAG, "Message: %s", it->message);
            }
            if(it->repeat_time > 0){
                it->trigger_time += it->repeat_time;
            }else{
                it = e_pet_timer_events.erase(it); // 删除已处理的事件
            }
        } else {
            break;
        }
    }
    timer_event_sort();
}



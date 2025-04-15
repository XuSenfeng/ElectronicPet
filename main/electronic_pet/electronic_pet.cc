/*
 * @Descripttion: 
 * @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Xvsenfeng helloworldjiao@163.com
 * Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
 */
#include "electronic_pet.h"
#include "esp_log.h"
#include "application.h"
#define TAG "ElectronicPet"

ElectronicPet::ElectronicPet(){
    clock_ticks_ = 0;
    ESP_LOGI(TAG, "ElectronicPet constructor");
    vigor = 100;
    satiety = 100;
    happiness = 100;
    esp_timer_create_args_t clock_timer_args = {
        .callback = [](void* arg) {
            ElectronicPet* e_pet = (ElectronicPet*)arg;
            e_pet->OnClockTimer();
        },
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "e_pet_timer",
        .skip_unhandled_events = true
    };
    esp_timer_create(&clock_timer_args, &electromic_prt_timer_);
    esp_timer_start_periodic(electromic_prt_timer_, 1000000); // 1 second

    timer_add_timer_event_relative(5, E_PET_TIMER_MESSAGE, NULL, (void*)"Hello from timer event!");
    timer_add_timer_event_relative(10, E_PET_TIMER_FUNCTION, [](void* arg) {
        ESP_LOGI(TAG, "Function callback triggered!");
    }, NULL);
}

ElectronicPet::~ElectronicPet(){
    ESP_LOGI(TAG, "ElectronicPet destructor");
    // Stop and delete the timer
    if (electromic_prt_timer_ != nullptr) {
        esp_timer_stop(electromic_prt_timer_);
        esp_timer_delete(electromic_prt_timer_);
    }
}

void ElectronicPet::OnClockTimer() {
    std::lock_guard<std::mutex> lock(mutex_);
    timer_event_process();
}

ElectronicPet* ElectronicPet::GetInstance() {
    return Application::GetInstance().GetMyPet();
}

void ElectronicPet::timer_event_sort(){
    std::sort(e_pet_timer_events.begin(), e_pet_timer_events.end(), [](const e_pet_timer_event_t& a, const e_pet_timer_event_t& b) {
        return a.trigger_time < b.trigger_time;
    });
}
/// @brief 设置定时器事件(相对时间)
/// @param seconds 多长时间以后
/// @param type 设置的事件类型
/// @param callback 回调函数
/// @param arg 参数(MESAGEE类型时，传入消息字符串的地址, FUNCTION类型时，传入函数的参数)
void ElectronicPet::timer_add_timer_event_relative(int seconds, e_pet_timer_type_e type, void (*callback)(void*), void* arg){
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
    e_pet_timer_events.push_back(event);
    timer_event_sort();
    ESP_LOGI(TAG, "Added timer event: %lld, type: %d", event.trigger_time, type);
}
/// @brief 设置定时器事件(绝对时间)
/// @param trigger_time 
/// @param type 
/// @param callback 
/// @param arg 
void ElectronicPet::timer_add_timer_event_absolute(time_t trigger_time, e_pet_timer_type_e type, void (*callback)(void*), void* arg){
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
    e_pet_timer_events.push_back(event);
    timer_event_sort();
}

void ElectronicPet::timer_event_process(){
    time_t current_time = time(nullptr);
    ESP_LOGI(TAG, "Current time: %lld", current_time);
    for (auto it = e_pet_timer_events.begin(); it != e_pet_timer_events.end();) {
        if (it->trigger_time <= current_time) {
            if (it->type == E_PET_TIMER_FUNCTION) {
                it->function.callback(it->function.arg);
            } else {
                ESP_LOGI(TAG, "Message: %s", it->message);
            }
            it = e_pet_timer_events.erase(it); // 删除已处理的事件
        } else {
            break;
        }
    }
}
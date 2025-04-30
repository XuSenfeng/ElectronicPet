#include "electronic_pet_timer.h"
#include "esp_log.h"
#include "electronic_pet.h"
#include <string>
#include <algorithm>
#include <stdio.h>
#include <cstring>
#include "stdlib.h"
#define MOUNT_POINT              "/sdcard"
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
    timer_read_csv_timer();
}



// 计算最大公约数
long gcd(long a, long b) {
    return b == 0 ? a : gcd(b, a % b);
}

// 计算最小公倍数
long lcm(long a, long b) {
    return (a * b) / gcd(a, b);
}

void calculate_next_trigger(
    int tm_sec, int tm_min, int tm_hour,
    int re_mday, int re_mon, int re_year,
    int re_wday, long* delta_sec, long* interval_sec) {
    
    time_t now;
    time(&now);
    struct tm* current = localtime(&now);
    printf("Current time: %02d:%02d:%02d %02d/%02d/%04d\n",
        current->tm_hour, current->tm_min, current->tm_sec,
        current->tm_mday, current->tm_mon + 1, current->tm_year + 1900);

    struct tm next = *current;

    // 初始化周期参数
    long interval = 1;
    int has_periodic = 0;

    // 修正后的宏定义：增加参数分离字段名和变量名
    #define PROCESS_FIELD(field_suffix, var, max, unit_sec) \
        if (var < 0) { \
            interval = lcm(interval, labs(var) * (unit_sec)); \
            has_periodic = 1; \
        } else if (var >= 0 && var <= max) { \
            next.tm_##field_suffix = var; \
        }

    // 修正调用方式：明确分离结构体字段后缀和变量名
    PROCESS_FIELD(sec, tm_sec, 59, 1)
    PROCESS_FIELD(min, tm_min, 59, 60)
    PROCESS_FIELD(hour, tm_hour, 23, 3600)

    // 处理日期字段
    if (re_mday < 0) {
        interval = lcm(interval, labs(re_mday) * 86400L);
        has_periodic = 1;
    } else if (re_mday > 0) {
        next.tm_mday = re_mday;
    }

    if (re_mon < 0) {
        interval = lcm(interval, labs(re_mon) * 2592000L); // 近似30天
        has_periodic = 1;
    } else if (re_mon > 0) {
        next.tm_mon = re_mon - 1;
    }

    if (re_year < 0) {
        interval = lcm(interval, labs(re_year) * 31536000L); // 近似365天
        has_periodic = 1;
    } else if (re_year > 0) {
        next.tm_year = re_year - 1900;
    }

    // 处理星期字段
    if (re_wday < 0) {
        interval = lcm(interval, labs(re_wday) * 604800L);
        has_periodic = 1;
    } else if (re_wday >= 0 && re_wday <= 7) {
        next.tm_wday = re_wday % 7;
    }

    // 计算初始候选时间
    next.tm_isdst = -1;
    printf("Next trigger time: %02d:%02d:%02d %02d/%02d/%04d\n",
           next.tm_hour, next.tm_min, next.tm_sec,
           next.tm_mday, next.tm_mon + 1, next.tm_year + 1900);
    time_t candidate = mktime(&next);

    // 自动调整策略
    while (1) {
        // 处理时间已过的情况
        if (candidate <= now) {
            // printf("Candidate time has passed, adjusting...\n");
            if (has_periodic) {
                candidate += interval;
            } else {
                // 单次事件已过期
                *delta_sec = -1;
                *interval_sec = 0;
                return;
            }
            continue;
        }
        break;
    }

    *delta_sec = candidate - now;
    *interval_sec = has_periodic ? interval : 0;
}

void ElectronicPetTimer::deal_one_csv_message(csv_info_t *csv_info, long *delta_sec, long *interval_sec){
    calculate_next_trigger(
        csv_info->tm_sec, csv_info->tm_min, csv_info->tm_hour,
        csv_info->re_mday, csv_info->re_mon, csv_info->re_year,
        csv_info->re_wday, delta_sec, interval_sec);
    
}



void ElectronicPetTimer::timer_read_csv_timer(){
    const char *file_hello = MOUNT_POINT"/electronic_pet.csv";
    ESP_LOGI(TAG, "Reading file: %s", file_hello);
    // 打开文件
    FILE *f = fopen(file_hello, "r");  // 以只读方式打开文件
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        // 处理每一行数据
        // ESP_LOGI(TAG, "Read line: %s", line);

        // 解析数据并添加定时器事件
        //tm_sec,tm_min,tm_hour,re_mday,re_mon,re_year,re_wday,random_l, random_h,function_id,message
        //0,30,7,0,0,0,-1,0,0,0,起床时间到，别忘了你的早八
        csv_info_t csv_info;

        int ret = sscanf(line, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%[^\n]", 
            &csv_info.tm_sec, &csv_info.tm_min, &csv_info.tm_hour, 
            &csv_info.re_mday, &csv_info.re_mon, &csv_info.re_year,
            &csv_info.re_wday, &csv_info.random_l, &csv_info.random_h,
            &csv_info.function_id, csv_info.message);
        if (ret == 11) {
            long delta_sec = 0;
            long interval_sec = 0;
            deal_one_csv_message(&csv_info, &delta_sec, &interval_sec);
            char *message = (char*)malloc(strlen(csv_info.message) + 1);
            if (message == NULL) {
                ESP_LOGE(TAG, "Failed to allocate memory for message");
                continue;
            }
            strcpy(message, csv_info.message);
            timer_add_timer_event_repeat(
                time(nullptr) + delta_sec, E_PET_TIMER_MESSAGE, 
                NULL, 
                (void*)message, interval_sec);
        } else {
            ESP_LOGE(TAG, "Failed to parse line: %s", line);
        }
    }
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
                if(it->type == E_PET_TIMER_FUNCTION){
                    // free(it->function.arg); // 释放函数参数
                }else{
                    free(it->message); // 释放消息字符串
                }

                it = e_pet_timer_events.erase(it); // 删除已处理的事件
            }
        } else {
            break;
        }
    }
    timer_event_sort();
}



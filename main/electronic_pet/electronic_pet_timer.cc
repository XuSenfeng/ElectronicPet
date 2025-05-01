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

// 辅助函数：判断闰年
int is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// 辅助函数：获取指定月份的天数
int get_month_days(int year, int month) {
    static const int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (month == 1 && is_leap_year(year)) 
        return 29;
    return days[month];
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
        if (!has_periodic &&  var < 0) { \
            interval = lcm(interval, labs(var) * (unit_sec)); \
            has_periodic = 1; \
        } else if (!has_periodic && var >= 0 && var <= max) { \
            next.tm_##field_suffix = var; \
        }

    // 修正调用方式：明确分离结构体字段后缀和变量名
    PROCESS_FIELD(sec, tm_sec, 59, 1)
    PROCESS_FIELD(min, tm_min, 59, 60)
    PROCESS_FIELD(hour, tm_hour, 23, 3600)

    if (re_wday != 0) { // 0表示不设置星期条件
        // 转换输入范围：0-7 → 0-6（周日=0）
        int target_wday = (re_wday < 0) ? labs(re_wday) % 7 : re_wday % 7;
        
        // 处理周期性（负数表示周期）
        if (re_wday < 0) {
            // 计算周周期（绝对值×7天的秒数）
            long week_interval = 604800L; 
            interval = lcm(interval, week_interval);
            has_periodic = 1;
            
            // 设置初始触发日为下一个目标星期几
            int days_to_add = (target_wday - next.tm_wday + 7) % 7;
            days_to_add = (days_to_add == 0) ? 7 : days_to_add; // 确保至少增加1天
            next.tm_mday += days_to_add;
        } 
        // 处理单次指定（正数）
        else if (re_wday > 0) {
            // 计算需要增加的天数（考虑跨周情况）
            int days_diff = (target_wday - next.tm_wday + 7) % 7;
            // 如果当天已过目标星期几，则跳到下周
            if (days_diff == 0 && mktime(&next) <= now) {
                days_diff = 7;
            }
            next.tm_mday += days_diff;
        }
        
    }

    // 处理日期字段
    if (!has_periodic &&  re_mday < 0) {
        interval = lcm(interval, labs(re_mday) * 86400L);
        has_periodic = 1;
    } else if (!has_periodic &&  re_mday > 0) {
        printf("Setting day of month: %d\n", re_mday);
        next.tm_mday = re_mday;
    }

    if (!has_periodic &&  re_mon < 0) { // 月周期处理
        if(mktime(&next) < now) {
            next.tm_mon += labs(re_mon);
        }

        int year = next.tm_year + 1900 + next.tm_mon / 12;
        int month = next.tm_mon % 12;

        interval = lcm(interval, labs(re_mon) * get_month_days(year, month) *86400L); // 近似值
        // 调整日期到有效值

        int max_day = get_month_days(year, month);
        if (next.tm_mday > max_day)
            next.tm_mday = max_day;
        has_periodic = 1;
    } else if (!has_periodic &&  re_mon > 0) {
        next.tm_mon = re_mon - 1;
    }

    if (!has_periodic &&  re_year < 0) { // 年周期处理
        int year = next.tm_year + 1900;
        interval = lcm(interval, labs(re_year) * (is_leap_year(year) ? 366 : 365) * 86400L); // 365.25天
        if(mktime(&next) < now) {
            next.tm_year += labs(re_year);
        }
        // 闰年调整
        if (next.tm_mon == 1) { // 二月
            int max_day = is_leap_year(year) ? 29 : 28;
            if (next.tm_mday > max_day)
                next.tm_mday = max_day;
        }
        has_periodic = 1;
    } else if (!has_periodic &&  re_year > 0) {
        next.tm_year = re_year - 1900;
    }

    // 计算初始候选时间
    next.tm_isdst = -1;
    time_t candidate = mktime(&next);
    printf("Next time (local): %02d:%02d:%02d %02d/%02d/%04d\n",
           next.tm_hour, next.tm_min, next.tm_sec,
           next.tm_mday, next.tm_mon + 1, next.tm_year + 1900);
    // 自动调整策略
    while (1) {
        // 处理时间已过的情况
        if (candidate <= now) {
            // printf("Candidate time has passed, adjusting...\n");
            if (has_periodic) {
                candidate += interval;
                printf("Adjusted candidate time: %lld %lld\n", candidate, now);
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

    struct tm* candidate_tm = localtime(&candidate);
    // 处理时区差异
    printf("Candidate time (local): %02d:%02d:%02d %02d/%02d/%04d  day:%ld\n hour: %ld, min: %ld, sec: %ld\n",
           candidate_tm->tm_hour, candidate_tm->tm_min, candidate_tm->tm_sec,
           candidate_tm->tm_mday, candidate_tm->tm_mon + 1, candidate_tm->tm_year + 1900, interval / 60 /60 / 24, interval / 60 /60, interval / 60, interval);

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



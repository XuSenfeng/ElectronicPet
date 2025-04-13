/*
 * @Descripttion: 
 * @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Xvsenfeng helloworldjiao@163.com
 * Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
 */
#pragma once

#include <esp_timer.h>
#include <atomic>
class ElectronicPet {
    std::mutex mutex_;
public:
    int vigor;      // 精神状态
    int satiety;    // 饱食度
    int happiness;  // 快乐度
    int clock_ticks_;
    static ElectronicPet* MyPet;

    esp_timer_handle_t electromic_prt_timer_ = nullptr;

    ElectronicPet();
    ~ElectronicPet();
    void OnClockTimer();
    ElectronicPet* GetInstance();
    
};

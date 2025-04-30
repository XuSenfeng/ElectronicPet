/*
 * @Descripttion: 
 * @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Xvsenfeng helloworldjiao@163.com
 * Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
 */
/*
 * @Descripttion: 
 * @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Xvsenfeng helloworldjiao@163.com
 * Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
 */
#pragma once

#include <atomic>
#include <string>
#include "electronic_pet_timer.h"

class ElectronicPet {
private:
    std::mutex mutex_;
    int vigor_;      // 精神状态
    int satiety;    // 饱食度
    int happiness;  // 快乐度
    
public:
    ElectronicPetTimer timer;
    static ElectronicPet* MyPet;
    ElectronicPet();
    ~ElectronicPet();
    
    static ElectronicPet* GetInstance();



    void vigor_add(int vigor);
    void satiety_add(int satiety);
    void happiness_add(int happiness);
};

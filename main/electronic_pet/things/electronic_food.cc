/*
 * @Descripttion: 
 * @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Xvsenfeng helloworldjiao@163.com
 * Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
 */

#include "electronic_food.h"
#include "electronic_pet.h"
#include "application.h"

void Food::Use() {
    // 使用食物的逻辑
    ElectronicPet* pet = ElectronicPet::GetInstance();
    int state[E_PET_STATE_NUMBER] = {0};
    state[E_PET_STATE_SATITY] = satiety; // 饱食度
    state[E_PET_STATE_VIGIR] = vigor; // 精神状态
    pet->change_statue(state); // 改变宠物的状态
    // 发送消息给小智
    std::string message = "使用了" + name + ", 效果是" + thing_description +"，饱食度增加" + std::to_string(satiety) + "，精神状态增加" + std::to_string(vigor);
    // 发送消息给小智的逻辑
    auto &app = Application::GetInstance();
    app.SendMessage(message);
}


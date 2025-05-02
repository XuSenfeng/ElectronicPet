/*
 * @Descripttion: 
 * @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Xvsenfeng helloworldjiao@163.com
 * Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
 */
#include "electronic_pet.h"
#include "esp_log.h"
#include "application.h"
#include "settings.h"
#include "string.h"
#include "electronic_food.h"
#define TAG "ElectronicPet"

ElectronicPet::ElectronicPet(){
    ESP_LOGI(TAG, "ElectronicPet constructor");
    Settings settings("e_pet", true);
    for(int i = 0; i < E_PET_STATE_NUMBER; i++){
        state_[i] = settings.GetInt("state_" + std::to_string(i), 100);
    }
    action_ = (electronic_pet_action_e)settings.GetInt("action", E_PET_ACTION_IDLE);
    Food food("food", {0}, "饱食度增加10", 10, 0);

    foods_.push_back(food);
    timer.timer_add_timer_event_relative(10, E_PET_TIMER_FUNCTION, [](void* arg) {
        ElectronicPet* pet = (ElectronicPet*)arg;
        Food food = pet->foods_[0];
        food.Use();
        int now_num = food.GetNum();
        if(now_num <= 0){
            pet->foods_.erase(pet->foods_.begin());
        }else{
            pet->foods_[0].SetNum(now_num);
        }
    }, this, false);
    // char* temp1 = (char*)malloc(strlen("<摸一摸喵喵的头>") + 1);
    // strcpy(temp1, "<摸一摸喵喵的头>");
    // char* temp2 = (char*)malloc(strlen("[喵喵茶几喜欢喜欢你]") + 1);
    // strcpy(temp2, "[喵喵茶几喜欢喜欢你]");
    // timer.timer_add_timer_event_relative(20, E_PET_TIMER_MESSAGE, NULL, (void*)temp1, false);
    // timer.timer_add_timer_event_relative(30, E_PET_TIMER_MESSAGE, NULL, (void*)temp2, false);
}

ElectronicPet::~ElectronicPet(){
    ESP_LOGI(TAG, "ElectronicPet destructor");
}


ElectronicPet* ElectronicPet::GetInstance() {
    return Application::GetInstance().GetMyPet();
}


void ElectronicPet::change_statue(int *change_state){
    std::lock_guard<std::mutex> lock(mutex_);
    for(int i = 0; i < E_PET_STATE_NUMBER; i++){
        state_[i] += change_state[i];
        if(state_[i] > 100) state_[i] = 100;
        if(state_[i] < 0) state_[i] = 0;
    }
    Settings settings("e_pet", true);
    for(int i = 0; i < E_PET_STATE_NUMBER; i++){
        settings.SetInt("state_" + std::to_string(i), state_[i]);
    }
}


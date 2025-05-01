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
#include "electronic_food.h"
#define TAG "ElectronicPet"

ElectronicPet::ElectronicPet(){
    ESP_LOGI(TAG, "ElectronicPet constructor");
    Settings settings("e_pet", true);
    for(int i = 0; i < E_PET_STATE_NUMBER; i++){
        state_[i] = settings.GetInt("state_" + std::to_string(i), 100);
    }
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


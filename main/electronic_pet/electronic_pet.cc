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

    ESP_LOGI(TAG, "ElectronicPet constructor");
    vigor_ = 100;
    satiety = 100;
    happiness = 100;
    // timer_add_timer_event_relative(5, E_PET_TIMER_MESSAGE, NULL, (void*)"Hello from timer event!", 1);
    // timer_add_timer_event_relative(10, E_PET_TIMER_FUNCTION, [](void* arg) {
    //     ESP_LOGI(TAG, "Function callback triggered!");
    // }, NULL, 0);
    // time_t time_n = time(nullptr);
    // timer.timer_add_timer_event_repeat(time_n + 5, E_PET_TIMER_MESSAGE, NULL, (void*)"Hello from timer event!", 10);
}

ElectronicPet::~ElectronicPet(){
    ESP_LOGI(TAG, "ElectronicPet destructor");
    // Stop and delete the timer
}





ElectronicPet* ElectronicPet::GetInstance() {
    return Application::GetInstance().GetMyPet();
}

void ElectronicPet::vigor_add(int vigor){
    vigor_ += vigor; 
    if(vigor_ > 100) 
        vigor_ = 100; 
    if (vigor_ < 0) 
        vigor_ = 0;
}

void ElectronicPet::satiety_add(int satiety){
    satiety += satiety; 
    if(satiety > 100) 
        satiety = 100; 
    if (satiety < 0) 
        satiety = 0;
}
void ElectronicPet::happiness_add(int happiness){
    happiness += happiness; 
    if(happiness > 100) 
        happiness = 100; 
    if (happiness < 0) 
        happiness = 0;
}
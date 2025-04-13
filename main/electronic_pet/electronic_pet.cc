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
    clock_ticks_++;
    ESP_LOGI(TAG, "clock_ticks_ = %d", clock_ticks_);
}

ElectronicPet* ElectronicPet::GetInstance() {
    return Application::GetInstance().GetMyPet();
}
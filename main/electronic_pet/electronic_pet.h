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
#include "electronic_food.h"
#include "electronic_pet_timer.h"
#include <vector>
typedef enum{
    E_PET_ACTION_IDLE = 0, // 空闲
    E_PET_ACTION_PLAY, // 玩耍
    E_PET_ACTION_SLEEP, // 睡觉
    E_PET_ACTION_WALK, // 散步
    E_PET_ACTION_BATH, // 洗澡
    E_PET_ACTION_WORK, // 工作
    E_PET_ACTION_STUDY, // 学习
    E_PET_ACTION_PLAY_MUSIC, // 听音乐
    E_PET_ACTION_NUMBER // 状态数量
}electronic_pet_action_e;

typedef enum{
    E_PET_STATE_VIGIR = 0, // 精力
    E_PET_STATE_SATITY, // 饱食度
    E_PET_STATE_HAPPINESS, // 快乐度
    E_PET_STATE_NUMBER // 状态数量
}electronic_pet_state_e;


class ElectronicPet {
private:
    std::mutex mutex_;
    int state_[E_PET_STATE_NUMBER];     // 当前状态
    
    std::vector<Food> foods_; // 物品列表
    electronic_pet_action_e action_; // 当前行动
public:
    static int state_time_change_[E_PET_ACTION_NUMBER][E_PET_STATE_NUMBER]; // 不同状态下边宠物的状态变化
    ElectronicPetTimer timer;
    static ElectronicPet* MyPet;
    ElectronicPet();
    ~ElectronicPet();
    int GetAction() const { return action_; }
    void SetAction(int action) { action_ = (electronic_pet_action_e)action; }
    int GetState(int state) const { return state_[state]; }
    void SetState(int state, int value) { state_[state] = value; }
    static ElectronicPet* GetInstance();

    void change_statue(int *change_state);
};

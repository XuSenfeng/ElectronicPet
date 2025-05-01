/*
 * @Descripttion: 
 * @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Xvsenfeng helloworldjiao@163.com
 * Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
 */

#pragma once

#include <string>
#include "lvgl.h"
#include "electronic_base_thing.h"


class Food: public BaseThing {
private:
    int satiety; // 饱食度
    int vigor; // 精神状态
public:
    Food(std::string name, lv_image_dsc_t thing_pic, std::string thing_description, int satiety, int vigor, int num = 1)
        : BaseThing(name, thing_pic, thing_description, E_THING_TYPE_FOOD, num), satiety(satiety), vigor(vigor) {}
    void Use() override;
};




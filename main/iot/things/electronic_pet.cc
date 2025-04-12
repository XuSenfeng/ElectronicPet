/*
 * @Descripttion: 
 * @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Xvsenfeng helloworldjiao@163.com
 * Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
 */
#include "iot/thing.h"
#include "board.h"
#include "display/lcd_display.h"
#include "settings.h"

#include <esp_log.h>
#include <string>

#define TAG "ElectronicPet"

namespace iot {

// 这里仅定义 ElectronicPet 的属性和方法，不包含具体的实现
class ElectronicPet : public Thing {
public:
    ElectronicPet() : Thing("ElectronicPet", "电子宠物相关的状态以及控制部分") {
        // 定义设备的属性
        properties_.AddNumberProperty("vigor", "宠物的精神状态,低于30的时候需要睡觉, 是一个0-100之间的数字", [this]() -> int {
            return 90;
        });

        properties_.AddNumberProperty("satiety", "当前的饱食度,低于30的时候要提醒主人, 是一个0-100之间的数字", [this]() -> int {
            return 10;
        });
    }
};

} // namespace iot

DECLARE_THING(ElectronicPet);
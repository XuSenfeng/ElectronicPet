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

#include <string>
#include "lvgl.h"
typedef enum{
    E_THING_TYPE_FOOD = 0,
    E_THING_TYPE_TOY,
    E_THING_TYPE_CLOTHES,
}thing_type_e;

class BaseThing{
public:
    BaseThing(std::string name, lv_image_dsc_t thing_pic, std::string thing_description, thing_type_e thing_type)
        : name(name), thing_pic(thing_pic), thing_description(thing_description), thing_type(thing_type) {}

    virtual ~BaseThing() = default;

    virtual void Use() = 0; // 使用物品
protected:
    std::string name;
    lv_image_dsc_t thing_pic;
    std::string thing_description; // 使用之后发送给小智
    thing_type_e thing_type;
};
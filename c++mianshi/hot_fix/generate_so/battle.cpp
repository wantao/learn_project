#include "battle.h"
#include <string>

// 计算伤害（C++ 逻辑）
int battle_calculate_damage(int attack, int defense) {
    int damage = attack - defense;
    return (damage > 0) ? damage : 0;
}

// 战斗结果（C++ 字符串）
const char* battle_result(int damage) {
    if (damage == 0) {
        return "格挡成功，无伤害";
    } else if (damage < 30) {
        return "轻攻击";
    } else if (damage < 60) {
        return "普通攻击";
    } else {
        return "暴击！！！";
    }
}
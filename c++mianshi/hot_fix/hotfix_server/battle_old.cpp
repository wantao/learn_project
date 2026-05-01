#include "logic_api.h"

// 必须 extern "C" 防止名字改编
extern "C"
int calculate_damage(int playerHp, int enemyAtk, int def)
{
    // 旧逻辑：简单减伤
    int real = enemyAtk - def;
    return real > 0 ? playerHp - real : playerHp;
}
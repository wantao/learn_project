#include "logic_api.h"

extern "C"
int calculate_damage(int playerHp, int enemyAtk, int def)
{
    // 新逻辑：加暴击、保底伤害
    int crit = 1.2;
    int real = (int)(enemyAtk * crit) - def;
    if (real < 1) real = 1;
    return playerHp - real;
}
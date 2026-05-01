#ifndef LOGIC_API_H
#define LOGIC_API_H

// 战斗逻辑函数签名
typedef int (*CalcDamageFunc)(int playerHp, int enemyAtk, int def);

// 全局原子函数指针声明
#include <atomic>
extern std::atomic<CalcDamageFunc> g_calc_damage;

#endif
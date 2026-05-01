#ifndef BATTLE_H
#define BATTLE_H

// C++ 导出函数必须加 extern "C"，否则无法被正常链接
#ifdef __cplusplus
extern "C" {
#endif

// 计算伤害
int battle_calculate_damage(int attack, int defense);

// 返回战斗结果
const char* battle_result(int damage);

#ifdef __cplusplus
}
#endif

#endif
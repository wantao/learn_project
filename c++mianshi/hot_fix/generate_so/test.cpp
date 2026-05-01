#include <iostream>
#include "battle.h"

using namespace std;

int main() {
    int attack = 80;
    int defense = 20;

    // 调用动态库函数
    int damage = battle_calculate_damage(attack, defense);
    const char* res = battle_result(damage);

    cout << "攻击力: " << attack << endl;
    cout << "防御力: " << defense << endl;
    cout << "伤害值: " << damage << endl;
    cout << "战斗结果: " << res << endl;

    return 0;
}
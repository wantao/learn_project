#include <lua.hpp>
int main() {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    // C++调用Lua函数
    luaL_dofile(L, "game_logic.lua");
    lua_getglobal(L, "player_level_up");
    lua_call(L, 0, 1);
    const char* res = lua_tostring(L, -1);
    printf("%s\n", res);  // 输出：玩家升级成功
    lua_close(L);
    return 0;
}
// 编译：g++ -o lua_test lua_test.cpp -llua
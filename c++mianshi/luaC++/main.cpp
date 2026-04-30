extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <iostream>
using namespace std;

extern "C" {

// 给 Lua 调用的加法函数
int add(lua_State *L) {
    int a = (int)lua_tonumber(L, 1);
    int b = (int)lua_tonumber(L, 2);
    lua_pushnumber(L, a + b);
    return 1;
}

// 给 Lua 调用的打印函数
int showMsg(lua_State *L) {
    const char *msg = lua_tostring(L, 1);
    cout << "[C++] " << msg << endl;
    return 0;
}

} // end extern "C"

int main() {
    // 创建Lua虚拟机
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    // 注册函数到Lua全局
    lua_pushcfunction(L, add);
    lua_setglobal(L, "add");

    lua_pushcfunction(L, showMsg);
    lua_setglobal(L, "showMsg");

    // 运行Lua脚本
    if (luaL_dofile(L, "game.lua")) {
        cerr << "Lua 错误: " << lua_tostring(L, -1) << endl;
    }

    lua_close(L);
    return 0;
}
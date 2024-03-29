#pragma once

#if defined(_WIN32) && defined(SCRIPT_LUA)
#include "ModWin.h"
#include "Memory.h"
#include <string>
#include <vector>
#include "Log.h"

extern "C"
{
#include "lualib/include/lua.h"
#include "lualib/include/lauxlib.h"
#include "lualib/include/lualib.h"
}

#pragma comment(lib, "lualib/lua54.lib")

namespace ingenium_lua {

	extern lua_State* state;

	void initLua();
	void loadFileA(const char* file);
	bool executeChunk();
	bool executeFunc(const char* function);
	void stopLua();
	std::string getStackTrace();

	luaL_Reg lua_func(const char* name, lua_CFunction cf);

	template <typename T>
	static inline int free(lua_State* lua)
	{
		memory::safe_delete(*(T**)lua_touserdata(lua, 1));
		return 0;
	}

	static inline void printStackTrace() {
		Debug::oss << getStackTrace();
		Debug::writeLn();
	}

	template <typename T>
	struct LuaClass {
		const char* name;
		const char* metaName;
		std::vector<luaL_Reg> methods = { };
		std::vector<luaL_Reg> metaMethods = { };

		std::string stdMetaName = std::string("Ingenium.");

		bool registering = false;

		inline LuaClass(const char* name_)
		{
			name = name_;
			stdMetaName = stdMetaName.append(name);
			metaName = stdMetaName.c_str();
		};
		inline void addMethod(lua_State* lua, luaL_Reg fn)
		{
			methods.push_back(fn);
		}
		inline void addMetaMethod(lua_State* lua, luaL_Reg fn)
		{
			metaMethods.push_back(fn);
		}
		inline T** newUserData(lua_State* lua, T* value)
		{
			T** tPP = (T**)lua_newuserdata(lua, sizeof(T*));
			*tPP = value;
			return tPP;
		};
		inline void createInstance(lua_State* lua, T* instanceValue)
		{
			lua_newtable(lua); // Create new table
			lua_pushvalue(lua, 1); // Create copy of the metatable passed in
			lua_setmetatable(lua, -2); // Set the metatable of the table we created to the copied one
			lua_pushvalue(lua, 1);
			lua_setfield(lua, 1, "__index");

			newUserData(lua, instanceValue);
			luaL_getmetatable(lua, metaName);
			lua_setmetatable(lua, -2);
			lua_setfield(lua, -2, "__self");
		};
		inline void registerMetatable(lua_State* lua) {
			registering = true;
			luaL_Reg nullFn = lua_func(nullptr, nullptr);
			metaMethods.push_back(nullFn);
			methods.push_back(nullFn);

			luaL_newmetatable(lua, metaName);
			luaL_setfuncs(lua, metaMethods.data(), 0);
			luaL_setfuncs(lua, methods.data(), 0);
		};
		inline void registerTable(lua_State* lua) {
			registering = false;

			lua_newtable(lua);
			luaL_setfuncs(lua, methods.data(), 0);
			lua_setfield(lua, -2, "__index");
			lua_setglobal(lua, name);
		};
		inline void registerClass(lua_State* lua)
		{
			registerMetatable(lua);
			registerTable(lua);
		};
	};
}

#endif
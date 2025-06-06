// Base Application taken from Klusark (GPLv2)
// https://code.google.com/archive/p/mafia2injector/

/*
 * Copyright (c) 2010 Barzakh (martinjk 'at' outlook 'dot' com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.

 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.

 * 3. This notice may not be removed or altered from any source
 * distribution.

 * 4. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <Export.h>
#include <Common.h>
#include <sstream>
#include <LuaFunctions.h>
#include <M2DEScriptHook.h>
#include <cstdint>
#include <hooking/hooking.h>
#include <fstream>
#include <thread>
#include "PatternScanner.h"

/************************************************************************/
/* Lua Load Buffer impl													*/
/************************************************************************/
typedef int32_t(__cdecl *luaL_loadbuffer_t)(lua_State *L, char *buff, size_t size, char *name);
luaL_loadbuffer_t		pluaL_loadbuffer = nullptr;

__declspec(dllexport) int32_t luaL_loadbuffer_(lua_State *L, char *buff, size_t size, char *name)
{
	return pluaL_loadbuffer(L, buff, size, name);
}

/************************************************************************/
/* Lua pcall implementation                                             */
/************************************************************************/
typedef int32_t(__cdecl *lua_pcall_t)(lua_State *L, int32_t nargs, int32_t nresults, int32_t errfunc);
lua_pcall_t				plua_pcall2 = nullptr;

__declspec(dllexport) int32_t lua_pcall_(lua_State *L, int32_t nargs, int32_t nresults, int32_t errfunc)
{
	return plua_pcall2(L, nargs, nresults, errfunc);
}

class C_ScriptMachineManager* g_scriptMachineManager;

/************************************************************************/
/* Lua tolstring implementation                                         */
/************************************************************************/
typedef const char *	(__cdecl *lua_tostring_t) (lua_State *L, int32_t idx, uintptr_t*);
lua_tostring_t			plua_tostring = nullptr;

__declspec(dllexport) const char *lua_tostring_(lua_State *L, int32_t idx)
{
	return plua_tostring(L, idx, nullptr);
}

/************************************************************************/
/* Lua isstring implementation                                         */
/************************************************************************/
typedef uint32_t(__cdecl *lua_isstring_t) (lua_State *L, int32_t idx);
lua_isstring_t			plua_isstring = nullptr;

__declspec(dllexport) uint32_t lua_isstring_(lua_State *L, int32_t idx)
{
	return plua_isstring(L, idx);
}

/************************************************************************/
/* Lua newthread implementation                                         */
/************************************************************************/
typedef	lua_State *		(__cdecl *lua_newthread_t) (lua_State *L);
lua_newthread_t		plua_newthread = nullptr;

__declspec(dllexport) lua_State *lua_newthread_(lua_State *L)
{
	return plua_newthread(L);
}

/************************************************************************/
/* Lua pushcclosure implementation                                      */
/************************************************************************/
typedef	lua_State *		(__cdecl *lua_pushcclosure_t) (lua_State *L, lua_CFunction fn, int n);
lua_pushcclosure_t		plua_pushcclosure = nullptr;

__declspec(dllexport) lua_State *lua_pushcclosure_(lua_State *L, lua_CFunction fn, int n)
{
	return plua_pushcclosure(L, fn, n);
}

/************************************************************************/
/* Lua setglobal implementation		                                    */
/************************************************************************/
typedef	lua_State *		(__cdecl *lua_setglobal_t) (lua_State *L, const char *var);
lua_setglobal_t		  plua_setglobal = nullptr;

__declspec(dllexport) lua_State *lua_setglobal_(lua_State *L, const char *var)
{
	return plua_setglobal(L, var);
}

/************************************************************************/
/* Lua setfield implementation		                                    */
/************************************************************************/
typedef	lua_State *		(__cdecl *lua_setfield_t) (lua_State *L, int idx, const char *k);
lua_setfield_t		  plua_setfield = nullptr;

__declspec(dllexport) lua_State *lua_setglobal_(lua_State *L, int idx, const char *k)
{
	return plua_setfield(L, idx, k);
}

/************************************************************************/
/* (Havok) Lua gettop implementation			                        */
/************************************************************************/
int32_t lua_gettop_(lua_State *L)
{
	return (int32_t)((*(uintptr_t *)((uintptr_t)L + 72) - *(uintptr_t *)((uintptr_t)L + 80)) >> 4);
}

//

__declspec(dllexport) void logPointer(std::string name, void* pointer)
{
	if (pointer == NULL)
	{
		std::stringstream ss;
		ss << name << "Failed to find: " << name;
		M2DEScriptHook::instance()->Log(ss.str());
		return;
	}

	// Get the base address of the main module
	uint64_t baseAddress = reinterpret_cast<uint64_t>(GetModuleHandle(nullptr));
	uint64_t pointerAddress = reinterpret_cast<uint64_t>(pointer);

	// Calculate the offset
	uint64_t offset = pointerAddress - baseAddress;

	// Format the log message
	std::stringstream ss;
	ss << name << " Mafia II Definitive Edition.exe+0x" << std::hex << std::uppercase << offset;

	// Log the message
	M2DEScriptHook::instance()->Log(ss.str().c_str());
}

//
lua_State* GetL()
{
	class C_ScriptGameMachine
	{
	public:
		char pad0[0x70];
		lua_State* luaState;

	public:
		inline lua_State* GetLuaState()
		{
			return luaState;
		}
	};

	class C_ScriptMachineManager
	{
	public:
		char pad0[0xD8];
		C_ScriptGameMachine mainScriptMachine;

	public:

		static C_ScriptMachineManager* GetInstance()
		{
			return *(C_ScriptMachineManager**)g_scriptMachineManager;
		}
		C_ScriptGameMachine* GetScriptMachine(int32_t idx) 
		{
			auto out = (uintptr_t)this + 8;
			out = **(uintptr_t**)(out);
			out = out + 8i64 * idx;
			return (C_ScriptGameMachine*)(out);
		}
		C_ScriptGameMachine* GetFirstScriptMachine()
		{
			return &mainScriptMachine;
		}
	};

	if (g_scriptMachineManager == nullptr)
	{
		return nullptr;
	}
	return C_ScriptMachineManager::GetInstance()->GetFirstScriptMachine()->GetLuaState();
}

int32_t LuaFunctions::PrintToLog(lua_State *L)
{
	const char *logFile;
	const char *message;

	if (plua_isstring(L, 1)) {
		logFile = plua_tostring(L, 1, nullptr);
	}

	if (plua_isstring(L, 2)) {
		message = plua_tostring(L, 2, nullptr);
	}

	M2DEScriptHook::instance()->LogToFile(logFile, message);
	return 0;
}

int32_t LuaFunctions::BindKey(lua_State *L)
{
	const char *key = "";
	const char *context = "";

	if (plua_isstring(L, 1)) {
		key = plua_tostring(L, 1, nullptr);
	}

	if (plua_isstring(L, 2)) {
		context = plua_tostring(L, 2, nullptr);
	}

	M2DEScriptHook::instance()->CreateKeyBind(key, context);
	return 0;
}

int32_t LuaFunctions::UnbindKey(lua_State *L)
{
	const char *key = "";

	if (plua_isstring(L, 1)) {
		key = plua_tostring(L, 1, nullptr);
	}

	M2DEScriptHook::instance()->DestroyKeyBind(key);
	return 0;
}

int32_t LuaFunctions::DelayBuffer(lua_State *L)
{
	M2DEScriptHook::instance()->Log(__FUNCTION__);
	const char *time = "";
	const char *context = "";

	if (plua_isstring(L, 1)) {
		time = plua_tostring(L, 1, nullptr);
	}

	if (plua_isstring(L, 2)) {
		context = plua_tostring(L, 2, nullptr);
	}

	// Pretty hacky implementation, we should consider using a Job queue instead and checking time?
	std::thread th = std::thread([L, time, context]() {
		M2DEScriptHook::instance()->Log(__FUNCTION__);
		auto mtime = std::stoi(time);
		std::this_thread::sleep_for(std::chrono::milliseconds(mtime));
		M2DEScriptHook::instance()->ExecuteLua(L, context);
	});
	th.detach();

	return 0;
}

int32_t LuaFunctions::FNV32a(lua_State *L)
{
	M2DEScriptHook::instance()->Log(__FUNCTION__);
	const char *toHash = "";

	if (plua_isstring(L, 1)) {
		toHash = plua_tostring(L, 1, nullptr);
	}

	static auto fnvHash = [](const char* str)
	{
		const size_t length = strlen(str) + 1;
		unsigned int hash = 2166136261u;
		for (size_t i = 0; i < length; ++i)
		{
			hash ^= *str++;
			hash *= 16777619u;
		}
		return hash;
	};

	return fnvHash(toHash);
}

//
LuaFunctions::LuaFunctions()
{
	M2DEScriptHook::instance()->Log(__FUNCTION__);
	// Yep, it's thread blocking, but that's what I want, no processing of other stuff until this shit's ready..
	while (!LoadPointers())
	{
		M2DEScriptHook::instance()->Log(__FUNCTION__ " Game is not ready, script engine not initialized, retry");
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		std::this_thread::yield();
	}
} 

bool LuaFunctions::IsMainScriptMachineReady()
{
	return true;
}

bool LuaFunctions::LoadPointers()
{
	M2DEScriptHook::instance()->Log(__FUNCTION__);

	if (auto scriptMachineManager = PatternScanner::Scan("48 89 05 ? ? ? ? 80 78", "g_scriptMachineManager"))
	{
		g_scriptMachineManager = scriptMachineManager.GetRef(3).To<C_ScriptMachineManager*>();
		logPointer("g_scriptMachineManager", g_scriptMachineManager);
	}

	if (auto pCall = PatternScanner::Scan("E8 ? ? ? ? 85 C0 0F 45 FF", "lua_pcall"))
	{
		plua_pcall2 = pCall.GetCall().To<lua_pcall_t>();
		logPointer("lua_pcall", plua_pcall2);
	}

	if (auto toString = PatternScanner::Scan("E8 ? ? ? ? 48 8B D8 4C 8B CE", "lua_tostring"))
	{
		plua_tostring = toString.GetCall().To<lua_tostring_t>();
		logPointer("lua_tostring", plua_tostring);
	}

	if (auto isString = PatternScanner::Scan("E8 ? ? ? ? 49 8B CE 85 C0 75", "lua_isstring"))
	{
		plua_isstring = isString.GetCall().To<lua_isstring_t>();
		logPointer("lua_isstring", plua_isstring);
	}

	if (auto loadBuffer = PatternScanner::Scan("E8 ? ? ? ? 48 8B 4F ? 85 C0 0F 85", "lua_loadBuffer"))
	{
		pluaL_loadbuffer = loadBuffer.GetCall().To<luaL_loadbuffer_t>();
		logPointer("lua_loadBuffer", pluaL_loadbuffer);
	}

	if (auto pushClosure = PatternScanner::Scan("E8 ? ? ? ? 4D 8B 06 41 8B D7", "lua_pushcclosure"))
	{
		plua_pushcclosure = pushClosure.GetCall().To<lua_pushcclosure_t>();
		logPointer("lua_pushcclosure", plua_pushcclosure);
	}

	if (auto setField = PatternScanner::Scan("E8 ? ? ? ? 8B 56 ? 85 D2", "lua_pushcclosure"))
	{
		plua_setfield = setField.GetCall().To<lua_setfield_t>();
		logPointer("lua_setfield", plua_setfield);
	}

	M2DEScriptHook::instance()->Log(__FUNCTION__ " Finished");

	return true;
}

bool LuaFunctions::Setup()
{
	M2DEScriptHook::instance()->Log(__FUNCTION__);

	static auto SetupGlobalLuaFunction = [](lua_State *L, const char *n, lua_CFunction f) {
		M2DEScriptHook::instance()->Log(__FUNCTION__);
		plua_pushcclosure(L, f, 0);
		//plua_setglobal(L, n);
		plua_setfield(L, -10002, (n));
	};

	auto L = GetL();
	SetupGlobalLuaFunction(L, "printToLog", LuaFunctions::PrintToLog);
	SetupGlobalLuaFunction(L, "bindKey", LuaFunctions::BindKey);
	SetupGlobalLuaFunction(L, "unbindKey", LuaFunctions::UnbindKey);
	SetupGlobalLuaFunction(L, "setTimeout", LuaFunctions::DelayBuffer);
	SetupGlobalLuaFunction(L, "fnv32", LuaFunctions::FNV32a);
	
	return true;
}
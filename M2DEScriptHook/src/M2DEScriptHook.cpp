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

#include <windows.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <algorithm>

#include <M2DEScriptHook.h>
#include <Common.h>
#include <LuaFunctions.h>
#include <ScriptSystem.h>
#include <hooking/hooking.h>

#ifdef _WIN64
#define DLLPATH "\\\\.\\GLOBALROOT\\SystemRoot\\System32\\dxgi.dll"
#else
#define DLLPATH "\\\\.\\GLOBALROOT\\SystemRoot\\SysWOW64\\dxgi.dll"
#endif // _WIN64

#pragma comment(linker, "/EXPORT:ApplyCompatResolutionQuirking=" DLLPATH ".ApplyCompatResolutionQuirking")
#pragma comment(linker, "/EXPORT:CompatString=" DLLPATH ".CompatString")
#pragma comment(linker, "/EXPORT:CompatValue=" DLLPATH ".CompatValue")
#pragma comment(linker, "/EXPORT:CreateDXGIFactory=" DLLPATH ".CreateDXGIFactory")
#pragma comment(linker, "/EXPORT:CreateDXGIFactory1=" DLLPATH ".CreateDXGIFactory1")
#pragma comment(linker, "/EXPORT:CreateDXGIFactory2=" DLLPATH ".CreateDXGIFactory2")
#pragma comment(linker, "/EXPORT:DXGID3D10CreateDevice=" DLLPATH ".DXGID3D10CreateDevice")
#pragma comment(linker, "/EXPORT:DXGID3D10CreateLayeredDevice=" DLLPATH ".DXGID3D10CreateLayeredDevice")
#pragma comment(linker, "/EXPORT:DXGID3D10GetLayeredDeviceSize=" DLLPATH ".DXGID3D10GetLayeredDeviceSize")
#pragma comment(linker, "/EXPORT:DXGID3D10RegisterLayers=" DLLPATH ".DXGID3D10RegisterLayers")
#pragma comment(linker, "/EXPORT:DXGIDeclareAdapterRemovalSupport=" DLLPATH ".DXGIDeclareAdapterRemovalSupport")
#pragma comment(linker, "/EXPORT:DXGIDisableVBlankVirtualization=" DLLPATH ".DXGIDisableVBlankVirtualization")
#pragma comment(linker, "/EXPORT:DXGIDumpJournal=" DLLPATH ".DXGIDumpJournal")
#pragma comment(linker, "/EXPORT:DXGIGetDebugInterface1=" DLLPATH ".DXGIGetDebugInterface1")
#pragma comment(linker, "/EXPORT:DXGIReportAdapterConfiguration=" DLLPATH ".DXGIReportAdapterConfiguration")
#pragma comment(linker, "/EXPORT:PIXBeginCapture=" DLLPATH ".PIXBeginCapture")
#pragma comment(linker, "/EXPORT:PIXEndCapture=" DLLPATH ".PIXEndCapture")
#pragma comment(linker, "/EXPORT:PIXGetCaptureState=" DLLPATH ".PIXGetCaptureState")
#pragma comment(linker, "/EXPORT:SetAppCompatStringPointer=" DLLPATH ".SetAppCompatStringPointer")
#pragma comment(linker, "/EXPORT:UpdateHMDEmulationStatus=" DLLPATH ".UpdateHMDEmulationStatus")

M2DEScriptHook::M2DEScriptHook()
{
	this->Log(__FUNCTION__);
	hooking::hooking_helpers::SetExecutableAddress(reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr)));
	hooking::ExecutableInfo::instance()->EnsureExecutableInfo();
	hooking::ExecutableInfo::instance()->GetExecutableInfo().SetSSEPatternSearching(false);

	this->keyBinds.clear();
}

#define BUFFER_COUNT 8
#define BUFFER_LENGTH 32768

void M2DEScriptHook::Log(const char* string, ...)
{
	static int32_t currentBuffer;
	static char* buffer = nullptr;

	if (!buffer)
	{
		buffer = new char[BUFFER_COUNT * BUFFER_LENGTH];
	}

	int32_t thisBuffer = currentBuffer;

	va_list ap;
	va_start(ap, string);
	int32_t length = vsnprintf(&buffer[thisBuffer * BUFFER_LENGTH], BUFFER_LENGTH, string, ap);
	va_end(ap);

	if (length >= BUFFER_LENGTH)
	{
		__debugbreak();
		exit(1);
	}

	buffer[(thisBuffer * BUFFER_LENGTH) + BUFFER_LENGTH - 1] = '\0';

	currentBuffer = (currentBuffer + 1) % BUFFER_COUNT;

	const char* msg = &buffer[thisBuffer * BUFFER_LENGTH];

	std::fstream file("ScriptHook.log", std::ios::out | std::ios::app);

	auto now = std::chrono::system_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
	auto timer = std::chrono::system_clock::to_time_t(now);
	auto local_time = *std::localtime(&timer);

	file << "[" << std::put_time(&local_time, "%m/%d/%Y %I:%M:%S") << ":" << std::setfill('0') << std::setw(3) << ms.count() << " " << std::put_time(&local_time, "%p") << "] " << msg << std::endl;
	file.close();
}

void M2DEScriptHook::Log(std::string message)
{
	return this->Log(message.c_str());
}

void M2DEScriptHook::LogToFile(const char *fileName, const char *string, ...)
{
	char tempBuffer[BUFFER_LENGTH];

	va_list args;
	va_start(args, string);
	vsnprintf(tempBuffer, BUFFER_LENGTH, string, args);
	va_end(args);

	std::ofstream(fileName, std::ios::app) << tempBuffer << std::endl;
}

void M2DEScriptHook::EndThreads()
{
	this->Log(__FUNCTION__);
	this->m_bEnded = true;
	PluginSystem::instance()->StopPlugins();
	delete LuaStateManager::instance();
}

void M2DEScriptHook::LoadScript(const std::string &file)
{
	this->Log(__FUNCTION__);
	auto threadState = LuaStateManager::instance()->GetState();
	this->LoadLuaFile(threadState, file);
}

void M2DEScriptHook::LoadLuaFile(lua_State *L, const std::string &name)
{
	this->Log(__FUNCTION__);
	std::string file = "function dofile (filename)local f = assert(loadfile(filename)) return f() end dofile(\"";
	file.append(name);
	file.append("\")");
	this->ExecuteLua(L, file);
}

// Export
LUA_API bool ExecuteLua(lua_State *L, const std::string &lua) 
{
	M2DEScriptHook::instance()->Log(__FUNCTION__);
	return M2DEScriptHook::instance()->ExecuteLua(L, lua);
}

bool M2DEScriptHook::ExecuteLua(lua_State *L, const std::string &lua)
{
	this->Log(std::string("Trying to execute: " + lua).c_str());

	if (!L) {
		this->Log("BadState");
		return false;
	}

	luaL_loadbuffer_(L, const_cast<char*>(lua.c_str()), lua.length(), "test");

	int32_t result = lua_pcall_(L, 0, LUA_MULTRET, 0);

	if (result != 0)
	{
		if (LUA_ERRSYNTAX == result)
		{
			this->Log("Error loading Lua code into buffer with (Syntax Error)");
			return false;
		}
		else if (LUA_ERRMEM == result)
		{
			this->Log("Error loading Lua code into buffer with (Memory Allocation Error)");
			return false;
		}
		else if (LUA_ERRRUN == result)
		{
			std::stringstream ss;
			ss << "Runtime Lua error. Message: ";
			const char* error = lua_tostring_(L, -1);
			ss << error;
			this->Log(ss.str());
			return false;
		}
		else
		{
			std::stringstream ss;
			ss << "Error loading Lua code into buffer. Error ";
			ss << result;
			this->Log(ss.str());
			const char *error = lua_tostring_(L, -1);
			this->Log(error);
			return false;
		}
	}
	return true;
}


uint32_t WINAPI M2DEScriptHook::mainThread(LPVOID) {
	static M2DEScriptHook *instance = M2DEScriptHook::instance();

	instance->Log(__FUNCTION__);

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	while (!instance->HasEnded()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		std::this_thread::yield(); // Process other threads

		M2DEScriptHook::instance()->ProcessKeyBinds();

		if (GetAsyncKeyState(VK_F1) & 1) {
			ScriptSystem::instance()->ReloadScripts();
		}

	}

	return 0;
}

void M2DEScriptHook::StartThreads()
{
	Log(__FUNCTION__);
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)M2DEScriptHook::mainThread, 0, 0, 0);

	LuaStateManager::instance()->StartThread();
}

bool M2DEScriptHook::HasEnded()
{
	return this->m_bEnded;
}

void M2DEScriptHook::ProcessKeyBinds()
{
	//this->Log(__FUNCTION__);
	std::unique_lock<std::recursive_mutex> lkScr(_keyBindMutex);

	auto L = GetL();
	if (!L)
		return;

	//this->Log("%d", keyBinds.size());
	if (!keyBinds.size())
		return;

	/*auto it = keyBinds.begin();
	for (auto bind : keyBinds) //; it != keyBinds.end();)
	{
		if (GetAsyncKeyState(bind->key) & 1)
		{
			M3ScriptHook::instance()->ExecuteLua(L, bind->bind);
		}
		//++it;
	}*/
	for (auto it = keyBinds.begin(); it != keyBinds.end(); ++it) {
		if (GetAsyncKeyState(it->first) & 1) {
			M2DEScriptHook::instance()->ExecuteLua(L, it->second);
		}
	}

}

void M2DEScriptHook::CreateKeyBind(const char* key, const char* context)
{
	Log("Binding key %s to function %s", key, context);

	uint8_t keyID = GetKeyID(key);
	if (keyID == 0xFF)
	{
		Log("Could not create keybind, key %s is unknown", key);
		return;
	}

	std::unique_lock<std::recursive_mutex> lkScr(_keyBindMutex);
	keyBinds[keyID] = context;
}

void M2DEScriptHook::DestroyKeyBind(const char* key)
{
	Log("Unbinding key %s", key);

	uint8_t keyID = GetKeyID(key);
	if (keyID != 0xFF)
	{
		std::unique_lock<std::recursive_mutex> lkScr(_keyBindMutex);
		keyBinds.erase(keyID);
	}
}

BOOL APIENTRY DllMain(HMODULE, DWORD code, LPVOID) {
	switch (code) {
	case DLL_PROCESS_ATTACH:
		PluginSystem::instance()->LoadPlugins();
		M2DEScriptHook::instance()->StartThreads();
		break;
	}
	return TRUE;
}
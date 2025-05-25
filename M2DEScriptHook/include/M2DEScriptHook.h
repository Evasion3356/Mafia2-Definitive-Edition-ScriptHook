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
#pragma once

#include <Export.h>
#include <PluginSystem.h>
#include <LuaStateManager.h>
#include <singleton.h>
#include <lua.h>
#include <map>
#include <mutex>
#include <vector>
#include <unordered_map>

class M2DEKeyBind
{
public: 
	M2DEKeyBind() = default;
	M2DEKeyBind(uint8_t a1, std::string a2) :
		key(a1),
		bind(a2) { }
	virtual ~M2DEKeyBind() = default;;

	uint8_t key;
	std::string bind;
};

class M2DEScriptHook : public singleton<M2DEScriptHook>
{
private:
	bool m_bEnded = false;
	std::unordered_map<uint8_t, std::string> keyBinds;
	std::recursive_mutex _keyBindMutex;

	uint8_t GetKeyID(const std::string& key)
	{
		static const std::unordered_map<std::string, uint8_t> BindableKeys = {
			{"VK_LBUTTON", VK_LBUTTON}, {"VK_RBUTTON", VK_RBUTTON}, {"VK_CANCEL", VK_CANCEL},
			{"VK_MBUTTON", VK_MBUTTON}, {"VK_XBUTTON1", VK_XBUTTON1}, {"VK_XBUTTON2", VK_XBUTTON2},
			{"VK_BACK", VK_BACK}, {"VK_TAB", VK_TAB}, {"VK_CLEAR", VK_CLEAR}, {"VK_RETURN", VK_RETURN},
			{"VK_SHIFT", VK_SHIFT}, {"VK_CONTROL", VK_CONTROL}, {"VK_MENU", VK_MENU}, {"VK_PAUSE", VK_PAUSE},
			{"VK_CAPITAL", VK_CAPITAL}, {"VK_KANA", VK_KANA}, {"VK_HANGUL", VK_HANGUL}, {"VK_IME_ON", VK_IME_ON},
			{"VK_JUNJA", VK_JUNJA}, {"VK_FINAL", VK_FINAL}, {"VK_HANJA", VK_HANJA}, {"VK_KANJI", VK_KANJI},
			{"VK_IME_OFF", VK_IME_OFF}, {"VK_ESCAPE", VK_ESCAPE}, {"VK_CONVERT", VK_CONVERT}, {"VK_NONCONVERT", VK_NONCONVERT},
			{"VK_ACCEPT", VK_ACCEPT}, {"VK_MODECHANGE", VK_MODECHANGE}, {"VK_SPACE", VK_SPACE}, {"VK_PRIOR", VK_PRIOR},
			{"VK_NEXT", VK_NEXT}, {"VK_END", VK_END}, {"VK_HOME", VK_HOME}, {"VK_LEFT", VK_LEFT}, {"VK_UP", VK_UP},
			{"VK_RIGHT", VK_RIGHT}, {"VK_DOWN", VK_DOWN}, {"VK_SELECT", VK_SELECT}, {"VK_PRINT", VK_PRINT},
			{"VK_EXECUTE", VK_EXECUTE}, {"VK_SNAPSHOT", VK_SNAPSHOT}, {"VK_INSERT", VK_INSERT}, {"VK_DELETE", VK_DELETE},
			{"VK_HELP", VK_HELP}, {"0", 0x30}, {"1", 0x31}, {"2", 0x32}, {"3", 0x33}, {"4", 0x34},
			{"5", 0x35}, {"6", 0x36}, {"7", 0x37}, {"8", 0x38}, {"9", 0x39}, {"a", 0x41},
			{"b", 0x42}, {"c", 0x43}, {"d", 0x44}, {"e", 0x45}, {"f", 0x46}, {"g", 0x47},
			{"h", 0x48}, {"i", 0x49}, {"j", 0x4A}, {"k", 0x4B}, {"l", 0x4C}, {"m", 0x4D},
			{"n", 0x4E}, {"o", 0x4F}, {"p", 0x50}, {"q", 0x51}, {"r", 0x52}, {"s", 0x53},
			{"t", 0x54}, {"u", 0x55}, {"v", 0x56}, {"w", 0x57}, {"x", 0x58}, {"y", 0x59},
			{"VK_Z", 0x5A}, {"VK_LWIN", VK_LWIN}, {"VK_RWIN", VK_RWIN}, {"VK_APPS", VK_APPS}, {"VK_SLEEP", VK_SLEEP},
			{"VK_NUMPAD0", VK_NUMPAD0}, {"VK_NUMPAD1", VK_NUMPAD1}, {"VK_NUMPAD2", VK_NUMPAD2}, {"VK_NUMPAD3", VK_NUMPAD3},
			{"VK_NUMPAD4", VK_NUMPAD4}, {"VK_NUMPAD5", VK_NUMPAD5}, {"VK_NUMPAD6", VK_NUMPAD6}, {"VK_NUMPAD7", VK_NUMPAD7},
			{"VK_NUMPAD8", VK_NUMPAD8}, {"VK_NUMPAD9", VK_NUMPAD9}, {"VK_MULTIPLY", VK_MULTIPLY}, {"VK_ADD", VK_ADD},
			{"VK_SEPARATOR", VK_SEPARATOR}, {"VK_SUBTRACT", VK_SUBTRACT}, {"VK_DECIMAL", VK_DECIMAL}, {"VK_DIVIDE", VK_DIVIDE},
			{"VK_F1", VK_F1}, {"VK_F2", VK_F2}, {"VK_F3", VK_F3}, {"VK_F4", VK_F4}, {"VK_F5", VK_F5}, {"VK_F6", VK_F6},
			{"VK_F7", VK_F7}, {"VK_F8", VK_F8}, {"VK_F9", VK_F9}, {"VK_F10", VK_F10}, {"VK_F11", VK_F11}, {"VK_F12", VK_F12},
			{"VK_NUMLOCK", VK_NUMLOCK}, {"VK_SCROLL", VK_SCROLL}, {"VK_LSHIFT", VK_LSHIFT}, {"VK_RSHIFT", VK_RSHIFT},
			{"VK_LCONTROL", VK_LCONTROL}, {"VK_RCONTROL", VK_RCONTROL}, {"VK_LMENU", VK_LMENU}, {"VK_RMENU", VK_RMENU},
			{"VK_BROWSER_BACK", VK_BROWSER_BACK}, {"VK_BROWSER_FORWARD", VK_BROWSER_FORWARD}, {"VK_BROWSER_REFRESH", VK_BROWSER_REFRESH},
			{"VK_BROWSER_STOP", VK_BROWSER_STOP}, {"VK_BROWSER_SEARCH", VK_BROWSER_SEARCH}, {"VK_BROWSER_FAVORITES", VK_BROWSER_FAVORITES},
			{"VK_BROWSER_HOME", VK_BROWSER_HOME}, {"VK_VOLUME_MUTE", VK_VOLUME_MUTE}, {"VK_VOLUME_DOWN", VK_VOLUME_DOWN},
			{"VK_VOLUME_UP", VK_VOLUME_UP}, {"VK_MEDIA_NEXT_TRACK", VK_MEDIA_NEXT_TRACK}, {"VK_MEDIA_PREV_TRACK", VK_MEDIA_PREV_TRACK},
			{"VK_MEDIA_STOP", VK_MEDIA_STOP}, {"VK_MEDIA_PLAY_PAUSE", VK_MEDIA_PLAY_PAUSE}, {"VK_LAUNCH_MAIL", VK_LAUNCH_MAIL},
			{"VK_LAUNCH_MEDIA_SELECT", VK_LAUNCH_MEDIA_SELECT}, {"VK_LAUNCH_APP1", VK_LAUNCH_APP1}, {"VK_LAUNCH_APP2", VK_LAUNCH_APP2},
			{"VK_OEM_1", VK_OEM_1}, {"VK_OEM_PLUS", VK_OEM_PLUS}, {"VK_OEM_COMMA", VK_OEM_COMMA}, {"VK_OEM_MINUS", VK_OEM_MINUS},
			{"VK_OEM_PERIOD", VK_OEM_PERIOD}, {"VK_OEM_2", VK_OEM_2}, {"VK_OEM_3", VK_OEM_3}, {"VK_OEM_4", VK_OEM_4},
			{"VK_OEM_5", VK_OEM_5}, {"VK_OEM_6", VK_OEM_6}, {"VK_OEM_7", VK_OEM_7}, {"VK_OEM_8", VK_OEM_8}, {"VK_OEM_102", VK_OEM_102},
			{"VK_PROCESSKEY", VK_PROCESSKEY}, {"VK_PACKET", VK_PACKET}, {"VK_ATTN", VK_ATTN}, {"VK_CRSEL", VK_CRSEL},
			{"VK_EXSEL", VK_EXSEL}, {"VK_EREOF", VK_EREOF}, {"VK_PLAY", VK_PLAY}, {"VK_ZOOM", VK_ZOOM}, {"VK_NONAME", VK_NONAME},
			{"VK_PA1", VK_PA1}, {"VK_OEM_CLEAR", VK_OEM_CLEAR}
		};

		auto it = BindableKeys.find(key);
		return it != BindableKeys.end() ? it->second : 0xFF;  // Invalid key
	}


public:

	M2DEScriptHook();
	virtual ~M2DEScriptHook() = default;

	void Log(std::string message);
	void Log(const char *string, ...);
	void LogToFile(const char *fileName, const char *string, ...);
	void EndThreads();
	void LoadScript(const std::string &file);
	void LoadLuaFile(lua_State *L, const std::string &name);
	bool ExecuteLua(lua_State *L, const std::string &lua);
	static uint32_t WINAPI mainThread(LPVOID);
	void StartThreads();
	bool HasEnded();
	void ProcessKeyBinds();
	void CreateKeyBind(const char* key, const char* context);
	void DestroyKeyBind(const char* key);
};

lua_State *GetL();
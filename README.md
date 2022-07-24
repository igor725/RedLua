# Lua scripting mod for Red Dead Redemption 2

RedLua is a ScriptHookRDR2 library that simplifies the game modding process.

## Installation

Download the latest [release](https://github.com/igor725/RedLua/releases) and extract the archive contents into your game folder. **Note that the RedLua won't work without the [ScriptHookRDR2](https://www.dev-c.com/rdr2/scripthookrdr2/) library!**

## Usage
Press **F7** (hotkey can be modified in the `Settings.json` file, keycodes can be found [here](https://cherrytree.at/misc/vk.htm)) to open the RedLua menu. Here you can load/reload/stop/unload scripts, reload the NativeDB and change library settings.

RedLua uses the [**Easylogging++**](https://github.com/amrayn/easyloggingpp#configuration) library, so if you want to configure the logger, just navigate to `<Your game directory>\RedLua\` and create a file called `Log.conf` with following contents:
```conf
* GLOBAL:
   FILENAME             = "RedLua\Run.log"
   ENABLED              = true
   TO_FILE              = false
   TO_STANDARD_OUTPUT   = true
   SUBSECOND_PRECISION  = 6
   PERFORMANCE_TRACKING = true
   MAX_LOG_FILE_SIZE    = 2097152
   LOG_FLUSH_THRESHOLD  = 0
* INFO:
   FORMAT               = "%datetime INFO  %msg"
* DEBUG:
   FORMAT               = "%datetime{%d/%M} DEBUG %func %msg"
* WARNING:
   FORMAT               = "%datetime WARN  %msg"
* ERROR:
   FORMAT               = "%datetime ERROR %msg"
* FATAL:
   FORMAT               = "%datetime FATAL %msg"

```

## Scripting

RedLua searches for scripts in the `<Your game directory>\RedLua\Scripts\`. Each script located in this folder will be loaded automatically (If the `Autorun feature` is enabled). Every script should return a `table` (can be empty) with functions `OnLoad`, `OnTick`, `OnStop`. Almost every function of the RDR2's RAGE is described [here](https://alloc8or.re/rdr3/nativedb/) and [here](https://www.rdr2mods.com/nativedb/index/builtin/).

Example:
```lua
local t = {}

function t.OnLoad()
	print('Yay!')
end

function t.OnTick()
	print('Tick!')
end

function t.OnStop()
	print('Goodbye!')
end

return t
```

### There are two ways to call native functions:
1. `native.call('PLAYER', 'PLAYER_ID')`
2. `PLAYER:GET_PLAYER_PED(PLAYER:PLAYER_ID())`

Note that the native functions can return RedLua's `userdata` called `NativeObject`. Each `NativeObject` has its own type (like `const char *`, `Ped`, `Vehicle`, `Entity *`, `Vector`, etc), as Lua-values, the native functions return (take) only the following types: `boolean`, `int`, `float`, `Hash`, `Any` and `string` (take only). Also, `NativeObjects` that are pointers, can be indexed.

Native functions that expect a parameter of type `Any *` as an argument can take a `cdata` value. Here is the example:
```lua
-- This script will display an in-game toast notification for 2 seconds when F8 key is released
local t = {}
local ffi = require'ffi'

function t.OnTick()
	if misc.iskeyjustup(VK_F8, true) then
		local duration = ffi.new([[
			struct {
				int duration;
				int some_weird_padding[14];
			}
		]], {
			2000 -- 2000 milliseconds
		})

		local data = ffi.new([[
			struct {
				uint64_t weird_padding_again;
				const char *title;
				const char *text;
				uint64_t god_another_unknown_field;
				uint64_t iconDict;
				uint64_t icon;
				uint64_t iconColor;
			}
		]], {
			0,
			MISC:VAR_STRING(10, 'LITERAL_STRING', 'Hello, RedLua!'):topointer(),
			MISC:VAR_STRING(10, 'LITERAL_STRING', jit.version):topointer(),
			0,
			MISC:GET_HASH_KEY('HUD_TOASTS'), MISC:GET_HASH_KEY('toast_player_deadeye'),
			MISC:GET_HASH_KEY('COLOR_RED')
		})

		UIFEED:_UI_FEED_POST_SAMPLE_TOAST(duration, data, true, true)
	end
	-- If you press F9 it will knock out all peds around you
	if mish.iskeyjustup(VK_F9, true) then
		local cnt = native.allpeds(mod.ent_arr) - 1
		for i = 0, cnt do
			if mod.ent_arr[i] ~= mod.me_ent then
				TASK:TASK_KNOCKED_OUT(mod.ent_arr[i], 10, false)
			end
		end
	end
end

return t
```

Here is a list of all the Lua functions provided by the RedLua library:

```lua
--[[
	Library: native
]]

-- Cast NativeObject to a pointer (useful for ffi)
NativeObject:topointer()

-- Invoke the native function
native.call('PLAYER', 'PLAYER_ID') -- Faster
PLAYER:PLAYER_ID() -- Simpler, but uses a lot of metacalls

-- Get the information about the function
native.info('BUILTIN', 'WAIT') -- Returns: {hash = '0x4EDE34FBADD967A6', build = 1207, returns = 'void', params = {[1] = {type = 'int', name = 'ms'}}} or nothing if specified function does not exists

-- Create new typed array
native.new('Vehicle', 5) -- Returns: NativeObject of 5 Vehicles
native.new('Vector3', 4) -- Returns: NativeVector, can be used in functions like ENTITY:GET_ENTITY_MATRIX(...)

-- Get all world objects
native.allobjects(objects_typed_array) -- Returns: objects count
native.allpeds(peds_typed_array)
native.allpickups(pickups_typed_array)
native.allvehicles(vehicles_typed_array)

--[[
	Library: misc
]]

-- Is key down for the last 5 seconds
misc.iskeydown(VK_*)

-- Is key down for the last 30 seconds
misc.iskeydownlong(VK_*)

-- Is key just up
misc.iskeyjustup(VK_*, exclusive = false)

-- Reset key state
misc.resetkey(VK_*)

-- Get game version
misc.gamever() -- Returns: number, e.g. 1436.31

-- Get RedLua version
misc.libver() -- Returns: integer, e.g. 010, 020, etc.
```

## Contribution

This is my first project that uses C++, so I'm not really good at it. If you see some cursed code and you know how to improve it, PRs are welcome.

## Thanks

Thanks to [Mike Pall](https://github.com/LuaJIT/LuaJIT), [Alexander Blade](https://www.dev-c.com/rdr2/scripthookrdr2/), [alloc8or](https://github.com/alloc8or/rdr3-nativedb-data), [Niels Lohmann](https://github.com/nlohmann/json) and [abumusamq](https://github.com/amrayn/easyloggingpp) for all their awesome work.

## License

RedLua is released under the MIT License. This license excludes files in the `src\thirdparty` folder that may be distributed under another license. Please read the `LICENSE` file for more information.

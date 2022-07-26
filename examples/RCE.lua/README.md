# Remote Code Execution example

As soon as RCE loads, it starts the web server on [0.0.0.0:1337](https://127.0.0.1:1337/). When you open this page, you will see the code editor where you can execute Lua scripts in realtime.

## Editor hotkeys:
1. F5 - Execute code
2. Ctrl+R - Clear output window
3. Ctrl+S - Save session to localStorage
4. Ctrl+num`[0-9]` - Switch editor tab

## Credits

This mod uses thirdparty libraries such as [luasocket](https://luarocks.org/modules/lunarmodules/luasocket) and [mimetypes](https://luarocks.org/modules/luarocks/mimetypes)

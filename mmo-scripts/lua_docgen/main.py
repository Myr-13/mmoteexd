from lua_analyzer import LuaAnalyzer


def main():
	lua = LuaAnalyzer("test.lua")
	lua.run()


if __name__ == "__main__":
	main()

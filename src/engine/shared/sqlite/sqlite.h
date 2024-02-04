#ifndef ENGINE_SHARED_SQLITE_SQLITE_H
#define ENGINE_SHARED_SQLITE_SQLITE_H

#include <thread>
#include <queue>
#include <string>

class CSQLite
{
	struct SRequest
	{
		struct lua_State *m_pState;
		std::string m_CallbackName;
	};

	struct sqlite3 *m_pDB;
	std::queue<SRequest> m_Requests;
	void *m_pThread;

public:
	CSQLite();
	~CSQLite();

	bool m_IsRunning;

	struct SResult
	{
		struct sqlite3_stmt *m_pStmt;
		struct sqlite3 *m_pDB;
		std::string m_Error;

		bool CheckError(int Result);

		bool Prepare(const char *pQuery);
		bool Step();
		int Execute();

		int GetInt(int Idx);
		float GetFloat(int Idx);
	};

	bool Connect(const char *pFile);
	void Disconnect();

	void Run();
	void Loop();

	// Lua
	void Execute(lua_State *L);
};

#endif // ENGINE_SHARED_SQLITE_SQLITE_H

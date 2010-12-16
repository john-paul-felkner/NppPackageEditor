#pragma once
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <stdio.h>

class SQL
{
public:
	static enum ReturnType { SCALAR, ROW, ROWS, TABLE };
	wchar_t connStr[255];
	wchar_t* cmdStr;

	int MAX_STMT_LEN;
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt;
	HWND hwnd;

	SQL(wchar_t* cmd, wchar_t* conn) 
	{
		MAX_STMT_LEN = 3200;
		cmdStr = cmd;
		if (conn != NULL) 
		{
			wcscpy(connStr, conn);
		}
		hwnd = GetDesktopWindow();   // desktop's window handle
	}
	~SQL(void);

	//template <class T>
	//T getData()
	SQLRETURN open() 
	{
		return openConn();
	}
	void close()
	{
		closeConn();
	}
	void getData()
	{
		openConn();
		// Allocate statement handle
		SQLRETURN retcode;
		retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt); 

		// Process data
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			// Bind parameters
			//retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &packageId, 0, NULL);
			//retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &stepId, 0, NULL);
			retcode = SQLExecDirect(hstmt, cmdStr, SQL_NTS);
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				// Bind columns
			//	SQLBindCol(hstmt, 1, SQL_C_CHAR, &code, sizeof(code), NULL );
				SQLFetch(hstmt);
			}
			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		}
		closeConn();
	}
private:

	SQLRETURN openConn() 
	{
		wchar_t OutConnStr[255];
		SQLSMALLINT OutConnStrLen;
		SQLRETURN retcode;

		// Allocate environment handle
		retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

		// Set the ODBC version environment attribute
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		  retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0); 

		  // Allocate connection handle
		  if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			 retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc); 

			 // Set login timeout to 5 seconds
			 if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				retcode = SQLDriverConnect( // SQL_NULL_HDBC
				   hdbc, 
				   hwnd, 
				   (SQLWCHAR*)connStr, 
				   _countof(connStr),
				   OutConnStr,
				   255, 
				   &OutConnStrLen,
				   SQL_DRIVER_COMPLETE_REQUIRED );
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					wcscpy(connStr, OutConnStr);
				}
			}
		  }
		}
		return retcode;
	}

	void closeConn()
	{
		SQLDisconnect(hdbc);
		SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
	}
	template <class T>
	T query() 
	{
		// Allocate statement handle
		SQLRETURN retcode;
		retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt); 

		// Process data
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			// Bind parameters
			//retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &packageId, 0, NULL);
			//retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &stepId, 0, NULL);
			retcode = SQLExecDirect(hstmt, cmdStr, SQL_NTS);
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				// Bind columns
			//	SQLBindCol(hstmt, 1, SQL_C_CHAR, &code, sizeof(code), NULL );
				SQLFetch(hstmt);
			}
			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		}
	}
};

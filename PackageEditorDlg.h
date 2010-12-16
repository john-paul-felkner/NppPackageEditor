#ifndef PACKAGEEDITOR_DLG_H
#define PACKAGEEDITOR_DLG_H
#include "DockingDlgInterface.h"
#include "resource1.h"
#include "SQL.h"
#include <sstream>

class PackageEditorDlg :
	public DockingDlgInterface
{
public:
	wchar_t dbConnStr[255];
	wchar_t saveDir[255];
	HWND getWin() const;
	__declspec( property( get=getWin ) ) HWND Window;
	void message(LPCWSTR msg) const
	{
		message(msg, NULL);
	}
	void message(LPCWSTR msg, LPCWSTR title) const
	{
		//if (msg == NULL) strcpy(msg, "");
		//if (title == NULL) strcpy(title, NPP_PLUGIN_NAME);
		::MessageBox(NULL, msg, (title != NULL ? title : _pluginName), MB_OK);
	}
	bool saveToDatabase(UINT packageId, UINT stepId, wchar_t* code, bool makeBackup);


	PackageEditorDlg() : DockingDlgInterface(IDD_PackageEditorDlg){};
    virtual void display(bool toShow = true) const {
        DockingDlgInterface::display(toShow);
		if (toShow) {
            ::SetFocus(::GetDlgItem(_hSelf, ID_PACKAGE_EDIT));
			bool isSuccessful = SetDlgItemText(Window, ID_SAVE_DIRECTORY, (LPCWSTR) saveDir);
			if (!isSuccessful) {
				message(TEXT("Failed displaying current Save Directory."));
			}
		}

	};

	void setParent(HWND parent2set){
		_hParent = parent2set;
	};

protected :
	virtual BOOL CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);

private :
	HWND getCurrScintilla();
	bool addPage(UINT packageId, UINT stepId, WCHAR* backupDateStr = NULL, bool addBackups = false);
	bool saveTmp(const wchar_t* text, wchar_t* filePath);
	void assignBackupList(UINT packageId, UINT stepId);
	std::wstring getScriptCode(UINT packageId, UINT stepId, WCHAR* backupDateStr = NULL) {
		const int MAX_CHAR = 64000;
		SQLWCHAR* buffer = new SQLWCHAR[MAX_CHAR];
		SQLINTEGER outLen;
		std::wstring data;

		SQLRETURN retcode;

		bool queryBackup = backupDateStr != NULL;
		SQLINTEGER bdLen = SQL_NTS;
		SQL* sql;
		if (queryBackup) {
			sql = new SQL(TEXT("SELECT [code] FROM PackageStepsBackup WHERE packageId = ? AND step = ? AND Date = ?"), dbConnStr);
		} else {
			sql =  new SQL(TEXT("SELECT [code] FROM PackageSteps WHERE packageId = ? AND step = ?"), dbConnStr);
		}
		sql->open();
		retcode = SQLAllocHandle(SQL_HANDLE_STMT, sql->hdbc, &(sql->hstmt)); 

		// Process data
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			// Bind parameters
			retcode = SQLBindParameter(sql->hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &packageId, 0, NULL);
			retcode = SQLBindParameter(sql->hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &stepId, 0, NULL);
			if (queryBackup) {
				retcode = SQLBindParameter(sql->hstmt, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_CHAR, 50, 0, backupDateStr, 0, (SQLINTEGER*)&bdLen);				
			}
			retcode = SQLExecDirect(sql->hstmt, sql->cmdStr, SQL_NTS);
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				// Bind columns
				retcode = SQLFetch(sql->hstmt);
				if (retcode == SQL_SUCCESS || SQL_SUCCESS_WITH_INFO) {
					//SQLBindCol(sql->hstmt, 1, SQL_C_CHAR, &code, sizeof(code), NULL );
					SQLINTEGER numChars = 0;
					SQLINTEGER numCharsWritten = 0;
					while ((retcode = ::SQLGetData(sql->hstmt, 1, SQL_C_WCHAR, buffer, MAX_CHAR, &outLen))
						!= SQL_NO_DATA){
							/*if (numCharsWritten == 0) {
								buf = new SQLWCHAR[outLen];
							}*/
						numChars = ((outLen > MAX_CHAR) || (outLen == SQL_NO_TOTAL)
							? MAX_CHAR : outLen);
						/*
						for (int i = 0; i < numChars; i++) {
							buf[numCharsWritten + i] = targetValPtr[i];
						}
						*/
						data.append(buffer);
						numCharsWritten += numChars;
						delete [] buffer;
						buffer = new SQLWCHAR[MAX_CHAR];
						//ss.write(targetValPtr, numChars);
						//ss.write((wchar_t*)targetValPtr, numChars);
					}
				}
			}
			SQLFreeHandle(SQL_HANDLE_STMT, sql->hstmt);
		}
		SQLFreeHandle(SQL_HANDLE_STMT, sql->hstmt);
		sql->close();
		wcscpy(dbConnStr, sql->connStr);
		//SQLINTEGER totalChars = wcslen(buf);
		delete [] buffer;
		buffer = NULL;
		return data;
	}
};

#endif //GOTILINE_DLG_H

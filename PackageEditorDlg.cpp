#include "PackageEditorDlg.h"
#include "PluginDefinition.h"
#include "menuCmdID.h"
#include <iostream>
#include <fstream>

extern NppData nppData;
bool PackageEditorDlg::addPage(UINT packageId, UINT stepId, WCHAR* backupDateStr, bool addBackups) {
	std::wstring codeStr = getScriptCode(packageId, stepId, backupDateStr);
	const wchar_t* code = codeStr.c_str();
	
	if (code != NULL)
	{
		// Open a new document
		//::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);

	 	// Get the current scintilla
		int which = -1;
		::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
		if (which == -1)
			return false;
		HWND curScintilla = (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;

		if (curScintilla == NULL) {
			return false;
		}
		// Scintilla control has no Unicode mode, so we use (char *) here
		size_t origsize = wcslen(code) + 1;
		size_t convertedChars = 0;
		char* conv = new char[origsize];
		wcstombs_s(&convertedChars, conv, origsize, code, _TRUNCATE);
		if (convertedChars != origsize) {
			std::wstring mStr;
			wchar_t oLen[10], cLen[10];
			_itow(origsize, oLen, 10);
			_itow(convertedChars, cLen, 10);
			mStr.append(TEXT("Warning: Script converted length ("))
				.append(oLen)
				.append(TEXT(") did not match original length ("))
				.append(cLen)
				.append(TEXT(")."));
			const wchar_t* msg = mStr.c_str();
			::MessageBox(NULL, msg, NPP_PLUGIN_NAME, MB_OK);
		}
		//::SendMessage(curScintilla, SCI_SETTEXT, 0, (LPARAM)conv);
		//::SendMessage(nppData._nppHandle, NPPM_SETCURRENTLANGTYPE, 0, (LPARAM)LangType::L_PYTHON);
		std::wstring locationStr = std::wstring(saveDir);
		wchar_t filePath[255];
		swprintf(filePath, 255,  TEXT("%s\\p%ds%d.py"), saveDir, packageId, stepId);
		bool saved = saveTmp(code, filePath);
		if (saved)
		{
			size_t origSizePath = wcslen(filePath) + 1;
			size_t convertedCharsPath = 0;
			char* convPath = new char[origSizePath];
			wcstombs_s(&convertedCharsPath, convPath, origSizePath, filePath, _TRUNCATE);
			bool opened = ::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)filePath);
			if (opened)
			{
				return true;
			}
			else 
			{
				wchar_t errMsg[255];
				swprintf(errMsg, TEXT("Unable to open file: %s"), filePath);
				message(errMsg);
				return false;
			}
		} 
		else
		{
			wchar_t errMsg[255];
			swprintf(errMsg, TEXT("Unable to save to path: %s"), filePath);
			message(errMsg);
			return false;
		}
		return false;
	}
	return false;
}
bool PackageEditorDlg::saveTmp(const wchar_t* text, wchar_t* filePath)
{
	std::wstring pathStr = std::wstring(filePath);
	std::wstring locationStr = pathStr.substr(0, pathStr.find_last_of('\\'));
	const wchar_t* location = locationStr.c_str();
	if (location == NULL || location == TEXT(""))
	{
		return false;
	}
	// if tmp path doesn't exist, we create it
	if (PathFileExists(location) == FALSE)
	{
		::CreateDirectory(location, NULL);
	}
	std::wofstream fs;
	fs.open(filePath, ::std::ios::out|::std::ios::in|::std::ios::binary|::std::ios::trunc);
	if (fs)
	{
		fs << text;
		fs.close();
		return true;
	}
	return false;
}
bool PackageEditorDlg::saveToDatabase(UINT packageId, UINT stepId, wchar_t* code, bool makeBackup) {
	const int MAX_CHAR = 64000;
	SQLWCHAR* buffer = new SQLWCHAR[MAX_CHAR];
	SQLINTEGER outLen;
	std::wstring data;
	int codeLen = wcslen(code);
	bool isSuccess = false;
	SQLRETURN retcode;

	SQLINTEGER bdLen = SQL_NTS;
	SQL* sql;
	if (makeBackup) {
		sql = new SQL(TEXT(
			"INSERT INTO PackageStepsBackup (packageId, step, code, TOCEntry, Date) \
			SELECT packageId, step, code, TOCEntry, GETDATE() \
			FROM PackageSteps \
			WHERE packageId = ? AND step = ?; \
			UPDATE PackageSteps \
			SET code = ? \
			WHERE packageId = ? AND step = ?"), dbConnStr);
	} else {
		sql =  new SQL(TEXT("UPDATE PackageSteps SET code = ? WHERE packageId = ? AND step = ?"), dbConnStr);
	}
	sql->open();
	retcode = SQLAllocHandle(SQL_HANDLE_STMT, sql->hdbc, &(sql->hstmt)); 

	// Process data
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		// Bind parameters
		if (makeBackup) {
			retcode = SQLBindParameter(sql->hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &packageId, 0, NULL);
			retcode = SQLBindParameter(sql->hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &stepId, 0, NULL);
			retcode = SQLBindParameter(sql->hstmt, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_CHAR, codeLen, 0, code, 0, (SQLINTEGER*)&bdLen);				
			retcode = SQLBindParameter(sql->hstmt, 4, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &packageId, 0, NULL);
			retcode = SQLBindParameter(sql->hstmt, 5, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &stepId, 0, NULL);
		} else {
			retcode = SQLBindParameter(sql->hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_CHAR, codeLen, 0, code, 0, (SQLINTEGER*)&bdLen);				
			retcode = SQLBindParameter(sql->hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &packageId, 0, NULL);
			retcode = SQLBindParameter(sql->hstmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &stepId, 0, NULL);
		}
		retcode = SQLExecDirect(sql->hstmt, sql->cmdStr, SQL_NTS);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			isSuccess = true;
		}
		SQLFreeHandle(SQL_HANDLE_STMT, sql->hstmt);
	}
	SQLFreeHandle(SQL_HANDLE_STMT, sql->hstmt);
	sql->close();
	return isSuccess;
}
void PackageEditorDlg::assignBackupList(UINT packageId, UINT stepId) {
	// Get backups

	// remove previous items
	int count = SendDlgItemMessage(_hSelf, ID_BACKUP_LIST, LB_GETCOUNT, 0, 0);
	for (int i = 0; i < count; i++) {
		SendDlgItemMessage(_hSelf, ID_BACKUP_LIST, LB_DELETESTRING, 0, 0);
	}
	SQLRETURN retcode;
	SQL* sql = new SQL(TEXT("SELECT Date FROM PackageStepsBackup WHERE packageId = ? AND step = ? ORDER BY Date DESC"), dbConnStr);
	sql->open();
	retcode = SQLAllocHandle(SQL_HANDLE_STMT, sql->hdbc, &(sql->hstmt)); 

	// Process data
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		// Bind parameters
		retcode = SQLBindParameter(sql->hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &packageId, 0, NULL);
		retcode = SQLBindParameter(sql->hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &stepId, 0, NULL);
		retcode = SQLExecDirect(sql->hstmt, sql->cmdStr, SQL_NTS);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			wchar_t date[50];
			// Bind columns
			int i = 0;
			SQLBindCol(sql->hstmt, 1, SQL_C_WCHAR, &date, sizeof(date), NULL );
			while ((retcode = SQLFetch(sql->hstmt)) == SQL_SUCCESS) {
				// add backup dates to listbox
				SendDlgItemMessage(_hSelf, ID_BACKUP_LIST, LB_ADDSTRING, 0, (LPARAM)date);
			}
		}
		SQLFreeHandle(SQL_HANDLE_STMT, sql->hstmt);
	}
	SQLFreeHandle(SQL_HANDLE_STMT, sql->hstmt);
	sql->close();
	wcscpy(dbConnStr, sql->connStr);
}
HWND PackageEditorDlg::getWin() const
{
	return this->_hSelf;
}
BOOL CALLBACK PackageEditorDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_COMMAND :
		{
			switch (LOWORD(wParam))
			{
				case IDOK :
				{
					BOOL isSuccessful;
					UINT packageId = ::GetDlgItemInt(Window, ID_PACKAGE_EDIT, &isSuccessful, FALSE);
					if (isSuccessful) {
						UINT stepId = ::GetDlgItemInt(_hSelf, ID_STEP_EDIT, &isSuccessful, FALSE);
						if (!isSuccessful || stepId < 1) stepId = 1;
						addPage(packageId, stepId, NULL, true);
						assignBackupList(packageId, stepId);
					}
					return true;
				}
				case ID_BACKUP_LIST:
				case ID_BACKUP_LOAD:
				{
					if (LOWORD(wParam) == ID_BACKUP_LOAD 
						|| (LOWORD(wParam) == ID_BACKUP_LIST && HIWORD(wParam) == LBN_DBLCLK))
					{
						HWND hwndList = GetDlgItem(_hSelf, ID_BACKUP_LIST);
						int lbItem = (int)SendMessage(hwndList, LB_GETCURSEL, 0, 0); 
						if (lbItem >= 0) {
							// Get item data
							wchar_t date[50];
							SendMessage(hwndList, LB_GETTEXT, lbItem, (LPARAM)date);
							BOOL isSuccessful;
							UINT packageId = ::GetDlgItemInt(_hSelf, ID_PACKAGE_EDIT, &isSuccessful, FALSE);
							if (isSuccessful) {
								UINT stepId = ::GetDlgItemInt(_hSelf, ID_STEP_EDIT, &isSuccessful, FALSE);
								if (!isSuccessful || stepId < 1) stepId = 1;
								addPage(packageId, stepId, date, false);
							}
						}
					}
					return true;
				}
				case ID_UPDATE_SAVE_DIRECTORY:
				{
					wchar_t path[255];
					GetDlgItemText(this->Window, ID_SAVE_DIRECTORY, (LPWSTR)path, 255);
					wcscpy(this->saveDir, path);
					this->message(L"Update successful!");
					return true;
				}
			}
			return FALSE;
		}

		default :
			return DockingDlgInterface::run_dlgProc(message, wParam, lParam);
	}
}

//this file is part of notepad++
//Copyright (C)2003 Don HO <donho@altern.org>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "PluginDefinition.h"

extern FuncItem funcItem[nbFunc];
extern NppData nppData;
extern PackageEditorDlg _pkgEditor;


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  reasonForCall, 
                       LPVOID lpReserved )
{
    switch (reasonForCall)
    {
      case DLL_PROCESS_ATTACH:
        pluginInit(hModule);
        break;

      case DLL_PROCESS_DETACH:
		commandMenuCleanUp();
        pluginCleanUp();
        break;

      case DLL_THREAD_ATTACH:
        break;

      case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}


extern "C" __declspec(dllexport) void setInfo(NppData notpadPlusData)
{
	nppData = notpadPlusData;
	commandMenuInit();
}

extern "C" __declspec(dllexport) const TCHAR * getName()
{
	return NPP_PLUGIN_NAME;
}

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbF)
{
	*nbF = nbFunc;
	return funcItem;
}

int i = 0;
extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode)
{
	if (notifyCode->nmhdr.code == NPPN_FILEBEFORESAVE) {
		i++;
		int which = -1;
		::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
		if (which == -1)
			return;
		HWND curScintilla = (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;

		if (curScintilla == NULL) {
			return;
		}
		wchar_t curPath[MAX_PATH];
		wchar_t fileName[MAX_PATH];
		SendMessage (nppData._nppHandle, NPPM_GETCURRENTDIRECTORY, MAX_PATH, (LPARAM)curPath);
		SendMessage(nppData._nppHandle, NPPM_GETFILENAME, MAX_PATH, (LPARAM)fileName);
		std::wstring pStr = std::wstring(curPath);
		std::wstring fnStr = std::wstring(fileName);
		
		int eInd = fnStr.rfind('.');
		if (pStr.compare(_pkgEditor.saveDir) == 0
			&& eInd > -1 && fnStr.compare(eInd, 3, L".py") == 0) {
			int pInd = fnStr.find('p');
			int sInd = fnStr.find('s');
			if (pInd >= 0 && sInd >= 2) {
				UINT packageId = 0, stepId = 0;
				std::wstringstream(fnStr.substr(pInd + 1, sInd - pInd - 1)) >> packageId;
				std::wstringstream(fnStr.substr(sInd + 1, eInd - sInd - 1)) >> stepId;
				if (packageId > 0 && stepId > 0) {
					int nLen = ::SendMessage(curScintilla, SCI_GETLENGTH, NULL, NULL);
					int oLen = nLen + 1;
					char* code = new char[oLen];
					::SendMessage(curScintilla, SCI_GETTEXT, oLen, (LPARAM)code);
					size_t convLen = 0;
					wchar_t* conv = new wchar_t[oLen];
					mbstowcs_s(&convLen, conv, oLen, code, oLen);
					if (convLen == oLen) {
						bool makeBackup = true;
						_pkgEditor.saveToDatabase(packageId, stepId, conv, makeBackup);
						wchar_t msg[255];
						swprintf(msg, 255, TEXT("Save complete for package %d step %d!%s"), packageId, stepId, (makeBackup ? TEXT(" Backup stored.") : TEXT("")));
						_pkgEditor.message(msg);
					} else {
						wchar_t errMsg[255];
						swprintf(errMsg, 255, TEXT("Error saving file: Inconsistent file length found on conversion - expected %d, received %d."),
							oLen, convLen);
						_pkgEditor.message(errMsg);
					}
				} else {
					_pkgEditor.message(TEXT("Error parsing filename: package or step id not a valid integer."));
				}
			} else {
				_pkgEditor.message(TEXT("Error parsing filename: package or step id not found."));
			}
		}
	}
}


// Here you can process the Npp Messages 
// I will make the messages accessible little by little, according to the need of plugin development.
// Please let me know if you need to access to some messages :
// http://sourceforge.net/forum/forum.php?forum_id=482781
//
extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam)
{/*
	if (Message == WM_MOVE)
	{
		::MessageBox(NULL, "move", "", MB_OK);
	}
*/
	return TRUE;
}

#ifdef UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode()
{
    return TRUE;
}
#endif //UNICODE

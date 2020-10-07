/* 
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     ReactOS system libraries
 * FILE:        lib/sdk/crt/process/_system.c
 * PURPOSE:     Excutes a shell command
 * PROGRAMER:   Ariadne
 * UPDATE HISTORY:
 *              04/03/99: Created
 */

#include <precomp.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <tchar.h>

/*
 * @implemented
 */
int _tsystem(const TCHAR *command)
{
  TCHAR *szCmdLine = NULL;
  TCHAR *szComSpec = NULL;

  PROCESS_INFORMATION ProcessInformation;
  STARTUPINFO StartupInfo;
  TCHAR *s;
  BOOL result;

  int nStatus;

  szComSpec = _tgetenv(_T("COMSPEC"));

// system should return 0 if command is null and the shell is found

  if (command == NULL) {
    if (szComSpec == NULL)
      return 0;
    else
      return 1;
  }

  if (szComSpec == NULL)
    return -1;

// should return 127 or 0 ( MS ) if the shell is not found
// _set_errno(ENOENT);

  if (szComSpec == NULL)
  {
    szComSpec = _T("cmd.exe");
  }

  /* split the path from shell command */
  s = max(_tcsrchr(szComSpec, '\\'), _tcsrchr(szComSpec, '/'));
  if (s == NULL)
    s = szComSpec;
  else
    s++;

  szCmdLine = malloc((_tcslen(s) + 4 + _tcslen(command) + 1)*sizeof(TCHAR));
  if (szCmdLine == NULL)
  {
     _set_errno(ENOMEM);
     return -1;
  }

  _tcscpy(szCmdLine, s);
  s = _tcsrchr(szCmdLine, '.');
  if (s)
    *s = 0;
  _tcscat(szCmdLine,_T(" /C "));
  _tcscat(szCmdLine, command);

//command file has invalid format ENOEXEC

  memset (&StartupInfo, 0, sizeof(StartupInfo));
  StartupInfo.cb = sizeof(StartupInfo);
  StartupInfo.lpReserved= NULL;
  StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
  StartupInfo.wShowWindow = SW_SHOWDEFAULT;
  StartupInfo.lpReserved2 = NULL;
  StartupInfo.cbReserved2 = 0;

// According to ansi standards the new process should ignore  SIGINT and SIGQUIT
// In order to disable ctr-c the process is created with CREATE_NEW_PROCESS_GROUP,
// thus SetConsoleCtrlHandler(NULL,TRUE) is made on behalf of the new process.


//SIGCHILD should be blocked aswell

  result = CreateProcess(szComSpec,
	                  szCmdLine,
			  NULL,
			  NULL,
			  TRUE,
			  CREATE_NEW_PROCESS_GROUP,
			  NULL,
			  NULL,
			  &StartupInfo,
			  &ProcessInformation);
  free(szCmdLine);

  if (result == FALSE)
  {
	_dosmaperr(GetLastError());
     return -1;
  }

  CloseHandle(ProcessInformation.hThread);

// system should wait untill the calling process is finished
  _cwait(&nStatus,(intptr_t)ProcessInformation.hProcess,0);
  CloseHandle(ProcessInformation.hProcess);

  return nStatus;
}


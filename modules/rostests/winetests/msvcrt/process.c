/*
 * Tests for process functions
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "wine/test.h"
#include <stdio.h>

static char ***(__cdecl *p__p__environ)(void);

static void init(void)
{
    HMODULE hmod = GetModuleHandleA("msvcrt.dll");

    p__p__environ = (void *)GetProcAddress(hmod, "__p__environ");
}


#define system_test(x, cmd) \
  ret = system(cmd); \
  ok(ret == x, "Expected system to return "#x", got %d\n", ret);
#define wsystem_test(x, cmd) \
  ret = _wsystem(cmd); err = errno; \
  ok(ret == x, "Expected _wsystem to return "#x", got %d\n", ret);
#define errno_chk(x) \
  ok(err == x, "Expected errno = "#x", got %d\n", err);

static void test_system(void)
{
    int ret, err;
    HANDLE hFile;
    char dir[260];
    char file[260];
    
    // normal comspec
    system_test(1, NULL);
    wsystem_test(1, NULL);
    system_test(1234568, "exit 1234568");
    wsystem_test(1234568, L"exit 1234568");
    
    // null comspec
    putenv("COMSPEC=");

    system_test(0, NULL);
    wsystem_test(0, NULL);
    system_test(1234568, "exit 1234568");
    wsystem_test(1234568, L"exit 1234568");
    
    // invalid comspec
    putenv("COMSPEC=THIS_NAME_DOES_NOT_EXIST");

    system_test(0, NULL);
    wsystem_test(0, NULL);
    system_test(1234568, "exit 1234568");
    wsystem_test(1234568, L"exit 1234568");

    // invalid exe in current directory
    putenv("COMSPEC=test_system.tmp");
    hFile = CreateFileA("test_system.tmp", GENERIC_READ, FILE_SHARE_READ,
        NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    CloseHandle(hFile);

    system_test(1, NULL);
    wsystem_test(1, NULL);
    system_test(-1, "exit 1234568");
    wsystem_test(-1, L"exit 1234568");
    errno_chk(8)
    
    // locked invalid exe in current directory
    hFile = CreateFileA("test_system.tmp", GENERIC_READ, 0,
        NULL, OPEN_EXISTING, FILE_FLAG_DELETE_ON_CLOSE, NULL);

    system_test(1, NULL);
    wsystem_test(1, NULL);
    system_test(1234568, "exit 1234568");
    wsystem_test(1234568, L"exit 1234568");

    CloseHandle(hFile);

    // invalid exe with space in name
    putenv("COMSPEC=test system.tmp");
    hFile = CreateFileA("test system.tmp", GENERIC_READ, FILE_SHARE_READ,
        NULL, CREATE_ALWAYS, FILE_FLAG_DELETE_ON_CLOSE, NULL);

    system_test(1, NULL);
    wsystem_test(1, NULL);
    system_test(-1, "exit 1234568");
    wsystem_test(-1, L"exit 1234568");
    errno_chk(8)

    CloseHandle(hFile);

    // null comspec, and null path
    putenv("COMSPEC=");
    putenv("PATH=");
    
    system_test(0, NULL);
    wsystem_test(0, NULL);
    system_test(-1, "exit 1234568");
    wsystem_test(-1, L"exit 1234568");
    errno_chk(2)
    
    // create temporary directory
    strcpy(dir, "PATH=");    
    GetCurrentDirectory(200, dir+5); 
    strcat(dir+5, "\\test_system");
    CreateDirectoryA(dir+5, NULL);
    putenv(dir);
    
    // create temporary file
    strcpy(file, "COMSPEC=");
    GetCurrentDirectory(200, file+8); 
    strcat(file+8, "\\test_system\\test.tmp");
    hFile = CreateFileA(file+8, GENERIC_READ, FILE_SHARE_READ,
        NULL, CREATE_ALWAYS, FILE_FLAG_DELETE_ON_CLOSE, NULL);
    putenv(file);
    
    // invalid exe, full path
    system_test(1, NULL);
    wsystem_test(1, NULL);
    system_test(-1, "exit 1234568");
    wsystem_test(-1, L"exit 1234568");
    errno_chk(8);
    
    // invalid exe, name only
    putenv("COMSPEC=test.tmp");
    
    system_test(0, NULL);
    wsystem_test(0, NULL);
    system_test(-1, "exit 1234568");
    wsystem_test(-1, L"exit 1234568");
    errno_chk(2);
    
    CloseHandle(hFile);
    
    // null comspec, invalid cmd.exe on path
    putenv("COMSPEC=");
    GetCurrentDirectory(200, file+8);    
    strcat(file+8, "\\test_system\\cmd.exe");
    hFile = CreateFileA(file+8, GENERIC_READ, FILE_SHARE_READ,
        NULL, CREATE_ALWAYS, FILE_FLAG_DELETE_ON_CLOSE, NULL);

    system_test(0, NULL);
    wsystem_test(0, NULL);
    system_test(-1, "exit 1234568");
    wsystem_test(-1, L"exit 1234568");
    errno_chk(8);

    CloseHandle(hFile);
    RemoveDirectoryA(dir+5);
}


START_TEST(process)
{
    init();

    test_system();
}

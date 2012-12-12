/*
	SD Card Recovery
	By: Lansdon Page, Steve Ahl
	Date: 12/12/2012
	Description: Recovers deleted files from an SD Card.
	

*/

#include <iostream>
#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include <winioctl.h>  // From the Win32 SDK \Mstools\Include
//#include "ntddcdrm.h"  // From the Windows NT DDK \Ddk\Src\Storage\Inc

#define BUFFER_SIZE 512



// Displays volumes on the computer
void DisplayVolumePaths(
        __in PWCHAR VolumeName
        )
{
    DWORD  CharCount = MAX_PATH + 1;
    PWCHAR Names     = NULL;
    PWCHAR NameIdx   = NULL;
    BOOL   Success   = FALSE;

    for (;;) 
    {
        //
        //  Allocate a buffer to hold the paths.
        Names = (PWCHAR) new BYTE [CharCount * sizeof(WCHAR)];

        if ( !Names ) 
        {
            //
            //  If memory can't be allocated, return.
            return;
        }

        //
        //  Obtain all of the paths
        //  for this volume.
        Success = GetVolumePathNamesForVolumeNameW(
            VolumeName, Names, CharCount, &CharCount
            );

        if ( Success ) 
        {
            break;
        }

        if ( GetLastError() != ERROR_MORE_DATA ) 
        {
            break;
        }

        //
        //  Try again with the
        //  new suggested size.
        delete [] Names;
        Names = NULL;
    }

    if ( Success )
    {
        //
        //  Display the various paths.
        for ( NameIdx = Names; 
              NameIdx[0] != L'\0'; 
              NameIdx += wcslen(NameIdx) + 1 ) 
        {
            wprintf(L"  %s", NameIdx);
        }
        wprintf(L"\n");
    }

    if ( Names != NULL ) 
    {
        delete [] Names;
        Names = NULL;
    }

    return;
}



// Search for volumes
void SearchVolumes() {
    DWORD  CharCount            = 0;
    WCHAR  DeviceName[MAX_PATH] = L"";
    DWORD  Error                = ERROR_SUCCESS;
    HANDLE FindHandle           = INVALID_HANDLE_VALUE;
    BOOL   Found                = FALSE;
    size_t Index                = 0;
    BOOL   Success              = FALSE;
    WCHAR  VolumeName[MAX_PATH] = L"";

    //
    //  Enumerate all volumes in the system.
    FindHandle = FindFirstVolumeW(VolumeName, ARRAYSIZE(VolumeName));

    if (FindHandle == INVALID_HANDLE_VALUE)
    {
        Error = GetLastError();
        wprintf(L"FindFirstVolumeW failed with error code %d\n", Error);
        return;
    }

    for (;;)
    {
        //
        //  Skip the \\?\ prefix and remove the trailing backslash.
        Index = wcslen(VolumeName) - 1;

        if (VolumeName[0]     != L'\\' ||
            VolumeName[1]     != L'\\' ||
            VolumeName[2]     != L'?'  ||
            VolumeName[3]     != L'\\' ||
            VolumeName[Index] != L'\\') 
        {
            Error = ERROR_BAD_PATHNAME;
            wprintf(L"FindFirstVolumeW/FindNextVolumeW returned a bad path: %s\n", VolumeName);
            break;
        }

        //
        //  QueryDosDeviceW does not allow a trailing backslash,
        //  so temporarily remove it.
        VolumeName[Index] = L'\0';

        CharCount = QueryDosDeviceW(&VolumeName[4], DeviceName, ARRAYSIZE(DeviceName)); 

        VolumeName[Index] = L'\\';

        if ( CharCount == 0 ) 
        {
            Error = GetLastError();
            wprintf(L"QueryDosDeviceW failed with error code %d\n", Error);
            break;
        }

        wprintf(L"\nFound a device:\n %s", DeviceName);
        wprintf(L"\nVolume name: %s", VolumeName);
        wprintf(L"\nPaths:");
        DisplayVolumePaths(VolumeName);

        //
        //  Move on to the next volume.
        Success = FindNextVolumeW(FindHandle, VolumeName, ARRAYSIZE(VolumeName));

        if ( !Success ) 
        {
            Error = GetLastError();

            if (Error != ERROR_NO_MORE_FILES) 
            {
                wprintf(L"FindNextVolumeW failed with error code %d\n", Error);
                break;
            }

            //
            //  Finished iterating
            //  through all the volumes.
            Error = ERROR_SUCCESS;
            break;
        }
    }

    FindVolumeClose(FindHandle);
    FindHandle = INVALID_HANDLE_VALUE;

    return;

}




// Testing Microsoft API for opening files and devices

void loadCard(void)
{
    HANDLE hFile;
    DWORD dwBytesRead = 0;
	char ReadBuffer[BUFFER_SIZE] = {0};
//	LPCTSTR fileName = "C:\\test.txt";
//	LPCTSTR fileName = "\\\\.\\CdRom0";
	LPCTSTR fileName = "\\\\.\\F:";
 
	printf("\n");

//    hFile = CreateFile("\\\\.\\E:",GENERIC_READ /*|GENERIC_WRITE*/,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	hFile = CreateFile(
		fileName,               //read a volume					LPCTSTR lpFileName,
		GENERIC_READ, //| GENERIC_WRITE, //access					DWORD dwDesiredAccess,
		FILE_SHARE_READ|FILE_SHARE_WRITE,                      //disables any sharing operation	DWORD dwShareMode,
		NULL,                   //no security descriptor			LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		OPEN_EXISTING,          //it's a volume, that's what MSDN recommends    DWORD dwCreationDisposition,
		FILE_ATTRIBUTE_NORMAL,  //wtf should go here?				DWORD dwFlagsAndAttributes,
		NULL);                  //a template file something			HANDLE hTemplateFile
 
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Could not open file (error %d)\n", GetLastError());
        return;
    }
 
	printf("OMG IT'S OPEN!!!\n");



    // Read one character less than the buffer size to save room for
    // the terminating NULL character.
/* 
    if( FALSE == ReadFile(hFile, ReadBuffer, BUFFER_SIZE-1, &dwBytesRead, NULL) ) {
        printf("Could not read from file (error %d)\n", GetLastError());
        CloseHandle(hFile);
        return;
    }

	printf("PAST READFILE!!!\n");

 
// This is the section of code that assumes the file is ANSI text. 
    // Modify this block for other data types if needed.

    if (dwBytesRead > 0 && dwBytesRead <= BUFFER_SIZE-1) {
        ReadBuffer[dwBytesRead]='\0'; // NULL character

        _tprintf(TEXT("Data read from %s (%d bytes): \n"), fileName, dwBytesRead);
        printf("%s\n", ReadBuffer);
    }
    else if (dwBytesRead == 0) {
        _tprintf(TEXT("No data read from file %s\n"), fileName);
    } else {
        printf("\n ** Unexpected value for dwBytesRead ** \n");
    }
*/

	
// If the CD-ROM drive was successfully opened, read sectors 16
   // and 17 from it and write their contents out to a disk file.
	const size_t sizer=5120;
	char *data=new(std::nothrow) char[sizer];       //CLEANUP
	ZeroMemory(data, sizer);

	DISK_GEOMETRY_EX *GeometryEx = (DISK_GEOMETRY_EX*)data;
	DWORD Len;

//		HANDLE h = CreateFileA("\\\\?\\D:", GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if(hFile!=INVALID_HANDLE_VALUE) {
			DeviceIoControl(hFile, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, 0 ,0, GeometryEx, sizer, &Len, 0);
			printf("Disk Size:  %I64u\n", GeometryEx->DiskSize.QuadPart);
			printf("Media Type:  %u\n", GeometryEx->Geometry.MediaType);
			printf("Bytes/Sector: %u\n", GeometryEx->Geometry.BytesPerSector);
		}
		else {
			printf("Fail: %u\n", GetLastError());
		}
	
	// It is always good practice to close the open file handles even though
    // the app will exit here and clean up open handles anyway.
    
    CloseHandle(hFile);
	delete data;
}



int main(int argc, int argv[]) {
	
	std::cout << "You have just entered the realm of awesome.\n";


	// Find all volumes and display information
	SearchVolumes();

	// Open a file/volume
	loadCard();

	std::cin.get();
	return 0;

}


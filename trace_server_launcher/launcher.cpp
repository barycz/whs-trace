#include "launcher.h"
#include "test_server.h"
#include <cstring>
#include <cstdint>
#include <cstddef>
#include <sysfn/sha1.h>
#include <windows.h>

namespace trace {

typedef char T_Sha1[SHA1_DIGEST_SIZE];

bool getHashFromFile (char const * fname, T_Sha1 sha)
{
	FILE * file;
	if (0 == fopen_s(&file, fname, "rb"))
	{
		printf("launcher: hashing file %s\n", fname);
		int const res = sha1_stream(file, sha);
		fclose(file);
		return res == 0;
	}
	return false;
}

bool compareFiles (char const * filename0, char const * filename1)
{
	T_Sha1 hash0;
	memset(hash0, 0, SHA1_DIGEST_SIZE);
	T_Sha1 hash1;
	memset(hash1, 0, SHA1_DIGEST_SIZE);

	bool const do_compare = getHashFromFile(filename0, hash0) && getHashFromFile(filename1, hash1);
	if (do_compare)
		return 0 == memcmp(hash0, hash1, SHA1_DIGEST_SIZE);
	return false;
}

bool fileExists (char const * fname)
{
	FILE * file;
	if (0 == fopen_s(&file, fname, "rb"))
	{
		fclose(file);
		return true;
	}
	return false;
}

void runTraceServer (char const * runname, char const * args)
{
	STARTUPINFO startupInfo;
	PROCESS_INFORMATION processInformation;

	ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);
	//startupInfo.dwFlags = STARTF_USESHOWWINDOW;
	//startupInfo.wShowWindow = SW_MINIMIZE;
	ZeroMemory(&processInformation, sizeof(processInformation));

	if (CreateProcess(runname, const_cast<char *>(args), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startupInfo, &processInformation))
	{
		CloseHandle(processInformation.hProcess);
		CloseHandle(processInformation.hThread);
	}
}

bool tryCopyTraceServer (char const * origname, char const * runname)
{
	DWORD targetAttribs = GetFileAttributes(runname);
	if(targetAttribs == INVALID_FILE_ATTRIBUTES)
	{
		printf("launcher: cannot get destination file attributes, the file probably does not exist\n");
	}
	else
	{
		if(targetAttribs & FILE_ATTRIBUTE_READONLY)
		{
			printf("launcher: destination is read-only, we will change the attribute.\n");
			targetAttribs &= ~FILE_ATTRIBUTE_READONLY;
			if(!SetFileAttributes(runname, targetAttribs))
			{
				printf("launcher: cannot clear destination read-only attribute\n");
			}
		}
	}

	if (BOOL res = CopyFile(origname, runname, false))
	{
		printf("launcher: copied to destination.\n");
		return true;
	}
	printf("launcher: copy failed with error=0x%08x\n", GetLastError());
	return false;
}

bool tryShutdownRunningServer (unsigned max_count)
{
	unsigned count = 0;
	while (isTraceServerRunning())
	{
		tryTraceServerShutdown();
		Sleep(250);
		if (count >= max_count / 2)
		{
			printf("launcher: old version not responding. never give up & never surrender!\n");
			Sleep(1000);
		}
		if (count >= max_count)
		{
			printf("launcher: old version not responding. d'oh i give up!\n");
			return false;
		}
		++count;
	}
	return true;
}

bool tryUpdateTraceServer (char const * origname, char const * runname)
{
	if (!fileExists(origname))
	{
		printf("launcher: Error: source file not present, nothing to do\n");
		return false;
	}
	if (!fileExists(runname))
	{
		printf("launcher: dest file not existing, copying...\n");
		tryCopyTraceServer(origname, runname);
		return false;
	}

	printf("launcher: both files exists, checking...\n");
	if (false == compareFiles(origname, runname))
	{
		printf("launcher: files differ!\n");
		return true;
	}
	else
	{
		printf("launcher: both files are up to date.\n");
		return false;
	}
}

} // namespace


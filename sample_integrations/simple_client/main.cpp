#include <trace_client/trace.h>
#include <cmath>

#include <cstdio>
#include <cstdlib>
#include "thrpool.h"

void sleep_ms (int ms)
{
#if defined WIN32 || defined WIN64
	Sleep(ms);
#elif defined __linux__
	usleep(ms * 1000);
#endif

}

void my_custom_vaarg_fn (char const * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	TRACE_MSG_VA(trace::e_Info, trace::CTX_Default, fmt, args);
	va_end(args);
}

unsigned g_Quit = 0;

#if defined WIN32 || defined WIN64
DWORD WINAPI thread_func ( LPVOID )
#elif defined __linux__
void * thread_func ( void * )
#endif
{
	TRACE_SCOPE(trace::e_Info, trace::CTX_Default);
	while (!g_Quit)
	{
		static int i = 0;
		++i;
		TRACE_MSG(trace::e_Info, trace::CTX_Default,  "Thread tick i=%u", i);
		sleep_ms(100);
	}
	return 0;
}

void foo ()
{
	TRACE_SCOPE(trace::e_Info, trace::CTX_Default);
	TRACE_MSG(trace::e_Info, trace::CTX_Default,  "%s %s", "\'llo world froom foo().", "and from bar!");
}

struct Bar
{
	Bar ()
	{
		TRACE_SCOPE(trace::e_Info, trace::CTX_Default);
	}

	~Bar ()
	{
		TRACE_SCOPE(trace::e_Info, trace::CTX_Default);
	}
};

static int g_ctrl = 0;

#if defined WIN32 || defined WIN64
//int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
int main ()
#elif defined __linux__
int main ()
#endif
{
#if defined WIN32
	setvbuf(stdout, 0, _IONBF, 0);
#endif
	TRACE_APPNAME("Simple Client");
	TRACE_CONNECT();
	TRACE_SCOPE(trace::e_Error, trace::CTX_Default);
	TRACE_MSG(trace::e_Info, trace::CTX_Default,  "Text with Error inside");
	{
		for (int i = 0; i < 64; ++i)
		{
			float x = 3.1415926535f * 2.0f / 128.0f * static_cast<float>(i);
			TRACE_PLOT_XY(trace::e_Info, trace::CTX_Default, x, sinf(x), "sample_plot/sin(x)");
		}

		TRACE_MSG(trace::e_Warning, trace::CTX_Default,	"first message\nsecond line"); // not sure if this is a valid case!
		TRACE_MSG(trace::e_Warning, trace::CTX_Game,	"first game message\nsecond line"); // not sure if this is a valid case
		TRACE_MSG(trace::e_Error, trace::CTX_Game,	"first game error\nsecond line");
		TRACE_MSG(trace::e_Error, trace::CTX_Game,	"second game error\nsecond line of that");
		TRACE_MSG(trace::e_Warning, trace::CTX_Default,  "First warning, errno=0x%08x", 0xFEEDDEAD);
		TRACE_MSG(trace::e_Warning, trace::CTX_Default,  "Second warning, errno=0x%08x", 0xFEEDDEAD);

		foo();
		Bar bar;

		ThreadPool<2> thr_pool;
		thr_pool.Create(thread_func, 0);

		for (;;)
		{
			static int i = 0;
			sleep_ms(200);

			TRACE_MSG(trace::e_Info, trace::CTX_Default,  "Some message i=%u from main thread", i);
      TRACE_MSG(trace::e_Debug, trace::CTX_Default,  "Some detailed message i=%u from main thread", i);
			++i;

			if (i == 1000)
				break;
		}
		g_Quit = 1;
		thr_pool.WaitForTerminate();
	}

	TRACE_DISCONNECT();
}

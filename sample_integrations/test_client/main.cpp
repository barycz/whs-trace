#include "sample_trace.h"
#include <cmath>
#include "folder/some_header.h"

#include <cstdio>
#include <cstdlib>
#include "thrpool.h"


void my_custom_vaarg_fn (char const * fmt, ...)
{
	TRACE_GANTT_SCOPE(trace::e_Info, trace::CTX_Default, "aa0/g0/%s", __FUNCTION__);
	va_list args;
	va_start(args, fmt);
	TRACE_MSG_VA(trace::e_Info, trace::CTX_Default, fmt, args);
	va_end(args);
}

void something_useful_too ()
{
	TRACE_GANTT_SCOPE(trace::e_Info, trace::CTX_Default, "aa0/g0/%s", __FUNCTION__);
	TRACE_SCOPE(trace::e_Info, trace::CTX_Game);
	TRACE_SCOPE_MSG(trace::e_Info, trace::CTX_Default, "%s %s", "nu", "pagadi");
	TRACE_MSG(trace::e_Info, trace::CTX_Default,  "%s", "Worker thread issues some another annoying message");
	for (int i = 0; i < 12; ++i)
	{
		TRACE_GANTT_SCOPE(trace::e_Info, trace::CTX_Default, "aa0/g0/ble [%i]", i);

		TRACE_GANTT_SCOPE(trace::e_Info, trace::CTX_Default, "aa0/g0/ble2 [%i]", -i);
		TRACE_MSG(trace::e_Info, trace::CTX_Default,  "%s %i", "ble :)", i);
		Sleep(170);
	}
}
	
void something_useful ()	
{
	TRACE_GANTT_SCOPE(trace::e_Info, trace::CTX_Default, "aa0/g0/%s", __FUNCTION__);
	TRACE_SCOPE(trace::e_Info, trace::CTX_Default);
	something_useful_too();
}


char const * GetName () { return "aa"; }

void TraceVal (int x, int y)
{
	TRACE_GANTT_SCOPE(trace::e_Info, trace::CTX_Default, "aa0/g0/filling table[x=%i, y=%i]", x,y);
	//Sleep(100);
	
	//TRACE_GANTT_BGN(trace::e_Info, trace::CTX_Default, "aa0/g0/table[x=%i y=%i]", x, y);
	static int n = 0;
	TRACE_TABLE(trace::e_Info, trace::CTX_Default, x, y, "%s0/%i",GetName(), n);
	TRACE_TABLE(trace::e_Info, trace::CTX_Default, x, y, "%s1/%i",GetName(), n);		
	TRACE_TABLE(trace::e_Info, trace::CTX_Default, x, y, "%s1/%i|row=%i|col=%i|11|22|33|44|55|66",GetName(), n, y, x);
	TRACE_TABLE(trace::e_Info, trace::CTX_Default, x, y, trace::Color(255,0,0,255), trace::Color(75,75,75,75), "%s2/Error: %i",GetName(), n);
	//TRACE_TABLE_COLOR(trace::e_Info, trace::CTX_Default, x + 1	, y, trace::Color(0,0,255,255), "%s0", GetName());
	//TRACE_TABLE_COLOR(trace::e_Info, trace::CTX_Default, x	, y, trace::Color(255,0,0,255), "%s2/", GetName());
	//TRACE_TABLE_COLOR(trace::e_Info, trace::CTX_Default, x	, y, trace::Color(255,0,255,255), trace::Color(0,0,0,0), "%s0", GetName());
	TRACE_GANTT_SCOPE(trace::e_Info, trace::CTX_Default, "aa0/g0/tblcolor%i", x);
	TRACE_TABLE_COLOR(trace::e_Info, trace::CTX_Default, x + 1	, y, trace::Color(255,0,255,255), trace::Color(0,255,0,0), "%s0", GetName());
	//Sleep(50);
	++n;
	//TRACE_GANTT_END(trace::e_Info, trace::CTX_Default, "aa0/g0/table");
}
	
unsigned g_Quit = 0;

#if defined WIN32 || defined WIN64
DWORD WINAPI do_something ( LPVOID )
#elif defined __linux__
void * do_something ( void * )
#endif
{
	TRACE_GANTT_SCOPE(trace::e_Info, trace::CTX_Default, "aa0/g1/thread main %x", GetCurrentThreadId());
	TRACE_SCOPE(trace::e_Info, trace::CTX_Default);
	something_useful();
	while (!g_Quit)
	{
		static int i = 0;
		++i;
		TRACE_GANTT_SCOPE(trace::e_Info, trace::CTX_Default, "aa0/g1/thread tick %x[tick=%i]", GetCurrentThreadId	(), i);
		TRACE_MSG(trace::e_Info, trace::CTX_Default,  "Thread tick i=%u", i);
#if defined WIN32 || defined WIN64
		Sleep(100);
#elif defined __linux__
		usleep(300 * 1000);
#endif
	}
	return 0;
}

void foo ()
{
	TRACE_SCOPE(trace::e_Info, trace::CTX_Default);
	TRACE_MSG(trace::e_Info, trace::CTX_Default,  "%s %s", "\'lloo woorld froom foo().", "and from bar!");
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
	TRACE_APPNAME("Sample_App");
	TRACE_CONNECT();
	TRACE_SCOPE(trace::e_Error, trace::CTX_Default);
	TRACE_MSG(trace::e_Info, trace::CTX_Default,  "Text with Error inside");
	{
		TRACE_GANTT_SCOPE(trace::e_Info, trace::CTX_Default, "aa0/g0/%s", __FUNCTION__);
		//TRACE_GANTT_BGN(trace::e_Info, trace::CTX_Default, "aa0/g0/%s", __FUNCTION__);
		for (int k = 0; k <   1; ++k)
			for (int i = 0; i < 64; ++i)
			{
				float x = 3.1415926535f * 2.0f / 128.0f * static_cast<float>(i);
				TRACE_PLOT_XY(trace::e_Info, trace::CTX_Default, x, sinf(x), "sample_plot%i/%s", k, "sin");
				TRACE_PLOT_XY(trace::e_Info, trace::CTX_Default, x, cosf(x), "sample_plot%i/%s", k, "cos");
				TRACE_PLOT_XY(trace::e_Info, trace::CTX_Default, x, sinf(x) * cosf(x), "sample_plot2%i/%s", k, "cos");
				TRACE_PLOT_XY(trace::e_Info, trace::CTX_Default, x, cosf(x) / x, "sample_plot3%i/%s", k, "hadej");
				TRACE_PLOT_XY(trace::e_Info, trace::CTX_Default, x, sinf(x) * sinf(x)/ x, "sample_plot4%i/%s", k, "hadej");
			}
		//TRACE_DISCONNECT();
		//return 0;


		TRACE_MSG(trace::e_Warning, trace::CTX_Default,	"first message\nsecond line"); // not sure if this is a valid case!
		TRACE_MSG(trace::e_Warning, trace::CTX_Game,	"first game message\nsecond line"); // not sure if this is a valid case
		TRACE_MSG(trace::e_Error, trace::CTX_Game,	"first game error\nsecond line");
		TRACE_MSG(trace::e_Error, trace::CTX_Game,	"second game error\nsecond line of that");
		TRACE_MSG(trace::e_Warning, trace::CTX_Default,  "First warning, errno=%x", 0xFEEDDEAD);
		TRACE_MSG(trace::e_Warning, trace::CTX_Default,  "Second warning, errno=%x", 0xFEEDDEAD);
		/*TRACE_MSG(trace::e_Info, trace::CTX_Default,	"this is %s", "first message");
		TRACE_MSG(trace::				e_Info, trace::CTX_Default,  "args: %s and %s", "first arg", "second arg");
		TRACE_MSG(trace::e_Fatal, trace::CTX_Default,  "First fatal error, errno=%08x", 0xDEAFDAD);
		TRACE_MSG(trace::e_Error, trace::CTX_Default,  "First error, errno=%08x", 0xBADBEEF);
		TRACE_MSG(trace::e_Warning, trace::CTX_Default,  "First warning, errno=%x", 0xFEEDDEAD);
		TRACE_MSG(trace::e_Detail, trace::CX_Default,	"%s%s", "This message should not appear", ".");
		TRACE_MSG(trace::e_Info, trace::CTX_Default,  "%s", "This message should appear.");
		TRACE_MSG(trace::e_Info, trace::CTX_Default,  "%s", "This message should partially appear too, but it's much longer in time and space so that it's very annoying and everyone will hate it as i do hate it now during typing as approaching to some 256 bytes boundary on which this message will be clipped and therefore it does not make any sense at all as all it does do is to show you in a rather graphomaniac light like Robert Smith or this Rowling bitch");*/
		my_custom_vaarg_fn("using va_arg macro %s and %s", "with some argument", "another one");

		TRACE_TABLE_HHEADER(trace::e_Info, trace::CTX_Default, 1, "hdr1", "%s0","aa");
		TRACE_TABLE_HHEADER(trace::e_Info, trace::CTX_Default, 2, "hdr2", "%s0","aa");
		//TRACE_TABLE_HHEADER(trace::e_Info, trace::CTX_Default, 3, "hdr3", "%s1","aa");
		//TRACE_TABLE_HHEADER(trace::e_Info, trace::CTX_Default, 4, "hdr4", "%s2","aa");
		foo();	
		Bar bar;

		ThreadPool<2> thr_pool;
		thr_pool.Create(do_something, 0);

		for (;;)
		{
		
			static int i	 = 0;
			TRACE_GANTT_FRAME_BGN(trace::e_Info, trace::CTX_Default, "aa0/g0/frame %i...", i);
			//TRACE_GANTT_SCOPE(trace::e_Info, trace::CTX_Default, "aa0/g0/main loop[tick=%i]", i);
	#if defined WIN32 || defined WIN64
			Sleep(200);	
	#elif defined __linux__	
			usleep(2000 * 1000);	
	#endif

			TRACE_GANTT_SCOPE(trace::e_Info, trace::CTX_Default, "aa0/g0/tracing table[tick=%i]", i);
			TRACE_MSG(trace::e_Info, trace::CTX_Default,	"%s", "This message should periodicaly appear too.");
		
			//for(size_t i = 0; i < 4; ++i)

			TRACE_MSG(trace::e_Info, trace::CTX_Default,  "Some another annoying message i=%u from main thread", i);
					TraceVal((2 * i) % 16, i * 2);
			++i;	

			//TRACE_TABLE(trace::e_Info, trace::CTX_Default, 0, 0, "hokus/%i|%i|%i", i, i*i, i*i*i);
			//TRACE_TABLE(trace::e_Info, trace::CTX_Default, 1, 1, "hokus/%i|%i|%i", 2 * i, 2 * i*i, 2 * i*i*i);
			//TRACE_TABLE(trace::e_Info, trace::CTX_Default, 0, -1, "pokus/%i|%i", i, -i);
			//TRACE_TABLE(trace::e_Info, trace::CTX_Default, 0, -1, "pokus/%i|%i", i, -i);
			//TRACE_TABLE(trace::e_Info, trace::CTX_Default,-1,  1, "fookus/%f", float(i) * 3.1415926f);

			/*if (i == 5)
			{
				TRACE_TABLE(trace::e_Info, trace::CTX_Default, 3, 3, "%s0/%i",GetName(), 555);
			}
			

			if (i == 6)
			{
				TRACE_TABLE(trace::e_Info, trace::CTX_Default, 1, 1, "%s1/%i|1|2|3|4|5|6",GetName(), 666);
			}*/

			if (i == 1000)
			{
				Sleep(100);
				break;
			}



			//TRACE_MSG(trace::e_Info, trace::CTX_Default,  "Some warning message i=%u from main thread", i);
			folder::some_fn(i);
			//TRACE_MSG(trace::e_Info, trace::CTX_Default,  "grr284 i=%u ", i);
			//TRACE_MSG(trace::e_Info, trace::CTX_Default,  "grr285 i=%u ", i);
			TRACE_MSG(trace::e_Info, trace::CTX_Default,  "grr286 i=%u ", i);
			TRACE_MSG(trace::e_Info, trace::CTX_Default,  "grr286 i=%u ", i);

			Sleep ((rand() % 256	));
		
			//TRACE_GANTT_END(trace::e_Info, trace::CTX_Default, "aa0/g0/end", i);
			TRACE_GANTT_FRAME_END(trace::e_Info, trace::CTX_Default, "aa0/g0/frame %i", i);

			/*if (i==55)
			{
				TRACE_PLOT_CLEAR(trace::e_Info, trace::CTX_Default, "sample_plot%i/%s", 0, "sin");
				TRACE_TABLE_CLEAR(trace::e_Info, trace::CTX_Default, "aa0");	
				TRACE_GANTT_CLEAR(trace::e_Info, trace::CTX_Default, "aa0/g0");
				break;
			}*/		
		}
		//TRACE_GANTT_END(trace::e_Info, trace::CTX_Default, "aa0/g0/%s", __FUNCTION__);
		//TRACE_GANTT_END(trace::e_Info, trace::CTX_Default, "aa0/g0/Entered %s...", __FUNCTION__);
		g_Quit = 1;
		thr_pool.WaitForTerminate();
	}

	Sleep(2000);
	TRACE_DISCONNECT();
#if defined WIN32 || defined WIN64
		Sleep(2000);
#elif defined __linux__
		usleep(2000 * 1000);
#endif


}

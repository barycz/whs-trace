#pragma once
#if defined WIN32 || defined WIN64
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>	
#	include <cstdio>

namespace {
	template<size_t N>
	struct ThreadPool
	{
		enum { e_thread_count = N };
		DWORD	m_tids[e_thread_count];
		HANDLE	m_handles[e_thread_count];

		ThreadPool () { memset(this, 0, sizeof(*this)); }
		~ThreadPool () { Close(); }
		void Create (DWORD (WINAPI * fn) (void *), void * ) {
			TRACE_SCOPE(trace::e_Info, trace::CTX_Default);
			for (size_t i = 0; i < e_thread_count; i++ )
				m_handles[i] = CreateThread( NULL, 0, fn, 0, 0, &m_tids[i]); 
		}
		void WaitForTerminate () { WaitForMultipleObjects(e_thread_count, m_handles, TRUE, INFINITE); }
		void Close () {
			for (size_t i = 0; i < e_thread_count; i++)
				CloseHandle(m_handles[i]);
		}
	};
}

#elif defined __linux__
#	include <pthread.h>
#	include <cstring>
#	include <cstdio>
#	include <cstdlib>
#	include <unistd.h>
#	include <errno.h>
#	include <ctype.h>
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE 1
#	endif

#	define handle_error_en(en, msg) do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)
#	define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

	struct thread_info {    /* Used as argument to thread_start() */
		pthread_t thread_id;        /* ID returned by pthread_create() */
		int       thread_num;       /* Application-defined thread # */
		char     *argv_string;      /* From command-line argument */
	};

	template<size_t N>
	struct ThreadPool
	{
		enum { e_thread_count = N };
		pthread_attr_t m_attr;
		thread_info m_tinfo[e_thread_count];

		ThreadPool () { memset(this, 0, sizeof(*this)); }
		~ThreadPool () { Close(); }
		void Create (void * (* fn) (void *), void * )
		{
			/* Initialize thread creation attributes */
			int s = pthread_attr_init(&m_attr);
			if (s != 0)
				handle_error_en(s, "pthread_attr_init");
			/*if (stack_size > 0)
			{
				s = pthread_attr_setstacksize(&m_attr, stack_size);
				if (s != 0)
					handle_error_en(s, "pthread_attr_setstacksize");
			}*/

			/* Create one thread for each command-line argument */
			for (size_t t = 0; t < N; ++t)
			{
				m_tinfo[t].thread_num = t + 1;
				//m_tinfo[t].argv_string = argv[optind + t];

				pthread_attr_t m_attr;
				pthread_attr_init(&m_attr);
				cpu_set_t cpuset;

				CPU_ZERO(&cpuset);
				CPU_SET(t, &cpuset);
				//CPU_SET(atoi(m_tinfo[t].argv_string), &cpuset);
				pthread_attr_setaffinity_np(&m_attr, sizeof(cpuset), &cpuset);

				/* The pthread_create() call stores the thread ID into corresponding element of m_tinfo[] */
				s = pthread_create(&m_tinfo[t].thread_id, &m_attr, fn, &m_tinfo[t]);
				if (s != 0)
					handle_error_en(s, "pthread_create");
			}
		}

		void WaitForTerminate ()
		{
			/* Destroy the thread attributes object, since it is no longer needed */
			int s = pthread_attr_destroy(&m_attr);
			if (s != 0)
				handle_error_en(s, "pthread_attr_destroy");

			/* Now join with each thread, and display its returned value */
			for (size_t t = 0; t < N; t++)
			{
				void * res = 0;
				s = pthread_join(m_tinfo[t].thread_id, &res);
				if (s != 0)
				   handle_error_en(s, "pthread_join");

				printf("Joined with thread %d; returned value was %s\n", m_tinfo[t].thread_num, (char *) res);
				free(res);      /* Free memory allocated by thread */
			}

		}
		void Close ()
		{
		}
	};

#endif



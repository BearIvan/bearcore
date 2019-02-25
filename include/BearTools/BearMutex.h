#pragma once
namespace BearCore
{
	class BEARTOOL_API BearMutex
	{
	public:
		BEAR_CLASS_NO_COPY(BearMutex);
		BearMutex();
		~BearMutex();
		void Lock();
		void Unlock();
		void Swap(BearMutex&right);
	private:
#ifdef WINDOWS
		CRITICAL_SECTION m_mutex;
#elif LINUX 
		pthread_mutex_t m_mutex;
#endif
	};
	class BearMutexLock
	{
		BEAR_CLASS_NO_COPY(BearMutexLock);
		BearMutex&m_mutex;
	public:
		BearMutexLock(BearMutex&mutex) :m_mutex(mutex) { m_mutex.Lock(); }
		inline ~BearMutexLock() { m_mutex.Unlock(); }
	};
}
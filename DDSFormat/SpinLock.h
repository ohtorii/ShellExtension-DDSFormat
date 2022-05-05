#pragma once
#ifndef	DDSFORMAT_SPINLOCK_H
#define	DDSFORMAT_SPINLOCK_H
#include "pch.h"

namespace dds_format {
    class SpinLock {
    public:
        SpinLock(const SpinLock&) = delete;
        SpinLock& operator=(const SpinLock&) = delete;

        SpinLock() {
            const DWORD defaultValue = 512;
            auto _ = InitializeCriticalSectionAndSpinCount(&m_cs, defaultValue);
        }
        ~SpinLock() {
            DeleteCriticalSection(&m_cs);
        }
        void Enter() {
            EnterCriticalSection(&m_cs);
        }
        void Leave() {
            LeaveCriticalSection(&m_cs);
        }
    private:
        CRITICAL_SECTION m_cs;
    };

    class Monitor {
    public:
        Monitor(const Monitor&) = delete;
        Monitor& operator=(const Monitor&) = delete;

        Monitor(SpinLock& lock) : m_lock(lock) {
            m_lock.Enter();
        }
        ~Monitor() {
            m_lock.Leave();
        }
        SpinLock& m_lock;
    };
};

#endif /*DDSFORMAT_SPINLOCK_H*/

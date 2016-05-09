#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include "src/signal.h"
#ifdef NACL
#include <ppapi/cpp/module.h>
#include <ppapi/utility/completion_callback_factory.h>
#else
#include <QObject>
#include <QTimer>
#endif

namespace operation
{
    enum Status { Success, Failure, Timeout };

    template <typename T2 = void, typename T3 = void> class Base;

    template <>
    class Base<void, void>
    {
        public:
            Base(Signal<Status> &sig) : m_sig(sig) { }

            void fail()
            {
                m_sig(Failure);
            }

            void success()
            {
                m_sig(Success);
            }

            void timeout()
            {
                m_sig(Timeout);
            }

        private:
            Signal<Status> &m_sig;
    };

    template <typename T2>
    class Base<T2, void>
    {
        public:
            Base(Signal<Status, T2> &sig) : m_sig(sig) { }

            void fail(const T2 &t2=T2())
            {
                m_sig(Failure, t2);
            }

            void success(const T2 &t2)
            {
                m_sig(Success, t2);
            }

            void timeout(const T2 &t2=T2())
            {
                m_sig(Timeout, t2);
            }

        private:
            Signal<Status, T2> &m_sig;
    };

    template <typename T2, typename T3>
    class Base
    {
        public:
            Base(Signal<Status, T2, T3> &sig) : m_sig(sig) { }

            void fail(const T2 &t2=T2(),
                      const T3 &t3=T3())
            {
                m_sig(Failure, t2, t3);
            }

            void success(const T2 &t2,
                         const T3 &t3)
            {
                m_sig(Success, t2, t3);
            }

            void timeout(const T2 &t2=T2(),
                         const T3 &t3=T3())
            {
                m_sig(Timeout, t2, t3);
            }

        private:
            Signal<Status, T2, T3> &m_sig;
    };

    template <typename Self, typename T2 = void, typename T3 = void>
    struct Immediate : public Base<T2, T3>
    {
        Immediate(Signal<Status, T2, T3> &sig) : Base<T2, T3>(sig) { }

        void exec()
        {
            static_cast<Self *>(this)->run();
        }
    };

#ifdef NACL
    template <class Self, typename T2 = void, typename T3 = void>
    struct Deferred : public Base<T2, T3>
    {
        Deferred(Signal<Status, T2, T3> &sig) :
            Base<T2, T3>(sig),
            cb_factory(static_cast<Self *>(this)) { }

        void exec()
        {
            pp::Module::Get()->core()->CallOnMainThread(
                0, cb_factory.NewCallback(&Self::_run));
        }

        void _run(int32_t)
        {
            static_cast<Self *>(this)->run();
        }

        pp::CompletionCallbackFactory<Self> cb_factory;
    };
#else
    template <typename Self, typename T2 = void, typename T3 = void>
    struct Deferred : public QObject,
                      public Base<T2, T3>
    {
        Deferred(Signal<Status, T2, T3> &sig) : Base<T2, T3>(sig) { }

        void exec()
        {
            QTimer::singleShot(0, static_cast<Self *>(this), SLOT(run()));
        }
    };
#endif
}

// facilité pour que les ptr fonctionnent avec des forward declare ...
struct BasePrivImpl
{
    virtual ~BasePrivImpl() { }
};
#endif // COMMON_H

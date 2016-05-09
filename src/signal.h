#ifndef SIGNAL_H
#define SIGNAL_H

#include <set>
#include <map>
#include <string.h>
#include <iostream>
#include "src/ptr.h"

template <typename T1 = void, typename T2 = void,
          typename T3 = void> class Callback;
template <typename T,
          typename Method,
          typename T1 = void,
          typename T2 = void,
          typename T3 = void> class MethodCallback;
template <typename T1 = void, typename T2 = void,
          typename T3 = void> class Delegate;
template <typename T1 = void, typename T2 = void,
          typename T3 = void> class Signal;

// Pas l'implémentation la plus efficace ni la plus type-safe, mais ça fait la job !
template <>
class Callback<void, void, void>
{
    public:
        virtual ~Callback() { }
        virtual ptr<Callback> clone() const = 0;
        virtual void invoke() const               = 0;
        virtual bool lessThan(Callback *cb) const = 0;
};

template <typename T, typename Method>
class MethodCallback<T, Method, void, void, void> : public Callback<>
{
    public:
        MethodCallback(T     *object,
                       Method method) :
            m_object(object), m_method(method) { }

        ptr< Callback<> > clone() const
        {
            return make_ptr<MethodCallback>(*this);
        }

        void invoke() const
        {
            (m_object->*m_method)();
        }

        bool lessThan(Callback<> *cb) const
        {
            MethodCallback *mcb = static_cast<MethodCallback *>(cb);
            if (m_object == mcb->m_object)
            {
                return memcmp(&m_method, &mcb->m_method, sizeof(m_method)) < 0;
            }

            return m_object < mcb->m_object;
        }

    private:
        T     *m_object;
        Method m_method;
};

template <>
class Delegate<void, void, void>
{
    public:
        template <typename T, typename Method>
        Delegate(T     *object,
                 Method method) :
            m_cb(make_ptr< MethodCallback<T, Method> >(object, method))
        { }

        ~Delegate()
        { }

        Delegate(const Delegate &dlg) :
            m_cb(dlg.m_cb->clone())
        { }

        Delegate & operator=(const Delegate &dlg)
        {
            if (this != &dlg)
            {
                m_cb = dlg.m_cb->clone();
            }

            return *this;
        }

        void operator()() const
        {
            return m_cb->invoke();
        }

        bool operator<(const Delegate &dlg) const
        {
            return m_cb->lessThan(dlg.m_cb.get());
        }

    private:
        ptr< Callback<> > m_cb;
};

template <>
class Signal<void, void, void>
{
    typedef Delegate<>                       Deleg;
    typedef std::map<Deleg, int>             DelegToPosList;
    typedef std::map<Deleg, int>::value_type DelegToPosPair;
    typedef DelegToPosList::iterator         DelegToPosListIter;
    typedef std::map<int, Deleg>             PosToDelegList;
    typedef std::map<int, Deleg>::value_type PosToDelegPair;
    typedef PosToDelegList::iterator         PosToDelegListIter;

    public:
        Signal() : m_id(0) { }

        template <typename T, typename Method>
        void connect(T     *obj,
                     Method method)
        {
            Deleg              d(obj, method);
            DelegToPosListIter it = m_delToPos.find(d);
            if (it == m_delToPos.end())
            {
                m_delToPos.insert(DelegToPosPair(d, m_id));
                m_posToDel.insert(PosToDelegPair(m_id, d));
                m_id++;
            }
        }

        template <typename T, typename Method>
        void disconnect(T     *obj,
                        Method method)
        {
            Deleg              d(obj, method);
            DelegToPosListIter it = m_delToPos.find(d);
            if (it != m_delToPos.end())
            {
                int id = it->second;
                m_delToPos.erase(it);
                m_posToDel.erase(id);
            }
        }

        void operator()() const
        {
            // itére sur une copie, pour ne pas utiliser des itérateurs qui auraient été
            // invalidées par disconnect en réponse à une slot
            PosToDelegList posToDel = m_posToDel;
            for (PosToDelegListIter it = posToDel.begin(); it != posToDel.end(); ++it)
            {
                (it->second)();
            }
        }

    private:
        int            m_id;
        DelegToPosList m_delToPos;
        PosToDelegList m_posToDel;
};

template <typename Param1>
class Callback<Param1, void, void>
{
    public:
        virtual ~Callback() { }
        virtual ptr<Callback> clone() const = 0;
        virtual void invoke(Param1 p1) const      = 0;
        virtual bool lessThan(Callback *cb) const = 0;
};

template <typename T, typename Method, typename Param1>
class MethodCallback<T, Method, Param1, void, void> : public Callback<Param1>
{
    public:
        MethodCallback(T     *object,
                       Method method) :
            m_object(object), m_method(method) { }

        ptr< Callback<Param1> > clone() const
        {
            return make_ptr<MethodCallback>(*this);
        }

        void invoke(Param1 p1) const
        {
            (m_object->*m_method)(p1);
        }

        bool lessThan(Callback<Param1> *cb) const
        {
            MethodCallback *mcb = static_cast<MethodCallback *>(cb);
            if (m_object == mcb->m_object)
            {
                return memcmp(&m_method, &mcb->m_method, sizeof(m_method)) < 0;
            }

            return m_object < mcb->m_object;
        }

    private:
        T     *m_object;
        Method m_method;
};

template <typename Param1>
class Delegate<Param1, void, void>
{
    public:
        template <typename T, typename Method>
        Delegate(T     *object,
                 Method method) :
            m_cb(make_ptr< MethodCallback<T, Method, Param1> >(object, method))
        { }

        ~Delegate()
        { }

        Delegate(const Delegate &dlg) :
            m_cb(dlg.m_cb->clone())
        { }

        Delegate & operator=(const Delegate &dlg)
        {
            if (this != &dlg)
            {
                m_cb = dlg.m_cb->clone();
            }

            return *this;
        }

        void operator()(Param1 p1) const
        {
            return m_cb->invoke(p1);
        }

        bool operator<(const Delegate &dlg) const
        {
            return m_cb->lessThan(dlg.m_cb.get());
        }

    private:
        ptr< Callback<Param1> > m_cb;
};

template <typename Param1>
class Signal<Param1, void, void>
{
    typedef Delegate<Param1>                          Deleg;
    typedef std::map<Deleg, int>                      DelegToPosList;
    typedef typename std::map<Deleg, int>::value_type DelegToPosPair;
    typedef typename DelegToPosList::iterator         DelegToPosListIter;
    typedef std::map<int, Deleg>                      PosToDelegList;
    typedef typename std::map<int, Deleg>::value_type PosToDelegPair;
    typedef typename PosToDelegList::iterator         PosToDelegListIter;

    public:
        Signal() : m_id(0) { }

        template <typename T, typename Method>
        void connect(T     *obj,
                     Method method)
        {
            Deleg              d(obj, method);
            DelegToPosListIter it = m_delToPos.find(d);
            if (it == m_delToPos.end())
            {
                m_delToPos.insert(DelegToPosPair(d, m_id));
                m_posToDel.insert(PosToDelegPair(m_id, d));
                m_id++;
            }
        }

        template <typename T, typename Method>
        void disconnect(T     *obj,
                        Method method)
        {
            Deleg              d(obj, method);
            DelegToPosListIter it = m_delToPos.find(d);
            if (it != m_delToPos.end())
            {
                int id = it->second;
                m_delToPos.erase(it);
                m_posToDel.erase(id);
            }
        }

        void operator()(Param1 p1=Param1()) const
        {
            // itére sur une copie, pour ne pas utiliser des itérateurs qui auraient été
            // invalidées par disconnect en réponse à une slot
            PosToDelegList posToDel = m_posToDel;
            for (PosToDelegListIter it = posToDel.begin(); it != posToDel.end(); ++it)
            {
                (it->second)(p1);
            }
        }

    private:
        int            m_id;
        DelegToPosList m_delToPos;
        PosToDelegList m_posToDel;
};

template <typename Param1, typename Param2>
class Callback<Param1, Param2, void>
{
    public:
        virtual ~Callback() { }
        virtual ptr<Callback> clone() const = 0;
        virtual void invoke(Param1 p1,
                            Param2 p2) const      = 0;
        virtual bool lessThan(Callback *cb) const = 0;
};

template <typename T, typename Method, typename Param1, typename Param2>
class MethodCallback<T, Method, Param1, Param2, void> : public Callback<Param1, Param2>
{
    public:
        MethodCallback(T     *object,
                       Method method) :
            m_object(object), m_method(method) { }

        ptr< Callback<Param1, Param2> > clone() const
        {
            return make_ptr<MethodCallback>(*this);
        }

        void invoke(Param1 p1,
                    Param2 p2) const
        {
            (m_object->*m_method)(p1, p2);
        }

        bool lessThan(Callback<Param1, Param2> *cb) const
        {
            MethodCallback *mcb = static_cast<MethodCallback *>(cb);
            if (m_object == mcb->m_object)
            {
                return memcmp(&m_method, &mcb->m_method, sizeof(m_method)) < 0;
            }

            return m_object < mcb->m_object;
        }

    private:
        T     *m_object;
        Method m_method;
};

template <typename Param1, typename Param2>
class Delegate<Param1, Param2, void>
{
    public:
        template <typename T, typename Method>
        Delegate(T     *object,
                 Method method) :
            m_cb(make_ptr< MethodCallback<T, Method, Param1, Param2> >(object, method))
        { }

        ~Delegate()
        { }

        Delegate(const Delegate &dlg) :
            m_cb(dlg.m_cb->clone())
        { }

        Delegate & operator=(const Delegate &dlg)
        {
            if (this != &dlg)
            {
                m_cb = dlg.m_cb->clone();
            }

            return *this;
        }

        void operator()(Param1 p1,
                        Param2 p2) const
        {
            return m_cb->invoke(p1, p2);
        }

        bool operator<(const Delegate &dlg) const
        {
            return m_cb->lessThan(dlg.m_cb.get());
        }

    private:
        ptr< Callback<Param1, Param2> > m_cb;
};

template <typename Param1, typename Param2>
class Signal<Param1, Param2, void>
{
    typedef Delegate<Param1, Param2>                  Deleg;
    typedef std::map<Deleg, int>                      DelegToPosList;
    typedef typename std::map<Deleg, int>::value_type DelegToPosPair;
    typedef typename DelegToPosList::iterator         DelegToPosListIter;
    typedef std::map<int, Deleg>                      PosToDelegList;
    typedef typename std::map<int, Deleg>::value_type PosToDelegPair;
    typedef typename PosToDelegList::iterator         PosToDelegListIter;

    public:
        Signal() : m_id(0) { }

        template <typename T, typename Method>
        void connect(T     *obj,
                     Method method)
        {
            Deleg              d(obj, method);
            DelegToPosListIter it = m_delToPos.find(d);
            if (it == m_delToPos.end())
            {
                m_delToPos.insert(DelegToPosPair(d, m_id));
                m_posToDel.insert(PosToDelegPair(m_id, d));
                m_id++;
            }
        }

        template <typename T, typename Method>
        void disconnect(T     *obj,
                        Method method)
        {
            Deleg              d(obj, method);
            DelegToPosListIter it = m_delToPos.find(d);
            if (it != m_delToPos.end())
            {
                int id = it->second;
                m_delToPos.erase(it);
                m_posToDel.erase(id);
            }
        }

        void operator()(Param1 p1=Param1(),
                        Param2 p2=Param2()) const
        {
            // itére sur une copie, pour ne pas utiliser des itérateurs qui auraient été
            // invalidées par disconnect en réponse à une slot
            PosToDelegList posToDel = m_posToDel;
            for (PosToDelegListIter it = posToDel.begin(); it != posToDel.end(); ++it)
            {
                (it->second)(p1, p2);
            }
        }

    private:
        int            m_id;
        DelegToPosList m_delToPos;
        PosToDelegList m_posToDel;
};

template <typename Param1, typename Param2, typename Param3>
class Callback
{
    public:
        virtual ~Callback() { }
        virtual ptr<Callback> clone() const = 0;
        virtual void invoke(Param1 p1,
                            Param2 p2,
                            Param3 p3) const      = 0;
        virtual bool lessThan(Callback *cb) const = 0;
};

template <typename T, typename Method, typename Param1, typename Param2, typename Param3>
class MethodCallback : public Callback<Param1, Param2, Param3>
{
    public:
        MethodCallback(T     *object,
                       Method method) :
            m_object(object), m_method(method) { }

        ptr< Callback<Param1, Param2, Param3> > clone() const
        {
            return make_ptr<MethodCallback>(*this);
        }

        void invoke(Param1 p1,
                    Param2 p2,
                    Param3 p3) const
        {
            (m_object->*m_method)(p1, p2, p3);
        }

        bool lessThan(Callback<Param1, Param2, Param3> *cb) const
        {
            MethodCallback *mcb = static_cast<MethodCallback *>(cb);
            if (m_object == mcb->m_object)
            {
                return memcmp(&m_method, &mcb->m_method, sizeof(m_method)) < 0;
            }

            return m_object < mcb->m_object;
        }

    private:
        T     *m_object;
        Method m_method;
};

template <typename Param1, typename Param2, typename Param3>
class Delegate
{
    public:
        template <typename T, typename Method>
        Delegate(T     *object,
                 Method method) :
            m_cb(make_ptr< MethodCallback<T, Method, Param1, Param2, Param3> >(object,
                                                                               method))
        { }

        ~Delegate()
        { }

        Delegate(const Delegate &dlg) :
            m_cb(dlg.m_cb->clone())
        { }

        Delegate & operator=(const Delegate &dlg)
        {
            if (this != &dlg)
            {
                m_cb = dlg.m_cb->clone();
            }

            return *this;
        }

        void operator()(Param1 p1,
                        Param2 p2,
                        Param3 p3) const
        {
            return m_cb->invoke(p1, p2, p3);
        }

        bool operator<(const Delegate &dlg) const
        {
            return m_cb->lessThan(dlg.m_cb.get());
        }

    private:
        ptr< Callback<Param1, Param2, Param3> > m_cb;
};

template <typename Param1, typename Param2, typename Param3>
class Signal
{
    typedef Delegate<Param1, Param2, Param3>          Deleg;
    typedef std::map<Deleg, int>                      DelegToPosList;
    typedef typename std::map<Deleg, int>::value_type DelegToPosPair;
    typedef typename DelegToPosList::iterator         DelegToPosListIter;
    typedef std::map<int, Deleg>                      PosToDelegList;
    typedef typename std::map<int, Deleg>::value_type PosToDelegPair;
    typedef typename PosToDelegList::iterator         PosToDelegListIter;

    public:
        Signal() : m_id(0) { }

        template <typename T, typename Method>
        void connect(T     *obj,
                     Method method)
        {
            Deleg              d(obj, method);
            DelegToPosListIter it = m_delToPos.find(d);
            if (it == m_delToPos.end())
            {
                m_delToPos.insert(DelegToPosPair(d, m_id));
                m_posToDel.insert(PosToDelegPair(m_id, d));
                m_id++;
            }
        }

        template <typename T, typename Method>
        void disconnect(T     *obj,
                        Method method)
        {
            Deleg              d(obj, method);
            DelegToPosListIter it = m_delToPos.find(d);
            if (it != m_delToPos.end())
            {
                int id = it->second;
                m_delToPos.erase(it);
                m_posToDel.erase(id);
            }
        }

        void operator()(Param1 p1=Param1(),
                        Param2 p2=Param2(),
                        Param3 p3=Param3()) const
        {
            // itére sur une copie, pour ne pas utiliser des itérateurs qui auraient été
            // invalidées par disconnect en réponse à une slot
            PosToDelegList posToDel = m_posToDel;
            for (PosToDelegListIter it = posToDel.begin(); it != posToDel.end(); ++it)
            {
                (it->second)(p1, p2, p3);
            }
        }

    private:
        int            m_id;
        DelegToPosList m_delToPos;
        PosToDelegList m_posToDel;
};
#endif // SIGNAL_H

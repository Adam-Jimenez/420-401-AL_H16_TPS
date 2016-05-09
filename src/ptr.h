#ifndef PTR_H
#define PTR_H

// décommenter cette ligne lorsque vous voulez tester votre implémentation
// de votre classe ptr
// #define USE_OWN_PTR
#ifdef USE_OWN_PTR
// écriver ici le code de votre classe ptr
#else
// classe ptr de remplacement : pas fonctionnelle, elle provoque des fuites de mémoire
template <typename T>
struct ptr
{
    ptr(T *p=0) : m_ptr(p) { }
    ptr(const ptr &p) : m_ptr(p.m_ptr) { }
    template <typename U> ptr(const ptr<U> &p) : m_ptr(p.m_ptr) { }

    T * operator->() const { return m_ptr; }
    T * get() const { return m_ptr; }
    T & operator*() const { return *m_ptr; }
    bool empty() const { return m_ptr == 0; }

    T *m_ptr;
};

template <typename T>
bool operator==(const ptr<T> &p1,
                const ptr<T> &p2) { return p1.m_ptr == p2.m_ptr; }

template <typename T>
bool operator<(const ptr<T> &p1,
               const ptr<T> &p2) { return p1.m_ptr < p2.m_ptr; }

template <typename T>
ptr<T> make_ptr() { return new T; }

template <typename T, typename P1>
ptr<T> make_ptr(const P1 &p1) { return new T(p1); }

template <typename T, typename P1, typename P2>
ptr<T> make_ptr(const P1 &p1,
                const P2 &p2) { return new T(p1, p2); }

template <typename T, typename P1, typename P2, typename P3>
ptr<T> make_ptr(const P1 &p1,
                const P2 &p2,
                const P3 &p3) { return new T(p1, p2, p3); }

template <typename T, typename P1, typename P2, typename P3, typename P4>
ptr<T> make_ptr(const P1 &p1,
                const P2 &p2,
                const P3 &p3,
                const P4 &p4) { return new T(p1, p2, p3, p4); }

template <typename T>
ptr<T> make_non_owning_ptr(T *ptr) { return ptr; }
#endif
#endif // PTR_H

#ifndef PTR_H
#define PTR_H

#define USE_OWN_PTR
#ifdef USE_OWN_PTR
#include <iostream>

template <typename T>
class ptr
{
    public:
        // constr vide
        ptr() : m_ptr(0), m_references(new int(1)), m_owning(true) {
        }
        
        // constr copie
        ptr(const ptr &p) : m_ptr(p.m_ptr), m_references(p.m_references), m_owning(p.m_owning) { 
            if(m_references!=0)
                (*m_references)++;
        }

        // constr copie a partir d'un autre ptr intelligent
        template <typename U>
            ptr(const ptr<U> &p) : m_owning(true) {
                if( !(m_ptr=dynamic_cast<T*>(p.get())) ){ // si cast ne reussi pas
                    m_ptr = 0;
                    m_references = new int(0);
                }else{
                    m_references = p.getref();
                    if(m_references!=0){
                        (*m_references)++;
                    }
                }
            }

        // operateur = avec meme type
        ptr<T> operator=(const ptr<T>& p)
        { 
            //decr();
            if(m_references!=0){
                (*m_references)--;
                if(*m_references == 0 && m_owning){
                    delete m_ptr;
                    delete m_references;
                }
            }
            m_ptr = p.m_ptr; 
            m_references = p.m_references;
            if(m_references!=0) (*m_references)++;
            // owning?
            return *this;
        }
        
        // destructeur
        ~ptr(){
            decr();
            /*if(*m_references==0 && m_owning && m_ptr!=0){
                delete m_ptr;
                delete m_references;
            }*/
        }

        // operateur ->
        T * operator->() const { 
            return m_ptr; 
        }

        T * get() const { 
            return m_ptr; 
        }

        int* getref() const { 
            return m_references;
        }//debug

        // operateur *
        T& operator*() const { 
            return *m_ptr; 
        }

        bool empty() const {
            return m_ptr == 0; 
        }

        void incr(){ 
            if(m_references!=0) (*m_references)++;
        }

        void decr()
        {
            if(m_references!=0){
                (*m_references)--;
                if(m_ptr != 0 && *m_references==0 && m_owning) {
                    delete m_ptr; 
                    delete m_references;
                    m_ptr = 0;
                }
            }
        }
        
    private:
        T *m_ptr;
        int* m_references;
        bool m_owning;

        // constr private
        ptr(T* p, bool owning=true) : m_ptr(p), m_owning(owning), m_references(0) { 
            if(m_owning){
                m_references=new int(1);
            }
        }

        template <typename U>
            friend bool operator==(const ptr<U> &p1,
                    const ptr<U> &p2);

        template <typename U>
            friend bool operator<(const ptr<U> &p1,
                    const ptr<U> &p2);

        template <typename U>
            friend ptr<U> make_ptr();

        template <typename U, typename P1>
            friend ptr<U> make_ptr(const P1 &p1);

        template <typename U, typename P1, typename P2>
            friend ptr<U> make_ptr(const P1 &p1,const P2 &p2);

        template <typename U, typename P1, typename P2, typename P3>
            friend ptr<U> make_ptr(const P1 &p1, const P2 &p2,const P3 &p3);

        template <typename U, typename P1, typename P2, typename P3, typename P4>
            friend ptr<U> make_ptr(const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4);

        template <typename U>
            friend ptr<U> make_non_owning_ptr(U *ptr);

};

template <typename U>
bool operator==(const ptr<U> &p1,
                const ptr<U> &p2) { return p1.m_ptr == p2.m_ptr; }

template <typename U>
bool operator<(const ptr<U> &p1,
               const ptr<U> &p2) { return p1.m_ptr < p2.m_ptr; }

template <typename U>
ptr<U> make_ptr() {  // att: cree une copie!
    return ptr<U>(new U);
}

template <typename U, typename P1>
ptr<U> make_ptr(const P1 &p1) {
    return ptr<U>(new U(p1)); 
}

template <typename U, typename P1, typename P2>
ptr<U> make_ptr(const P1 &p1,
                const P2 &p2) {
    return ptr<U>(new U(p1, p2)); 
}

template <typename U, typename P1, typename P2, typename P3>
ptr<U> make_ptr(const P1 &p1,
                const P2 &p2,
                const P3 &p3) {
    return ptr<U>(new U(p1, p2, p3)); 
}

template <typename U, typename P1, typename P2, typename P3, typename P4>
ptr<U> make_ptr(const P1 &p1,
                const P2 &p2,
                const P3 &p3,
                const P4 &p4) {
    return ptr<U>(new U(p1, p2, p3, p4)); 
}

template <typename U>
ptr<U> make_non_owning_ptr(U *p_ptr) { 
    ptr<U> u_ptr(p_ptr, false);
    return u_ptr;
}


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

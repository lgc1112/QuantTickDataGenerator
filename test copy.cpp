#include <iostream>
#include <coroutine>

template<typename T>
class singleton
{
private:
    volatile static T* m_instance;
    static pthread_mutex_t mutex;
    singleton() {};
    ~singleton() {};
    singleton(const singleton&) {};
    singleton& operator=(const singleton&) {};
public:
    volatile static T* GetInstance();
};

template<typename T>
volatile  T* singleton<T>::GetInstance()
{
    if (m_instance == NULL){
        pthread_mutex_lock(&mutex);
        if (m_instance == NULL){
            m_instance = new T();
        }
        pthread_mutex_unlock(&mutex);
    }
    return m_instance;
}

template<typename T>
volatile T* singleton<T>::m_instance = NULL;

template<typename T>
pthread_mutex_t singleton<T>::mutex;

class A : public singleton<A>
{
public:
    int a = 100;
};

int main()
{
    auto a = A::GetInstance();
    a->a = 999;
    std::cout <<A::GetInstance()->a << std::endl;
}
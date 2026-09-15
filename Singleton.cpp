
#include<iostream>
#include<thread>
#include<string>
#include<cstdlib>
#include<mutex>
using namespace std;

class Logger{
   private:
   static atomic<Logger*> instance;
   static mutex m;
   Logger() = default;
   ~Logger() = default;
   Logger(const Logger&) = delete;
   Logger& operator=(const Logger&) = delete;
   Logger(Logger&&) = delete;
   Logger& operator=(Logger&&) = delete;

   public:
   static Logger* getInstance()
   { 
     Logger* ptr = instance.load(memory_order_acquire);
     if(ptr == nullptr)
     {
       lock_guard<mutex> lock(m);
       ptr = instance.load(memory_order_relaxed);
       if(ptr == nullptr)
      {
	ptr = new Logger();
        instance.store(ptr,memory_order_release);
	atexit(Logger::cleanup);
      }
     }
        return ptr;
   } 
   static void cleanup() { 
        Logger* ptr = instance.exchange(nullptr);
	delete instance; } 
   void log(string msg)
   {
	cout << "Printing the message" << msg << endl;
   }
};
atomic<Logger*> Logger::instance{nullptr};
mutex Logger::m;
void userA()
{

   Logger::getInstance()->log("I am userA");
}

void userB()
{
   Logger::getInstance()->log("I am userB");
}

int main()
{

thread t1(userA);
thread t2(userB);

t1.join();
t2.join();

return 0;
}


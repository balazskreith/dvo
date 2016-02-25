/*
 ============================================================================
 Name        : dvo.cpp
 Author      : Balázs Kreith
 Version     :
 Copyright   : Apache licence
 Description : Hello World in C++,
 ============================================================================
 */

#include <iostream>
#include <iomanip>
#include "dvo.h"


static void factorial_example(int);
static int tests_ps(void);
static void string_length_example(std::string str);
static void readme_example(void);

int main(void) {
  tests_ps();
  factorial_example(3);
  string_length_example("str");
  return 0;
}

//-------------------- Readme example --------------------------------
struct Message{
  std::string message;
  int         priority;
};

struct Command{
  int   code;
  void* data;
};

class Interpreter : public dvo::ps::transceiver<Message, Command>{
 public:
  Interpreter() {}
  virtual ~Interpreter() {}
 protected:
  virtual void Process(Message& message) override{
    Command result;
    //Do the interpreter process here
    dvo::ps::transceiver<Message, Command>::Send(result);
  }
};

class Executor : public dvo::ps::receiver<Command>{
 public:
  Executor() {}
  virtual ~Executor() {}
 protected:
  virtual void Process(Command& command) override{
    Command result;
    //Do the executor process here
  }
};

void readme_example(void)
{
  Interpreter interpreter;
  Executor    executor;

  interpreter.Output() >> executor.Input();
}

//-------------------- String length example --------------------------
class StringLength : public dvo::ps::transceiver<const char*, int>{
 public:
  StringLength(){
    process_ = [&](const char* c)->int{
      return c[0] ? process_(c+1) + 1 : 0;
    };
  }
  virtual ~StringLength() {}
 protected:
  virtual void Process(const char*& value) override {
     transceiver<const char*, int>::Send(process_(value));
    }
 private:
  std::function<int(const char*)> process_;
};
void string_length_example(std::string str)
{
  const char *s = str.c_str();
  dvo::ps::SimpleReceiver<int> printer([&](int& value) { std::cout << "String length of "<< str << " is " << value << std::endl; });
  StringLength length;

  length.Output() >> printer.Input();
  s >> length.Input();
}


// ------------------- Factorial example ------------------------------

class Factorial : public dvo::ps::transceiver<int, int>{
 public:
  Factorial(){
    process_ = [&](int val)->int
    {
        return ( val <= 1 ) ? 1 : val * process_(val-1);
    };
  }
  virtual ~Factorial() {}
 protected:
  virtual void Process(int& value) override {
    transceiver<int, int>::Send(process_(value));
  }
 private:
  std::function<int(int)> process_;
};


void factorial_example(int n)
{
  dvo::ps::SimpleReceiver<int> printer([&](int& value) { std::cout << "The factorial for " << n << " is "<< value << std::endl; });
  Factorial factorialer;

  factorialer.Output() >> printer.Input();
  n >> factorialer.Input();
}

//-------------------------------------- TEST ---------------------------------
static void test_ps_receiver(void);
static void test_ps_transmitter(void);
static void test_ps_transceiver(void);
static void test_ps_merger(void);
static void test_ps_splitter(void);

int tests_ps(void) {
  std::cout << std::setw(9) << "Flow type" << std::setw(15) << "Element" << std::setw(10) << "Result" << std::endl;
        test_ps_receiver();
        test_ps_transmitter();
        test_ps_transceiver();
        test_ps_merger();
        test_ps_splitter();
        return 0;
}

void test_ps_receiver(void)
{
        //initiate the result and the value variables
        int something = 0;
        int *value = new int(1), *result = &something;

        //initiate processing elements
        dvo::ps::SimpleReceiver<int> tester([&](int& value) { result = &value; });

        //send the value into the network
        *value >> tester.Input();

        //test the result
        bool test_failed = result != value;
        std::cout << std::setw(9) << "push-flow" << std::setw(15) << "Receiver" << std::setw(10) << (test_failed ? "NOK" : "OK") << std::endl;
}


void test_ps_transmitter(void)
{
        //initiate the result and the value variables
        int something = 0;
        int *value = new int(1), *result = &something;

        //initiate processing elements
        dvo::ps::SimpleTransmitter<int> tester([&]()->int& {return *value; });

        //define connections
        tester.Output() >> [&](int& value) {result = &value; };

        //test the result
        tester.Process();
        bool test_failed = result != value;
        std::cout << std::setw(9) << "push-flow" << std::setw(15) << "Transmitter" << std::setw(10) << (test_failed ? "NOK" : "OK") << std::endl;
}

void test_ps_transceiver(void)
{
        //initiate the result and the value variables
        int something = 0;
        int *value = new int(1), *result = &something;

        //initiate processing elements
        dvo::ps::SimpleTransceiver<int, int> tester([&](int& value)->int& { return ++value; });

        //define connections
        tester.Output() >> [&](int& v) {result = &v; };

        //send the value into the network
        *value >> tester.Input();

        //test the result
        bool test_failed = result != value;
        std::cout << std::setw(9) << "push-flow" << std::setw(15) << "Transceiver" << std::setw(10) << (test_failed ? "NOK" : "OK") << std::endl;
}

void test_ps_merger(void)
{
        //initiate the result and the value variables
        int something = 0;
        int *value = new int(1), *result = &something;
        double value2 = 2.0;

        //initiate processing elements
        dvo::ps::SimpleMerger<int, double, int> tester([&](int& v1, double& v2)->int& { return v1 += v2; });

        //define connections
        tester.Output() >> [&](int& v) {result = &v; };

        //send the value into the network
        *value >> tester.InputX();
        value2 >> tester.InputY();

        //test the result
        bool test_failed = result != value || *result != 3;
        std::cout << std::setw(9) << "push-flow" << std::setw(15) << "Merger" << std::setw(10) << (test_failed ? "NOK" : "OK") << std::endl;
}


void test_ps_splitter(void)
{
        //initiate the result and the value variables
        int source, *trans_int = new int(1), *res_int;
        double *trans_double = new double(1.0), *res_double;

        //initiate processing elements
        dvo::ps::SimpleSplitter<int, double, int>
        tester([&](int& v)->std::tuple<double&, int&>
          {
            return std::tuple<double&, int&>(*trans_double, *trans_int);
          }
        );

        //define connections
        tester.OutputX() >> [&](double& v) {res_double = &v; };
        tester.OutputY() >> [&](int& v) {res_int = &v; };

        //send valu to the network
        source >> tester.Input();

        //test the result
        bool test_failed = res_double != trans_double || res_int != trans_int;
        std::cout << std::setw(9) << "push-flow" << std::setw(15) << "Splitter" << std::setw(10) << (test_failed ? "NOK" : "OK") << std::endl;
}

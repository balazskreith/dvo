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
static int tests_pl(void);
static void string_length_example(std::string str);
static void readme_example(void);
static void functional_examples(const std::vector<int>& integers);
static void functional_examples2(const std::vector<double>& floats);

int main(void) {
  tests_ps();
  tests_pl();
  factorial_example(3);
  string_length_example("str");
  functional_examples(std::vector<int>{1,2,3,4,5,6,7});
  functional_examples2(std::vector<double>{1.1,2.2,3.3,4.4,5.5,6.6,7.7});
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
  dvo::ps::asyncer<Command> asyncer;

  interpreter.Output() >> asyncer.Input();
  asyncer.Output() >> executor.Input();
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


//------- Squeres of integers example ------



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

//-------------------------------------- TEST ---------------------------------
static void test_pl_receiver(void);
static void test_pl_transmitter(void);
static void test_pl_transceiver(void);
static void test_pl_merger(void);
static void test_pl_splitter(void);

int tests_pl(void) {
  std::cout << std::setw(9) << "Flow type" << std::setw(15) << "Element" << std::setw(10) << "Result" << std::endl;
        test_pl_receiver();
        test_pl_transmitter();
        test_pl_transceiver();
        test_pl_merger();
        test_pl_splitter();
        return 0;
}
void test_pl_receiver()
{
        //initiate the result and the value variables
        int source = 1, *result;

        //initiate processing elements
        dvo::pl::SimpleReceiver<int> tester( [&](int& value) {result = &++value; });

        //send the value into the network
        std::function<int&(void)>([&](void)->int& { return source; }) << tester.Input();

        //test the result
        tester.Process();
        bool test_failed = result != &source;
        std::cout << std::setw(9) << "pull-flow" << std::setw(15) << "Receiver" << std::setw(10) << (test_failed ? "NOK" : "OK") << std::endl;
}


void test_pl_transmitter()
{
        //initiate the result and the value variables
        int source = 1, *result = nullptr, *check = &source;

        //initiate processing elements
        dvo::pl::SimpleTransmitter<int> tester([&]()->int& {return source; });

        //define connections
        tester.Output() << result;

        //test the result
        bool test_failed = result != &source;
        std::cout << std::setw(9) << "pull-flow" << std::setw(15) << "Transmitter" << std::setw(10) << (test_failed ? "NOK" : "OK") << std::endl;
}

void test_pl_transceiver()
{
        //initiate the result and the value variables
        int source = 1, *result = nullptr;

        //initiate processing elements
        dvo::pl::SimpleTransceiver<int, int> tester([&](int& value)->int& { return ++value; });

        //define connections
        std::function<int&(void)>([&](void)->int& { return source; }) << tester.Input();

        //request the value from the network
        tester.Output() << result;

        //test the result
        bool test_failed = result != &source;
        std::cout << std::setw(9) << "pull-flow" << std::setw(15) << "Transceiver" << std::setw(10) << (test_failed ? "NOK" : "OK") << std::endl;
}


void test_pl_merger()
{
        //initiate the result and the value variables
        int something = 0;
        int *value = new int(1), *result = &something;
        double value2 = 2.0;

        //initiate processing elements
        dvo::pl::SimpleMerger<int, double, int> tester ( [&](int&, double&)->int& { return *value += value2; } );

        //define connections
        *value << tester.InputX();
        value2 << tester.InputY();

        tester.Output() << result;

        //send the value into the network


        //test the result
        bool test_failed = result != value || *result != 3;
        std::cout << std::setw(9) << "pull-flow" << std::setw(15) << "Merger" << std::setw(10) << (test_failed ? "NOK" : "OK") << std::endl;
}



void test_pl_splitter()
{
        //initiate the result and the value variables
        int source, *trans_int = new int(1), *res_int;
        double *trans_double = new double(1.0), *res_double;

        //initiate processing elements
        dvo::pl::SimpleSplitter<int, double, int> tester;
        tester |= [&](int& v)->double& { return *trans_double += v; };
        tester &= [&](int& v)->int& { return *trans_int += v; };

        //send values to the network
        source << tester.Input();
        tester.OutputX() << res_double;
        tester.OutputY() << res_int;

        //test the result
        bool test_failed = res_double != trans_double || res_int != trans_int;
        std::cout << std::setw(9) << "pull-flow" << std::setw(15) << "Splitter" << std::setw(10) << (test_failed ? "NOK" : "OK") << std::endl;
}


using dvo::ps::SimpleReceiver;
using namespace dvo::fnc;


/*


                                                     .-------.
                                                     | split |
                                                     '-------'
                                                         |
                          .------------------------------'-------------------------------.
                          |                                                              |
                      .-------.                                                          |
                      | shift |                                                          |
                      '-------'                                                          |
                          |                                                              |
                      .-------.                                                      .-------.
                      | split |                                                      | split |
                      '-------'                                                      '-------'
                          |                                                              |
                .---------'----------.                                         .---------'----------.
                |                    |                                         |                    |
           .--------.            .-------.                                .--------.            .-------.
           | length |            | split |                                | length |            | split |
           '--------'            '-------'                                '--------'            '-------'
                |                    |                                         |                    |
                '--------



 */

void functional_examples(const std::vector<int>& integers)
{
  print_all<int>   print_all;
  square<int>      square;
  filter_out<int>  odds([](int value)->bool {return value % 2 == 0;});

  //create a pipeline
  square >> odds >> print_all;

  //start the process
  integers >> square;



//
//  merge<int>          add([](int x,int y)->int {return x+y;});
//  split<int>          split1,split2;
//  std::vector<int>    integers{1,2,3,4,5,6,7,8,9};
//  transform<int>      squere([](int value){ return value * value; });
//  transform<int>      plus_one([](int value){ return value + 1; });
//  foreach<int>        print([](int value) {std::cout << value << " "; });
//  accumulate<int>     sum([](int sum, const int& value)->int {return sum + value;});
//  accumulate<int>     length([](int sum, const int& value)->int {return sum + 1;});
//  SimpleReceiver<int> print_one([](int& value) { std::cout << value << std::endl; });
//  filter_out<int>     evens([](int value)->bool {return value % 2 == 0;});
//  shift_first<int>    shift1, shift2;
//
//
////  dvo::ps::SimpleTransceiver<std::vector<int>,std::vector<int>> squere(
////      [&](std::vector<int>& vector)->std::vector<int>& {
////        std::transform(vector.begin(), vector.end(), vector.begin(), [](int i){ return i*i; });
////        return vector;
////  });
//
//  squere >> print;
//  evens >> sum >> print_one;
//  split1.OutputX() >> squere;
//  split1.OutputY() >> evens;
//
//  std::vector<int>{1,2,3,4,5,6} >> split1;
//
//  split2.OutputX() >> shift1 >> add.InputX();
//  split2.OutputY() >> add.InputY();
//  add.Output() >> print;
//
//  std::vector<int>{1,2,3} >> split2;
}

void functional_examples2(const std::vector<double>& floats)
{
  print_one<double>   print_one;
  square<double>      square;
  filter_out<double>  lowpass_filter([](double value)->bool {return value > 10.;});
  sum<double>         sum;

  //create a pipeline
  square >> lowpass_filter >> sum >> print_one;

  //start the process
  floats >> square;
}




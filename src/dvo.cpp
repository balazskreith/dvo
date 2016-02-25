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

void test_ps_receiver(void);

int main(void) {
	std::cout << "devcpplego" << std::endl; /* prints devcpplego */
	test_ps_receiver();
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
        std::cout << std::setw(9) << "push-flow" << std::setw(12) << "Receiver" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}


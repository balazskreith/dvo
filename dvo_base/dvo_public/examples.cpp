#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <numeric>
#include <future>
#include <functional>

#include "dvo.h"

static void test_dvo();

int main()
{
	test_dvo();
	std::cin.ignore();
	return 0;
}

//pushflow elements
static void test_ps_overall();
static void test_ps_receiver();
static void test_ps_transmitter();
static void test_ps_transceiver();
static void test_ps_merger();
static void test_ps_splitter();
static void test_ps_joiner();
static void test_ps_detacher();
static void test_ps_selector();
static void test_ps_restrictor();
static void test_ps_asyncer();
static void test_ps_mover();
static void test_ps_capacitor();
static void test_ps_async_buffer();

static void test_pl_receiver();
static void test_pl_transmitter();
static void test_pl_transceiver();
static void test_pl_splitter();
static void test_pl_merger();
static void test_pl_joiner();
static void test_pl_detacher();
static void test_pl_selector();
static void test_pl_restrictor();
static void test_pl_asyncer();
static void test_pl_mover();
static void test_pl_capacitor();
static void test_pl_iterator();

static void test_as_accumulator();
static void test_as_relayer();
static void test_as_recycle();


void test_dvo()
{
	test_ps_overall();
	test_ps_receiver();
	test_ps_transmitter();
	test_ps_transceiver();
	test_ps_merger();
	test_ps_splitter();
	test_ps_joiner();
	test_ps_detacher();
	test_ps_selector();
	test_ps_restrictor();
	test_ps_asyncer();
	test_ps_mover();
	test_ps_capacitor();
	test_ps_async_buffer();

	std::cout << std::endl;

	test_pl_receiver();
	test_pl_transmitter();
	test_pl_transceiver();
	test_pl_merger();
	test_pl_splitter();
	test_pl_joiner();
	test_pl_detacher();
	test_pl_selector();
	test_pl_restrictor();
	test_pl_asyncer();
	test_pl_mover();
	test_pl_capacitor();
	test_pl_iterator();

	std::cout << std::endl;

	test_as_accumulator();
	test_as_recycle();
	test_as_relayer();

}


void test_ps_overall()
{
	int result = 0, number = 0;
	dvo::ps::SimpleTransmitter<int> transm = [&]()->int& { return result; };
	dvo::ps::SimpleTransceiver<int, int> trancs = [&](int& value)->int& { return ++value; };
	dvo::ps::SimpleReceiver<int> recv = [&](int& value) {std::cout << value << std::endl; };
	dvo::ps::SimpleMerger<int, int, int> merger = [&](int& x, int& y)->int& { return x += y; };
	dvo::ps::SimpleSplitter<int, int, int> splitter = [&](int& value)->std::tuple<int&, int&> { return std::tuple<int&, int&>(value, value); };
	//dvo::ps::SimpleJoiner<int, int, int> joiner = [&](std::map<int, int*>& values)->int& { return *values[0] += *values[1]; };
	dvo::ps::SimpleDetacher<int, int, int> detacher = [&](int& value)->const std::map<int, int*> {
		std::map<int, int*> result;
		result.emplace(0, &number);
		return result;
	};
	dvo::ps::SimpleSelector<int, int, int> selector = [&](int& value)->std::pair<int, int*> { return std::pair<int, int*>(0, &number); };
	dvo::ps::SimpleRestrictor<int, int, int> restrictor = [&](const int& key, int& value)->int& { return value; };
	dvo::ps::asyncer<int> asyncer;

	transm.Output() >> trancs >> recv.Input();
	transm.Output() >> merger.InputX();
	transm.Output() >> merger.InputY();
	merger.Output() >> recv.Input();
	transm.Output() >> splitter.Input();
	splitter.OutputX() >> recv.Input();
	splitter.OutputY() >> recv.Input();
	//transm.Output() >> joiner(1);
	//joiner.Output() >> recv.Input();
	detacher.Outputs(1) >> recv.Input();
	transm.Output() >> selector.Input();
	selector.Outputs(2) >> recv;
	transm.Output() >> restrictor.Inputs(2);
	restrictor.Output() >> recv;
	//trancs.Output() >> std::function<void(int&)>([&](int& v){ std::cout << v;});
	transm.Output() >> asyncer >> trancs >> recv;
	std::cout << std::setw(9) << "Overall" << std::setw(12) << "push-flow" << std::setw(5) << "OK" << std::endl;
}


void test_ps_receiver()
{
	//initiate the result and the value variables
	int something = 0;
	int *value = new int(1), *result = &something;

	//initiate processing elements
	dvo::ps::SimpleReceiver<int> tester = [&](int& value) {result = &value; };

	//send the value into the network
	*value >> tester.Input();

	//test the result
	bool test_failed = result != value;
	std::cout << std::setw(9) << "push-flow" << std::setw(12) << "Receiver" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}


void test_ps_transmitter()
{
	//initiate the result and the value variables
	int something = 0;
	int *value = new int(1), *result = &something;

	//initiate processing elements
	dvo::ps::SimpleTransmitter<int> tester = [&]()->int& {return *value; };

	//define connections
	tester.Output() >> [&](int& value) {result = &value; };

	//test the result
	tester.Process();
	bool test_failed = result != value;
	std::cout << std::setw(9) << "push-flow" << std::setw(12) << "Transmitter" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}

void test_ps_transceiver()
{
	//initiate the result and the value variables
	int something = 0;
	int *value = new int(1), *result = &something;

	//initiate processing elements
	dvo::ps::SimpleTransceiver<int, int> tester = [&](int& value)->int& { return ++value; };

	//define connections
	tester.Output() >> [&](int& v) {result = &v; };

	//send the value into the network
	*value >> tester.Input();

	//test the result
	bool test_failed = result != value;
	std::cout << std::setw(9) << "push-flow" << std::setw(12) << "Transceiver" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}


void test_ps_merger()
{
	//initiate the result and the value variables
	int something = 0;
	int *value = new int(1), *result = &something;
	double value2 = 2.0;

	//initiate processing elements
	dvo::ps::SimpleMerger<int, double, int> tester = [&](int& v1, double& v2)->int& { return v1 += v2; };

	//define connections
	tester.Output() >> [&](int& v) {result = &v; };

	//send the value into the network
	*value >> tester.InputX();
	value2 >> tester.InputY();

	//test the result
	bool test_failed = result != value || *result != 3;
	std::cout << std::setw(9) << "push-flow" << std::setw(12) << "Merger" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}



void test_ps_splitter()
{
	//initiate the result and the value variables
	int source, *trans_int = new int(1), *res_int;
	double *trans_double = new double(1.0), *res_double;

	//initiate processing elements
	dvo::ps::SimpleSplitter<int, double, int> tester = [&](int& v)->std::tuple<double&, int&>
	{ return std::tuple<double&, int&>(*trans_double, *trans_int); };

	//define connections
	tester.OutputX() >> [&](double& v) {res_double = &v; };
	tester.OutputY() >> [&](int& v) {res_int = &v; };

	//send valu to the network
	source >> tester.Input();

	//test the result
	bool test_failed = res_double != trans_double || res_int != trans_int;
	std::cout << std::setw(9) << "push-flow" << std::setw(12) << "Splitter" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}

void test_ps_joiner()
{
	/*
	//initiate the result and the value variables
	std::promise<void> ready[2];
	int *result, *sum = new int(0), actual = 0,
		*v11 = new int(1), *v21 = new int(1), *v12 = new int(2), *v22 = new int(2);
	dvo::ps::plug<int> plug_1, plug_2;

	//initiate processing elements
	dvo::ps::SimpleJoiner<int, int, int> tester = [&](const std::map<int, int*>& map)->int& { return *sum = *map.at(1) + *map.at(2); };

	//define connections
	plug_1 >> tester.Inputs(1);
	plug_2 >> tester.Inputs(2);
	tester.Output() >> [&](int& fwd) { result = &fwd; ready[actual++].set_value(); };;

	//send the value into the network
	auto fd1 = std::async(std::launch::async, [&]() { *v11 >> plug_1; });
	fd1.wait();
	auto fd2 = std::async(std::launch::async, [&]() { *v12 >> plug_1; });
	auto fd3 = std::async(std::launch::async, [&]() { *v21 >> plug_2; });
	fd3.wait();
	auto fd4 = std::async(std::launch::async, [&]() { *v22 >> plug_2; });

	//test the result  
	ready[0].get_future().wait();
	bool test_failed = *sum != 2 || result != sum;
	ready[1].get_future().wait();
	test_failed |= *sum != 4;
	std::cout << std::setw(9) << "push-flow" << std::setw(12) << "Joiner" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
	/**/
}


void test_ps_detacher()
{
	//initiate the result and the value variables
	std::promise<void> signal1, signal2;
	int *source = new int(1), *v1, *v2;

	//initiate processing elements
	dvo::ps::SimpleDetacher<int, int, int> tester = [&](int& v)->const std::map < int, int* >
	{
		std::map<int, int*> map;
		++v;
		map.emplace(1, &v);
		map.emplace(2, &v);
		return map;
	};

	//define connections
	tester.Outputs(1) >> [&](int& v) {v1 = &v; signal1.set_value(); };
	tester.Outputs(2) >> [&](int& v) {v2 = &v; signal2.set_value(); };

	//send the value into the network
	*source >> tester.Input();

	//test the result
	signal1.get_future().wait(); signal2.get_future().wait();
	bool test_failed = v1 != source || v2 != source;
	std::cout << std::setw(9) << "push-flow" << std::setw(12) << "Detacher" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}


void test_ps_selector()
{
	//initiate the result and the value variables
	int *source = new int(1), *result1 = nullptr, *result2 = nullptr;

	//initiate processing elements
	dvo::ps::SimpleSelector<int, int, int> tester = [&](int& v)->std::pair<int, int*>
	{ return std::pair<int, int*>(2, source); };

	//define connections
	tester(1) >> [&](int& v) {result1 = &v; };
	tester(2) >> [&](int& v) {result2 = &v; };

	//send the value into the network
	*source >> tester.Input();

	//test the result
	bool test_failed = result1 == source || result2 != source;
	std::cout << std::setw(9) << "push-flow" << std::setw(12) << "Selector" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}


void test_ps_restrictor()
{
	//initiate the result and the value variables
	int *source1 = new int(1), *source2 = new int(2), *result = nullptr;
	int *stored;

	//initiate processing elements
	dvo::ps::SimpleRestrictor<int, int, int> tester = [&](const int& k, int& v)->int& { return (stored = k == 1 ? &v : stored), *stored; };

	//define connections
	tester.Output() >> [&](int& v) { result = &v; };

	//send the value into the network
	*source1 >> tester(1);
	*source2 >> tester(2);

	//test the result
	bool test_failed = result != source1;
	std::cout << std::setw(9) << "push-flow" << std::setw(12) << "Regulator" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}


void test_ps_asyncer()
{
	//initiate the result and the value variables
	int *source = new int(1), *result = nullptr;

	//initiate processing elements
	dvo::ps::asyncer<int> tester;

	//define connections
	tester.Output() >> [&](int& fwd) { result = &fwd; };

	//send the value into the network
	*source >> tester.Input();

	//test the result
	tester.Wait();
	bool test_failed = result != source;
	std::cout << std::setw(9) << "push-flow" << std::setw(12) << "Asyncer" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}


void test_ps_mover()
{
	//initiate the result and the value variables
	int reference = 2;
	int* value = &reference;
	int* result = &reference;

	//initiate processing elements
	dvo::ps::mover<int> tester;

	//define connections
	tester.Output() >> [&](int& v) { result = &v; };

	//send the value to the network
	*value >> tester.Input();

	//test the result
	bool test_failed = *result != 2 || result == value;
	std::cout << std::setw(9) << "push-flow" << std::setw(12) << "Mover" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}

void test_ps_capacitor()
{
	//initiate the result and the value variables
	int something = 0;
	int *value = new int(1), *result = &something;

	//initiate processing elements
	dvo::ps::capacitor<int> tester(2);

	//define connections
	tester.Output() >> [&](int& v) {result = &v; };

	//send the value into the network
	*value >> tester.Input();
	bool test_failed = result == value;
	*value >> tester.Input();
	test_failed |= result != value;
	std::cout << std::setw(9) << "push-flow" << std::setw(12) << "Capacitor" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}


void test_ps_async_buffer()
{
	//initiate the result and the value variables
	int something = 0;
	int *value = new int(1), *result = &something;
	std::promise<int> promise;
	//initiate processing elements
	dvo::ps::async_buffer<int> tester;

	//define connections
	tester.Output() >> [&](int& v) {result = &v; promise.set_value(v); };

	//send the value into the network
	*value >> tester.Input();
	promise.get_future().wait();
	bool test_failed = result != value;
	std::cout << std::setw(9) << "push-flow" << std::setw(12) << "AsyncBuffer" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}

/*
void operator<<(const std::function<int&(void)>& target, dvo::pl::plug<int>& source)
{
source.Connect(target);
}
*/

void test_pl_receiver()
{
	//initiate the result and the value variables
	int source = 1, *result;

	//initiate processing elements
	dvo::pl::SimpleReceiver<int> tester = [&](int& value) {result = &++value; };

	//send the value into the network
	std::function<int&(void)>([&](void)->int& { return source; }) << tester.Input();

	//test the result
	tester.Process();
	bool test_failed = result != &source;
	std::cout << std::setw(9) << "pull-flow" << std::setw(12) << "Receiver" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}


void test_pl_transmitter()
{
	//initiate the result and the value variables
	int source = 1, *result = nullptr, *check = &source;

	//initiate processing elements
	dvo::pl::SimpleTransmitter<int> tester = [&]()->int& {return source; };

	//define connections
	tester.Output() << result;

	//test the result
	bool test_failed = result != &source;
	std::cout << std::setw(9) << "pull-flow" << std::setw(12) << "Transmitter" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}

void test_pl_transceiver()
{
	//initiate the result and the value variables
	int source = 1, *result = nullptr;

	//initiate processing elements
	dvo::pl::SimpleTransceiver<int, int> tester = [&](int& value)->int& { return ++value; };

	//define connections
	std::function<int&(void)>([&](void)->int& { return source; }) << tester.Input();

	//request the value from the network
	tester.Output() << result;

	//test the result
	bool test_failed = result != &source;
	std::cout << std::setw(9) << "pull-flow" << std::setw(12) << "Transceiver" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}


void test_pl_merger()
{
	//initiate the result and the value variables
	int something = 0;
	int *value = new int(1), *result = &something;
	double value2 = 2.0;

	//initiate processing elements
	dvo::pl::SimpleMerger<int, double, int> tester = [&](int&, double&)->int& { return *value += value2; };

	//define connections
	*value << tester.InputX();
	value2 << tester.InputY();

	tester.Output() << result;

	//send the value into the network


	//test the result
	bool test_failed = result != value || *result != 3;
	std::cout << std::setw(9) << "pull-flow" << std::setw(12) << "Merger" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
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
	std::cout << std::setw(9) << "pull-flow" << std::setw(12) << "Splitter" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}


void test_pl_joiner()
{
	//initiate the result and the value variables
	int src1, src2, *result;
	//initiate processing elements
	dvo::pl::SimpleJoiner<int, int, int> tester = [&](std::map<int, int*>& map)->int& { return *map.at(1) += *map.at(2); };

	//define connections
	src1 << tester(1);
	src2 << tester(2);

	//send the value into the network
	tester.Output() << result;

	//test the result  
	bool test_failed = result != &src1;
	std::cout << std::setw(9) << "pull-flow" << std::setw(12) << "Joiner" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}


void test_pl_detacher()
{
	int src, *dst;
	dvo::pl::SimpleDetacher<int, int, int> tester = [&](const int& k, int& v)->int& { return v; };

	src << tester.Input();
	tester.Outputs(1) << dst;

	//test the result  
	bool test_failed = dst != &src;
	std::cout << std::setw(9) << "pull-flow" << std::setw(12) << "Detacher" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}


void test_pl_selector()
{
	//initiate the result and the value variables
	int *result;
	int src1 = 1, src2 = 2, key = 2;

	//initiate processing elements
	dvo::pl::SimpleSelector<int, int, int> tester = [&](int& v)->int& { return ++v; };

	//define connections
	std::function<int&(void)>([&]()->int& { return src1; }) << tester(1);
	std::function<int&(void)>([&]()->int& { return src2; }) << tester(2);

	//send the value into the network
	key << tester.Key();
	tester.Output() << result;

	//test the result
	bool test_failed = result != &src2;
	std::cout << std::setw(9) << "pull-flow" << std::setw(12) << "Selector" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}


void test_pl_restrictor()
{
	//initiate the result and the value variables
	int *dst1, *dst2;
	int value = 1, null = 0;

	//initiate processing elements
	dvo::pl::SimpleRestrictor<int, int, int> tester = [&](int k, int& v)->int& { return ++v; };

	//define connections
	std::function<int&(void)>([&]()->int& { return value; }) << tester.Input();

	//send the value into the network
	tester(1) << dst1;
	tester(2) << dst2;

	//test the result
	bool test_failed = dst1 != &value || dst2 != &value;
	std::cout << std::setw(9) << "pull-flow" << std::setw(12) << "Restrictor" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}

void test_pl_asyncer()
{
	//initiate the result and the value variables
	int value = 2;
	int *input = &value, *output;

	//initiate processing elements
	dvo::pl::asyncer<int> tester;

	//define connections
	std::function<int&(void)>([&]()->int& { return value; }) << tester.Input();

	//request a value from the network
	tester.Output() << output;
	//test the result
	bool test_failed = input != output;
	std::cout << std::setw(9) << "pull-flow" << std::setw(12) << "Asyncer" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}

void test_pl_mover()
{
	//initiate the result and the value variables
	int reference = 2;
	int* value = &reference;
	int* result = &reference;

	//initiate processing elements
	dvo::pl::mover<int> tester;

	//define connections
	std::function<int&(void)>([&]()->int& { return *value; }) << tester.Input();

	//send the value to the network
	tester.Output() << result;

	//test the result
	bool test_failed = *result != 2 || result == value;
	std::cout << std::setw(9) << "push-flow" << std::setw(12) << "Mover" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}

void test_pl_capacitor()
{
	//initiate the result and the value variables
	int sources[2], i = 0;
	int *results[2], *result = nullptr;

	//initiate processing elements
	dvo::pl::capacitor<int> tester(2);

	//define connections
	std::function<int&(void)>([&]()->int& { return sources[i++]; }) << tester.Input();

	//send the value into the network
	tester.Output() << results[0];
	tester.Output() << results[1];
	bool test_failed = results[0] != &sources[0] || results[1] != &sources[1];
	std::cout << std::setw(9) << "push-flow" << std::setw(12) << "Capacitor" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}

void test_pl_iterator()
{
	//initiate the result and the value variables
	std::vector<int*> src;
	int *r1, *r2, v1, v2;
	src.push_back(&v1);
	src.push_back(&v2);

	//initiate processing elements
	dvo::pl::iterator<std::vector<int*>, int*> tester;

	//define connections
	src << tester.Input();
	r2 = &v1;
	tester.Output() << r1;
	tester.Output() << r2;
	bool test_failed = r1 != &v1 || r2 != &v2;
	std::cout << std::setw(9) << "pull-flow" << std::setw(12) << "Iterator" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}


void test_as_accumulator()
{
	//initiate the result and the value variables
	int *result, source = 2;

	//initiate processing elements
	dvo::as::accumulator<int> tester;

	//send the value into the network
	source >> tester.Input();
	tester.Output() << result;

	//test the result
	bool test_failed = result != &source;
	std::cout << std::setw(12) << "hybrid-flow" << std::setw(15) << "Accumulator" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}


void test_as_relayer()
{
	//initiate the result and the value variables
	int result = 0;
	int value = 2;
	int signal;

	//initiate processing elements
	dvo::as::relayer<int> tester;

	//define connections
	std::function<int&(void)>([&]()->int& { return value; }) << tester.Input();
	tester.Output() >> [&](int& v) { result = value; };

	//send the value into the network
	tester.Process(signal);

	//test the result
	bool test_failed = result != 2;
	std::cout << std::setw(12) << "hybrid-flow " << std::setw(15) << "Relayer" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}
void test_as_recycle()
{
	//initiate the result and the value variables

	//initiate processing elements
	dvo::as::recycle<int> tester;

	//define connections

	//send the value into the network
	int *value, *result;
	tester.Output() << value;
	*value >> tester.Input();
	*value = 2;
	tester.Output() << result;

	//test the result
	bool test_failed = value != result;
	std::cout << std::setw(12) << "hybrid-flow " << std::setw(15) << "Recycle" << std::setw(5) << (test_failed ? "NOK" : "OK") << std::endl;
}


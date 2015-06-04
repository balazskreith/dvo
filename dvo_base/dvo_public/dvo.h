#pragma once
#include <map>
#include <thread>
#include <future>
#include <memory>
#include <queue>
#include <functional>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <stack>
#include <iostream>
#include <cstdarg>
#include <list>

#define DISALLOW_COPY(TypeName) \
   TypeName(const TypeName&)

#define DISALLOW_MOVE(TypeName) \
  void operator=(const TypeName&&)

#define DISALLOW_ASSIGN(TypeName) \
  void operator=(const TypeName&)

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  DISALLOW_COPY(TypeName);                 \
  DISALLOW_ASSIGN(TypeName)


#define DISALLOW_MOVE_AND_ASSIGN(TypeName) \
  DISALLOW_MOVE(TypeName);                 \
  DISALLOW_ASSIGN(TypeName)

#define DISALLOW_COPY_AND_MOVE_AND_ASSIGN(TypeName) \
  DISALLOW_COPY(TypeName);                          \
  DISALLOW_MOVE(TypeName);                          \
  DISALLOW_ASSIGN(TypeName)

#define DEBUGPRINT(ARG) std::cout << ARG << std::endl

template<typename T>
void operator>>(T& value, const std::function<void(T&)>& target)
{
	target(value);
}


template<typename T>
void operator>>(T* value, const std::function<void(T&)>& target)
{
	target(*value);
}


namespace dvo
{
	namespace ps
	{
		template<typename T>
		class isimplex {
		public:
			virtual void Transmit(T*) = 0;
			virtual void Transmit(T&) = 0;
			virtual void Transmit(T&&) = 0;
		};

		template<typename T>
		class plug : public isimplex<T> {
		public:
			plug() {}
			plug(plug<T>& peer) : target_(peer.target_) {};
			plug(plug<T>&& peer) : target_(peer.target_) {};
			virtual ~plug() {}
			inline plug<T>& operator>>(std::function<void(T&)> target) { target_ = target; return *this; }
			virtual void Transmit(T* data) { target_(*data); }
			virtual void Transmit(T& data) { target_(data); }
			virtual void Transmit(T&& data) { target_(data); }
			inline void Connect(const std::function<void(T&)>& target) { target_ = target; }
		private:
			std::function<void(T&)> target_;
		};


		template<typename TKey, typename TValue>
		class socket {
		public:
			socket() {}
			socket(const TKey& key, const std::function<void(const TKey& key, TValue&)>& target) : key_(key), target_(target) {}
			socket(socket<TKey, TValue>& peer) : target_(peer.target_), key_(peer.key_) {}
			socket(socket<TKey, TValue>&& peer) : target_(peer.target_), key_(peer.key_) {}
			std::function<void(TValue&)> operator()() { return std::bind(&socket<TKey, TValue>::Process, this, std::placeholders::_1); }
			virtual ~socket() {}
		protected:
			void Process(TValue& value) { target_(key_, value); }
		private:
			TKey key_;
			std::function<void(const TKey& key, TValue&)> target_;
		};

		//----------------------------------------------------------------------------------
		template<typename T>
		plug<T>& operator>>(plug<T>& source, const std::function<void(T&)>& target)
		{
			source.Connect(target);
			return source;
		}

		//----------------------------------------------------------------------------------
		template<typename T> class receiver {
		public:
			receiver();
			receiver(receiver<T>& peer) {}
			receiver(receiver<T>&& peer) {}
			virtual ~receiver();
			std::function<void(T&)> Input() { return std::bind(&receiver<T>::Process, this, std::placeholders::_1); }
		protected:
			virtual void Process(T& value) = 0;
		};

		template<typename T>
		receiver<T>::receiver()
		{

		}

		template<typename T>
		receiver<T>::~receiver()
		{

		}
		//--------------------------------------------------------------------------------
		template<typename T>
		receiver<T>& operator>>(plug<T>& source, receiver<T>& target)
		{
			source >> target.Input();
			return target;
		}
		//---------------------------------------------------------------------------------
		template<typename T>
		class SimpleReceiver : public receiver<T>
		{
		public:
			SimpleReceiver(std::function<void(T&)> process) { process_ = process; }
			SimpleReceiver(SimpleReceiver<T>& peer) : process_(peer.process_) { }
			SimpleReceiver(SimpleReceiver<T>&& peer) : process_(peer.process_) { }
			virtual ~SimpleReceiver() {}
			SimpleReceiver& operator=(const std::function<void(T&)>& process) { process_ = process; return *this; }
		protected:
			virtual void Process(T& value) override { process_(value); }
		private:
			std::function<void(T&)> process_;
		};


		//---------------------------------------------------------------------------------

		template<typename T> class transmitter {
		public:
			transmitter();
			transmitter(transmitter<T>& peer) {}
			transmitter(transmitter<T>&& peer) {}
			virtual ~transmitter();
			plug<T>& Output() { return output_; }
			virtual void Process() = 0;
		protected:
			inline void Send(T& value) { output_.Transmit(value); }
		private:
			plug<T> output_;
		};

		template<typename T>
		transmitter<T>::transmitter()
		{

		}

		template<typename T>
		transmitter<T>::~transmitter()
		{

		}
		//--------------------------------------------------------------------------------
		template<typename T>
		transmitter<T>& operator>>(transmitter<T>& source, const std::function<void(T&)>& target)
		{
			source.Output() >> target;
			return source;
		}
		//---------------------------------------------------------------------------------
		template<typename T>
		class SimpleTransmitter : public transmitter<T>
		{
		public:
			SimpleTransmitter(std::function<T&(void)> process) { process_ = process; }
			SimpleTransmitter(SimpleTransmitter<T>& peer) : process_(peer.process_) { }
			SimpleTransmitter(SimpleTransmitter<T>&& peer) : process_(peer.process_) { }
			virtual ~SimpleTransmitter() { }
			SimpleTransmitter& operator=(const std::function<T&(void)>& process) { process_ = process; return *this; }
			virtual void Process() override { Send(process_()); }
		private:
			std::function<T&(void)> process_;
		};
		//---------------------------------------------------------------------------------

		template<typename TInput, typename TOutput> class transceiver {
		public:
			transceiver();
			virtual ~transceiver();
			std::function<void(TInput&)> Input() { return std::bind(&transceiver<TInput, TOutput>::Process, this, std::placeholders::_1); }
			plug<TOutput>& Output() { return output_; }
		protected:
			virtual void Process(TInput& value) = 0;
			inline void Send(TOutput& value) { output_.Transmit(value); }
			inline void Send(TOutput&& value) { output_.Transmit(value); }
		private:
			plug<TOutput> output_;
		};

		template<typename TInput, typename TOutput>
		transceiver<TInput, TOutput>::transceiver()
		{

		}

		template<typename TInput, typename TOutput>
		transceiver<TInput, TOutput>::~transceiver()
		{

		}
		//--------------------------------------------------------------------------------
		template<typename T>
		plug<T>& operator>>(plug<T>& source, transceiver<T, T>& target)
		{
			source >> target.Input();
			return target.Output();
		}
		//---------------------------------------------------------------------------------
		template<typename TInput, typename TOutput>
		class SimpleTransceiver : public transceiver<TInput, TOutput>
		{
		public:
			SimpleTransceiver& operator=(std::function<TOutput&(TInput&)> process) { process_ = process; return *this; }
			SimpleTransceiver(std::function<TOutput&(TInput&)> process) { process_ = process; }
			SimpleTransceiver(SimpleTransceiver<TInput, TOutput>& peer) : process_(peer.process_) { }
			SimpleTransceiver(SimpleTransceiver<TInput, TOutput>&& peer) : process_(peer.process_) { }
			virtual ~SimpleTransceiver() {}
		protected:
			virtual void Process(TInput& value) override { Send(process_(value)); }
		private:
			std::function<TOutput&(TInput&)> process_;
		};

		//--------------------------------------------------------------------------------
		template<typename TInX, typename TInY, typename TOutput>
		class merger {
		public:
			merger() : x_(nullptr), y_(nullptr) {};
			merger(merger<TInX, TInY, TOutput>& peer) : x_(nullptr), y_(nullptr) {}
			merger(merger<TInX, TInY, TOutput>&& peer) : x_(nullptr), y_(nullptr) {}
			virtual ~merger() {}
			inline std::function<void(TInX&)> InputX() { return std::bind(&merger<TInX, TInY, TOutput>::ReceiveX, this, std::placeholders::_1); }
			inline std::function<void(TInY&)> InputY() { return std::bind(&merger<TInX, TInY, TOutput>::ReceiveY, this, std::placeholders::_1); }
			inline plug<TOutput>& Output() { return output_; }
			inline void Send(TOutput& value) { output_.Transmit(value); }
			inline void Send(TOutput&& value) { output_.Transmit(value); }
		protected:
			virtual void ReceiveX(TInX& x);
			virtual void ReceiveY(TInY& y);
			virtual void Process(TInX& x, TInY& y) = 0;
		private:
			TInX* x_;
			TInY* y_;
			plug<TOutput> output_;
		};


		template<typename TInX, typename TInY, typename TOutput>
		void merger<TInX, TInY, TOutput>::ReceiveX(TInX& x)
		{
			if (y_ == nullptr) {
				x_ = &x;
				return;
			}
			TInY& _y_ = *y_; y_ = nullptr;
			TInX* _x_ = x_;
			if (x_ != nullptr) {
				x_ = &x;
			}
			else {
				_x_ = &x;
			}
			Process(*_x_, _y_);
		}

		template<typename TInX, typename TInY, typename TOutput>
		void merger<TInX, TInY, TOutput>::ReceiveY(TInY& y)
		{
			if (x_ == nullptr) {
				y_ = &y;
				return;
			}
			TInX& _x_ = *x_; x_ = nullptr;
			TInY* _y_ = y_;
			if (y_ != nullptr) {
				y_ = &y;
			}
			else {
				_y_ = &y;
			}
			Process(_x_, *_y_);
		}

		//------------------------------------------------------------------------
		template<typename TInX, typename TInY, typename TOutput>
		class SimpleMerger : public merger<TInX, TInY, TOutput> {
		public:
			SimpleMerger& operator=(const std::function<TOutput&(TInX&, TInY&)>& process) { process_ = process; return *this; }
			SimpleMerger(const std::function<TOutput&(TInX&, TInY&)>& process) { process_ = process; }
			SimpleMerger(SimpleMerger<TInX, TInY, TOutput>& peer) : process_(peer.process_) { }
			SimpleMerger(SimpleMerger<TInX, TInY, TOutput>&& peer) : process_(peer.process_) { }
			virtual ~SimpleMerger() {}
		protected:
			virtual void Process(TInX& x, TInY& y) override { Send(process_(x, y)); }
		private:
			std::function<TOutput&(TInX&, TInY&)> process_;
		};

		//------------------------------------------------------------------------
		template<typename TInput, typename TOutX, typename TOutY>
		class splitter {
		public:
			splitter() {}
			splitter(splitter<TInput, TOutX, TOutY>& peer) : outx_(peer.outx_), outy_(peer.outy_) {}
			splitter(splitter<TInput, TOutX, TOutY>&& peer) : outx_(peer.outx_), outy_(peer.outy_) {}
			virtual ~splitter() {}
			inline std::function<void(TInput&)> Input() { return std::bind(&splitter<TInput, TOutX, TOutY>::Process, this, std::placeholders::_1); }
			inline plug<TOutX>& OutputX() { return outx_; }
			inline plug<TOutY>& OutputY() { return outy_; }
		protected:
			inline void SendX(TOutX& value) { outx_.Transmit(value); }
			inline void SendX(TOutX&& value) { outx_.Transmit(value); }
			inline void SendY(TOutY& value) { outy_.Transmit(value); }
			inline void SendY(TOutY&& value) { outy_.Transmit(value); }
			virtual void Process(TInput& value) = 0;
		private:
			plug<TOutX> outx_;
			plug<TOutY> outy_;
		};
		//------------------------------------------------------------------------
		template<typename TInput, typename TOutX, typename TOutY>
		class SimpleSplitter : public splitter<TInput, TOutX, TOutY> {
		public:
			SimpleSplitter& operator=(const std::function<std::tuple<TOutX&, TOutY&>(TInput&)>& process) { process_ = process; return *this; }
			SimpleSplitter(const std::function<std::tuple<TOutX&, TOutY&>(TInput&)>& process) { process_ = process; }
			SimpleSplitter(SimpleSplitter<TInput, TOutX, TOutY>& peer) : process_(peer.process_) { }
			SimpleSplitter(SimpleSplitter<TInput, TOutX, TOutY>&& peer) : process_(peer.process_) { }
			virtual ~SimpleSplitter() {}
		protected:
			virtual void Process(TInput& value) override
			{
				std::tuple<TOutX&, TOutY&> values = process_(value); SendX(std::get<0>(values)); SendY(std::get<1>(values));
			}
		private:
			std::function<std::tuple<TOutX&, TOutY&>(TInput&)> process_;
		};

		//-----------------------------------------------------------------
		/*
		template<typename TKey, typename TInput, typename TOutput> class joiner {
		public:
			joiner();
			virtual ~joiner();
			std::function<void(TInput&)> Inputs(const TKey& key);
			void Remove(const TKey& key);
			plug<TOutput>& Output() { return output_; }
		protected:
			virtual void Process(std::map<TKey, TInput*>& values) = 0;
			inline void Send(TOutput& value) { output_.Transmit(value); }
			virtual void Receive(const TKey& key, TInput& value);
			std::map<TKey, TInput*>& GetAll();
		private:
			plug<TOutput> output_;
			std::map<TKey, socket<TKey, TInput>> sockets_;
			std::map<TKey, std::unique_ptr<std::condition_variable>> cvs_;
			std::map<TKey, std::atomic_int> waits_;
			std::map<TKey, TInput*> values_;
			std::size_t arrived_;
			std::size_t limit_;
			std::mutex mutex_;
		};

		template<typename TKey, typename TInput, typename TOutput>
		joiner<TKey, TInput, TOutput>::joiner() : limit_(0), arrived_(0)
		{

		}

		template<typename TKey, typename TInput, typename TOutput>
		std::function<void(TInput&)> joiner<TKey, TInput, TOutput>::Inputs(const TKey& key)
		{
			std::map<TKey, socket<TKey, TInput>>::iterator it = sockets_.find(key);
			if (it != sockets_.end()) {
				return (it->second)();
			}
			++limit_;
			sockets_.emplace(key, socket<TKey, TInput>(key, std::bind(&joiner<TKey, TInput, TOutput>::Receive, this, std::placeholders::_1, std::placeholders::_2)));
			cvs_.emplace(key, std::unique_ptr<std::condition_variable>(new std::condition_variable()));
			waits_.emplace(key, std::atomic_int());
			return (sockets_.at(key))();
		}

		template<typename TKey, typename TInput, typename TOutput>
		void joiner<TKey, TInput, TOutput>::Remove(const TKey& key)
		{
			map<TKey, socket<TKey, TInput>>::iterator it = sockets_.find(key);
			if (it == sockets_.end()) {
				return;
			}
			--limit_;
			sockets_.erase(it);
			cvs_.erase(key);
			waits_.erase(key);
		}

		template<typename TKey, typename TInput, typename TOutput>
		void joiner<TKey, TInput, TOutput>::Receive(const TKey& key, TInput& value)
		{
			std::unique_lock<std::mutex> mutex(mutex_);
			std::map<TKey, TInput*>::iterator it = values_.find(key);
			if (it != values_.end()) {
				++waits_.at(key);
				cvs_.at(key)->wait(mutex);
			}
			values_.emplace(key, &value);
			if (++arrived_ < limit_) {
				return;
			}
			arrived_ = 0;
			std::map<TKey, TInput*> values;
			values.swap(values_);
			Process(values);

			for (std::pair<const TKey, std::atomic_int>& pair : waits_) {
				if (pair.second < 1) {
					continue;
				}
				cvs_.at(pair.first)->notify_one();
				--pair.second;
			}

		}

		template<typename TKey, typename TInput, typename TOutput>
		joiner<TKey, TInput, TOutput>::~joiner()
		{

		}
		//---------------------------------------------------------------------------------
		template<typename TKey, typename TInput, typename TOutput>
		class SimpleJoiner : public joiner< TKey, TInput, TOutput > {
		public:
			SimpleJoiner() {}
			SimpleJoiner(const std::function<TOutput&(std::map<TKey, TInput*>&)>& process) { process_ = process; }
			SimpleJoiner(SimpleJoiner<TKey, TInput, TOutput>& peer) : joiner<TKey, TInput, TOutput>::joiner(peer), process_(peer.process_) { }
			SimpleJoiner(SimpleJoiner<TKey, TInput, TOutput>&& peer) : joiner<TKey, TInput, TOutput>::joiner(peer), process_(peer.process_) { }
			virtual ~SimpleJoiner() {}
			SimpleJoiner& operator=(const std::function<TOutput&(std::map<TKey, TInput*>&)>& process) { process_ = process; return *this; }
			std::function<void(TInput&)> operator()(const TKey& key) { return Inputs(key); }
		protected:
			virtual void Process(std::map<TKey, TInput*>& values) override { Send(process_(values)); }
		private:
			std::function<TOutput&(std::map<TKey, TInput*>&)> process_;
		};
		/**/

		//---------------------------------------------------------------------------------
		template<typename TKey, typename TInput, typename TOutput>
		class detacher {
		public:
			detacher();
			detacher(detacher<TKey, TInput, TOutput>& peer) : outputs_(peer.outputs_) {}
			detacher(detacher<TKey, TInput, TOutput>&& peer) : outputs_(peer.outputs_) {}
			virtual ~detacher();
			std::function<void(TInput&)> Input() { return std::bind(&detacher<TKey, TInput, TOutput>::Process, this, std::placeholders::_1); }
			plug<TOutput>& Outputs(const TKey& key);
			void Remove(const TKey& key);
		protected:
			inline std::future<void> Send(const TKey& key, TOutput& value);
			virtual void Process(TInput& value) = 0;
		private:
			std::map<TKey, plug<TOutput>> outputs_;
		};

		template<typename TKey, typename TInput, typename TOutput>
		detacher<TKey, TInput, TOutput>::detacher()
		{

		}

		template<typename TKey, typename TInput, typename TOutput>
		plug<TOutput>& detacher<TKey, TInput, TOutput>::Outputs(const TKey& key)
		{
			std::map<TKey, plug<TOutput>>::iterator it = outputs_.find(key);
			if (it != outputs_.end()) {
				return it->second;
			}
			outputs_.emplace(key, plug<TOutput>());
			return outputs_.at(key);
		}
		template<typename TKey, typename TInput, typename TOutput>
		void detacher<TKey, TInput, TOutput>::Remove(const TKey& key)
		{
			outputs_.erase(key);
		}

		template<typename TKey, typename TInput, typename TOutput>
		std::future<void> detacher<TKey, TInput, TOutput>::Send(const TKey& key, TOutput& value)
		{
			return std::async(std::launch::async, [&](TKey fwdK, TOutput& fwdV) { outputs_[fwdK].Transmit(fwdV); }, key, std::ref(value));
		}

		template<typename TKey, typename TInput, typename TOutput>
		detacher<TKey, TInput, TOutput>::~detacher()
		{

		}
		//---------------------------------------------------------------------------------
		template<typename TKey, typename TInput, typename TOutput>
		class SimpleDetacher : public detacher < TKey, TInput, TOutput > {
		public:
			SimpleDetacher() {}
			SimpleDetacher(SimpleDetacher<TKey, TInput, TOutput>& peer) : detacher<TKey, TInput, TOutput>::detacher(peer), process_(peer.process_) { }
			SimpleDetacher(SimpleDetacher<TKey, TInput, TOutput>&& peer) : detacher<TKey, TInput, TOutput>::detacher(peer), process_(peer.process_) { }
			SimpleDetacher(const std::function<const std::map<TKey, TOutput*>(TInput&)> process) : process_(process) { }
			SimpleDetacher& operator=(const std::function<const std::map<TKey, TOutput*>(TInput&)> process) { process_ = process; return *this; }
			virtual ~SimpleDetacher() {}
		protected:
			virtual void Process(TInput& value) override {
				for (const std::pair<const TKey, TOutput*>& pair : process_(value)) {
					Send(pair.first, *pair.second);
				}
			}
		private:
			std::function<const std::map<TKey, TOutput*>(TInput&)> process_;
		};

		//---------------------------------------------------------------------------------
		template<typename TKey, typename TInput, typename TOutput> class selector {
		public:
			selector() {}
			selector(selector<TKey, TInput, TOutput>& peer) : outputs_(peer.outputs_) {  }
			selector(selector<TKey, TInput, TOutput>&& peer) : outputs_(peer.outputs_) {  }
			virtual ~selector() {}
			std::function<void(TInput&)> Input() { return std::bind(&selector<TKey, TInput, TOutput>::Process, this, std::placeholders::_1); }
			plug<TOutput>& Outputs(const TKey& key);
		protected:
			void Remove(const TKey& key);
			inline void Send(const TKey& key, TOutput& value) { outputs_.at(key).Transmit(value); }
			virtual void Process(TInput& value) = 0;
		private:
			std::map<TKey, plug<TOutput>> outputs_;
		};

		template<typename TKey, typename TInput, typename TOutput>
		plug<TOutput>& selector<TKey, TInput, TOutput>::Outputs(const TKey& key)
		{
			std::map<TKey, plug<TOutput>>::iterator it = outputs_.find(key);
			if (it != outputs_.end()) {
				return it->second;
			}
			outputs_.emplace(key, plug<TOutput>());
			return outputs_.at(key);
		}

		template<typename TKey, typename TInput, typename TOutput>
		void selector<TKey, TInput, TOutput>::Remove(const TKey& key)
		{
			std::map<TKey, plug<TOutput>>::iterator it = outputs_.find(key);
			if (it == outputs_.end()) {
				return;
			}
			outputs_.erase(key);
		}
		//---------------------------------------------------------------------------------
		template<typename TKey, typename TInput, typename TOutput>
		class SimpleSelector : public selector < TKey, TInput, TOutput > {
		public:
			SimpleSelector() : selector<TKey, TInput, TOutput>::selector(peer) {}
			SimpleSelector(SimpleSelector<TKey, TInput, TOutput>& peer) : selector<TKey, TInput, TOutput>::selector(peer), key_(peer.key_), process_(peer.process_) {}
			SimpleSelector(SimpleSelector<TKey, TInput, TOutput>&& peer) : selector<TKey, TInput, TOutput>::selector(peer), key_(peer.key_), process_(peer.process_) {}
			SimpleSelector(const std::function<std::pair<TKey, TOutput*>(TInput&)>& process) { process_ = process; }
			SimpleSelector& operator=(const std::function<std::pair<TKey, TOutput*>(TInput&)>& process) { process_ = process; return *this; }
			plug<TOutput>& operator()(const TKey& key) { return Outputs(key); }
			void operator()(const std::function<std::pair<TKey, TOutput*>(TInput&)>& process) { process_ = process; }
			virtual ~SimpleSelector() {}
		protected:
			virtual void Process(TInput& value) override {
				std::pair<TKey, TOutput*> pair = process_(value);
				Send(pair.first, *pair.second);
			}
		private:
			std::function<std::pair<TKey, TOutput*>(TInput&)> process_;
		};

		//---------------------------------------------------------------------------------

		template<typename TKey, typename TInput, typename TOutput> class restrictor {
		public:
			restrictor() {}
			restrictor(restrictor<TKey, TInput, TOutput>& peer)
				: sockets_(peer.sockets_), output_(peer.output_) {}
			restrictor(restrictor<TKey, TInput, TOutput>&& peer)
				: sockets_(peer.sockets_), output_(peer.output_) {}
			virtual ~restrictor() {}
			std::function<void(TInput&)> Inputs(const TKey& key);
			plug<TOutput>& Output() { return output_; }
		protected:
			virtual void Remove(const TKey& key);
			virtual void Process(const TKey&, TInput&) = 0;
			inline void Send(TOutput& value) { output_.Transmit(value); }
		private:
			std::map<TKey, socket<TKey, TInput>> sockets_;
			plug<TOutput> output_;
		};

		template<typename TKey, typename TInput, typename TOutput>
		std::function<void(TInput&)> restrictor<TKey, TInput, TOutput>::Inputs(const TKey& key)
		{
			std::map<TKey, socket<TKey, TInput>>::iterator it = sockets_.find(key);
			if (it != sockets_.end()) {
				return (it->second)();
			}
			sockets_.emplace(key, socket<TKey, TInput>(key, std::bind(&restrictor<TKey, TInput, TOutput>::Process, this, std::placeholders::_1, std::placeholders::_2)));
			return (sockets_.at(key))();
		}

		template<typename TKey, typename TInput, typename TOutput>
		void restrictor<TKey, TInput, TOutput>::Remove(const TKey& key)
		{
			std::map<TKey, socket<TKey, TOutput>>::iterator it = sockets_.find(key);
			if (it == sockets_.end()) {
				return;
			}
			sockets_.erase(key);
		}
		//---------------------------------------------------------------------------------
		template<typename TKey, typename TInput, typename TOutput>
		class SimpleRestrictor : public restrictor < TKey, TInput, TOutput > {
		public:
			SimpleRestrictor() : restrictor<TKey, TInput, TOutput>::restrictor() {}
			SimpleRestrictor(SimpleRestrictor<TKey, TInput, TOutput>& peer) : restrictor<TKey, TInput, TOutput>::restrictor(peer), process_(peer.process_) {}
			SimpleRestrictor(SimpleRestrictor<TKey, TInput, TOutput>&& peer) : restrictor<TKey, TInput, TOutput>::restrictor(peer), process_(peer.process_) {}
			SimpleRestrictor(const std::function<TOutput&(const TKey&, TInput&)>& process) : restrictor<TKey, TInput, TOutput>::restrictor() { process_ = process; }
			std::function<void(TInput&)> operator()(const TKey& key) { return Inputs(key); }
			virtual ~SimpleRestrictor() {}
		protected:
			virtual void Process(const TKey& key, TInput& value) override { Send(process_(key, value)); }
		private:
			std::function<TOutput&(const TKey&, TInput&)> process_;
		};

		//---------------------------------------------------------------------------------

		template<typename T> class asyncer {
		public:
			asyncer() : value_(nullptr) {}
			asyncer(asyncer<T>& peer) : output_(peer.output_), value_(nullptr) { }
			asyncer(asyncer<T>&& peer) : output_(peer.output_), value_(nullptr) { }
			virtual ~asyncer() {}
			std::function<void(T&)> Input() { return std::bind(&asyncer<T>::Receive, this, std::placeholders::_1); }
			plug<T>& Output() { return output_; }
			inline void Wait() { signal_.wait(); }
		protected:
			virtual void Receive(T& value);
			inline void Send(T& value) { output_.Transmit(value); }
		private:
			plug<T> output_;
			std::future<void> signal_;
			std::mutex mutex_;
			T* value_;
		};

		template<typename T>
		void asyncer<T>::Receive(T& value)
		{
			std::lock_guard<std::mutex> lock(mutex_);
			value_ = &value;
			signal_ = std::async(std::launch::async, [&]()
			{
				std::unique_lock<std::mutex> mutex(mutex_);
				T& forwarded = *value_;
				Send(forwarded);
			});
		}
		//--------------------------------------------------------------------------------
		template<typename T>
		plug<T>& operator>>(plug<T>& source, asyncer<T>& target)
		{
			source >> target.Input();
			return target.Output();
		}

		//---------------------------------------------------------------------------------
		template<typename T> class mover : public transceiver<T, T> {
		public:
			mover() {}
			mover(const T& value) : value_(value) {}
			mover(mover<T>& peer) : output_(peer.output_), value_(peer.value_) { }
			mover(mover<T>&& peer) : output_(peer.output_), value_(peer.value_) { }
			virtual ~mover() {}
			inline void Wait() { signal_.wait(); }
		protected:
			virtual void Process(T& value) override;
		private:
			std::mutex mutex_;
			T value_;
		};

		template<typename T>
		void mover<T>::Process(T& value)
		{
			std::lock_guard<std::mutex> lock(mutex_);
			value_ = std::move(value);
			Send(value_);
		}

		template<typename T>
		class capacitor : public transceiver<T, T> {
		public:
			capacitor(int capacity = 1) : capacity_(capacity) {}
			inline void SetCapacity(int capacity) { capacity_ = capacity; }
			inline int& GetCapacity() { return capacity_; }
		protected:
			virtual void Process(T& item) override;
		private:
			unsigned int capacity_;
			std::list<T*> items_;
		};

		template<typename T>
		void capacitor<T>::Process(T& item)
		{
			items_.push_back(&item);
			if (items_.size() < capacity_) {
				return;
			}
			while (!items_.empty()) {
				T* item = items_.front();
				items_.pop_back();
				Send(*item);
			}
		}
		//--------------------------------------------------------------------
		template<typename T> class async_buffer {
		public:
			async_buffer() {}
			async_buffer(async_buffer<T>& peer) : output_(peer.output_), run_() { }
			async_buffer(async_buffer<T>&& peer) : output_(peer.output_) { }
			virtual ~async_buffer();
			std::function<void(T&)> Input() { return std::bind(&async_buffer<T>::Receive, this, std::placeholders::_1); }
			plug<T>& Output() { return output_; }
		protected:
			virtual void Receive(T& value);
			virtual void Process();
			inline void Send(T& value) { output_.Transmit(value); }
		private:
			plug<T> output_;
			std::future<void> signal_;
			std::mutex mutex_;
			std::queue<T*> values_;
			std::condition_variable cv_;
			std::atomic_flag run_;
		};

		template<typename T>
		async_buffer<T>::~async_buffer()
		{
			{
				std::lock_guard<std::mutex> lock(mutex_);
				run_.clear();
				if (values_.empty()) {
					cv_.notify_one();
				}
			}
			signal_.wait();
		}

		template<typename T>
		void async_buffer<T>::Receive(T& value)
		{
			std::unique_lock<std::mutex> lock(mutex_);
			if (values_.empty()) {
				cv_.notify_one();
			}
			values_.push(&value);
			if (!run_.test_and_set()) {
				signal_ = std::async(std::launch::async, [&]() { while (run_.test_and_set()) { Process(); } });
			}
		}

		template<typename T>
		void async_buffer<T>::Process()
		{
			std::unique_lock<std::mutex> lock(mutex_);
			if (values_.empty()) {
				cv_.wait(lock);
			}
			while (!values_.empty()) {
				T& forwarded = *values_.front();
				Send(forwarded);
				values_.pop();
			}
		}
		//--------------------------------------------------------------------------------
	}//end of namespace ps

	namespace pl
	{
		template<typename T>
		class isimplex {
		public:
			virtual T& Request() = 0;
		};

		template<typename T>
		class plug : public isimplex<T> {
		public:
			plug() {}
			plug(plug<T>& peer) : target_(peer.target_) {};
			plug(plug<T>&& peer) : target_(peer.target_) {};
			virtual ~plug() {}
			inline plug<T>& operator<<(std::function<T&(void)> target) { target_ = target; return *this; }
			virtual T& Request() { return target_(); }
			inline void Connect(const std::function<T&(void)>& target) { target_ = target; }
		private:
			std::function<T&(void)> target_;
		};

		template<typename T>
		void operator<<(T&(*fptr)(void), dvo::pl::plug<T>& target) {
			std::function<T&(void)>(fptr) << target;
		}

		template<typename TKey, typename TValue>
		class socket {
		public:
			socket() {}
			socket(const TKey& key, const std::function<TValue&(const TKey& key)>& target) : key_(key), target_(target) {}
			socket(socket<TKey, TValue>& peer) : target_(peer.target_), key_(peer.key_) {}
			socket(socket<TKey, TValue>&& peer) : target_(peer.target_), key_(peer.key_) {}
			std::function<TValue&(void)> operator()() { return std::bind(&socket<TKey, TValue>::Process, this); }
			virtual ~socket() {  }
		protected:
			TValue& Process() { return target_(key_); }
		private:
			TKey key_;
			std::function<TValue&(const TKey& key)> target_;
		};

		//----------------------------------------------------------------------------------
		template<typename T>
		class receiver {
		public:
			receiver();
			receiver(receiver<T>& peer) {}
			receiver(receiver<T>&& peer) {}
			virtual ~receiver();
			inline plug<T>& Input() { return input_; }
			virtual void Process() = 0;
		protected:
			inline T& Demand() { return input_.Request(); }
		private:
			plug<T> input_;
		};

		template<typename T>
		receiver<T>::receiver()
		{

		}

		template<typename T>
		receiver<T>::~receiver()
		{

		}
		//--------------------------------------------------------------------------------
		template<typename T>
		void operator<<(const std::function<T&(void)>& source, dvo::pl::plug<T>& target)
		{
			target.Connect(source);
		}
		//---------------------------------------------------------------------------------
		template<typename T>
		class SimpleReceiver : public receiver<T>
		{
		public:
			SimpleReceiver(std::function<void(T&)> process) { process_ = process; }
			SimpleReceiver(SimpleReceiver<T>& peer) : process_(peer.process_) { }
			SimpleReceiver(SimpleReceiver<T>&& peer) : process_(peer.process_) { }
			virtual ~SimpleReceiver() {}
			SimpleReceiver& operator=(const std::function<void(T&)>& process) { process_ = process; return *this; }
			virtual void Process() override { process_(Demand()); }
		private:
			std::function<void(T&)> process_;
		};
		//---------------------------------------------------------------------------------
		template<typename T> class transmitter {
		public:
			transmitter() {}
			transmitter(transmitter<T>& peer) {}
			transmitter(transmitter<T>&& peer) {}
			virtual ~transmitter() {}
			std::function<T&(void)> Output() { return std::bind(&transmitter<T>::Process, this); }
		protected:
			virtual T& Process() = 0;
		};

		//---------------------------------------------------------------------------------
		template<typename T>
		class SimpleTransmitter : public transmitter<T>
		{
		public:
			SimpleTransmitter(SimpleTransmitter<T>& peer) : process_(peer.process_) { }
			SimpleTransmitter& operator=(const std::function<T&(void)>& process) { process_ = process; return *this; }
			SimpleTransmitter(const std::function<T&(void)>& process) { process_ = process; }
			virtual ~SimpleTransmitter() {}
		protected:
			virtual T& Process() override { return process_(); }
		private:
			std::function<T&(void)> process_;
		};
		//---------------------------------------------------------------------------------
		template<typename TInput, typename TOutput> class transceiver {
		public:
			transceiver() {}
			transceiver(transceiver<TInput, TOutput>& peer) : input_(peer.input_) {};
			transceiver(transceiver<TInput, TOutput>&& peer) : input_(peer.input_) {};
			virtual ~transceiver() {}
			plug<TInput>& Input() { return input_; }
			inline std::function<TOutput&(void)> Output() { return std::bind(&transceiver<TInput, TOutput>::Process, this); }
		protected:
			inline TInput& Demand() { return input_.Request(); }
			virtual TOutput& Process() = 0;
		private:
			plug<TInput> input_;
		};

		//---------------------------------------------------------------------------------
		template<typename TInput, typename TOutput>
		class SimpleTransceiver : public transceiver<TInput, TOutput>
		{
		public:
			SimpleTransceiver() {}
			SimpleTransceiver(SimpleTransceiver<TInput, TOutput>& peer) : process_(peer.process_) { }
			SimpleTransceiver(SimpleTransceiver<TInput, TOutput>&& peer) : process_(peer.process_) { }
			SimpleTransceiver& operator=(const std::function<TOutput&(TInput&)>& process) { process_ = process; return *this; }
			SimpleTransceiver(const std::function<TOutput&(TInput&)>& process) { process_ = process; }
			virtual ~SimpleTransceiver() {}
		protected:
			virtual TOutput& Process() override { return process_(Demand()); }
		private:
			std::function<TOutput&(TInput&)> process_;
		};

		//----------------------------------------------------------------------
		template<typename TInputX, typename TInputY, typename TOutput>
		class merger
		{
		public:
			merger() {}
			merger(merger<TInputX, TInputY, TOutput>& peer) : process_(peer.process_) { }
			merger(merger<TInputX, TInputY, TOutput>&& peer) : process_(peer.process_) { }
			virtual ~merger() {}
			inline plug<TInputX>& InputX() { return inputx_; }
			inline plug<TInputY>& InputY() { return inputy_; }
			inline std::function<TOutput&(void)> Output() { return std::bind(&merger<TInputX, TInputY, TOutput>::Process, this); }
		protected:
			inline TInputX& DemandX() { return  inputx_.Request(); }
			inline TInputY& DemandY() { return  inputy_.Request(); }
			virtual TOutput& Process() = 0;
		private:
			plug<TInputX> inputx_;
			plug<TInputY> inputy_;
		};

		//---------------------------------------------------------------------------------
		template<typename TInputX, typename TInputY, typename TOutput>
		class SimpleMerger : public merger<TInputX, TInputY, TOutput>
		{
		public:
			SimpleMerger() { }
			SimpleMerger(SimpleMerger<TInputX, TInputY, TOutput>& peer) : process_(peer.process_) { }
			SimpleMerger(SimpleMerger<TInputX, TInputY, TOutput>&& peer) : process_(peer.process_) { }
			SimpleMerger& operator=(const std::function<TOutput&(TInputX&, TInputY&)>& process) { process_ = process; return *this; }
			SimpleMerger(const std::function<TOutput&(TInputX&, TInputY&)>& process) { process_ = process; }
			virtual ~SimpleMerger() { }
		protected:
			virtual TOutput& Process() override { return process_(DemandX(), DemandY()); }
		private:
			std::function<TOutput&(TInputX&, TInputY&)> process_;
		};

		//----------------------------------------------------------------------
		template<typename TInput, typename TOutputX, typename TOutputY>
		class splitter
		{
		public:
			splitter() {}
			splitter(splitter<TInput, TOutputX, TOutputY>& peer) : process_(peer.process_) { }
			splitter(splitter<TInput, TOutputX, TOutputY>&& peer) : process_(peer.process_) { }
			virtual ~splitter() {}
			inline plug<TInput>& Input() { return input_; }
			inline std::function<TOutputX&(void)> OutputX() { return std::bind(&splitter<TInput, TOutputX, TOutputY>::ProcessX, this); }
			inline std::function<TOutputY&(void)> OutputY() { return std::bind(&splitter<TInput, TOutputX, TOutputY>::ProcessY, this); }
		protected:
			inline TInput& Demand() { return  input_.Request(); }
			virtual TOutputX& ProcessX() = 0;
			virtual TOutputY& ProcessY() = 0;
		private:
			plug<TInput> input_;
		};
		//---------------------------------------------------------------------------------
		template<typename TInput, typename TOutputX, typename TOutputY>
		class SimpleSplitter : public splitter<TInput, TOutputX, TOutputY>
		{
		public:
			SimpleSplitter() { }
			SimpleSplitter(SimpleSplitter<TInput, TOutputX, TOutputY>& peer) : process_(peer.process_) { }
			SimpleSplitter(SimpleSplitter<TInput, TOutputX, TOutputY>&& peer) : process_(peer.process_) { }
			virtual ~SimpleSplitter() { }
			SimpleSplitter& operator|=(const std::function<TOutputX&(TInput&)>& processx) { processx_ = processx; return *this; }
			SimpleSplitter& operator&=(const std::function<TOutputY&(TInput&)>& processy) { processy_ = processy; return *this; }
			inline void SetProcessY(const std::function<TOutputY&(TInput&)>& processx) { processy_ = processy; }
		protected:
			virtual TOutputX& ProcessX() override { return processx_(Demand()); }
			virtual TOutputY& ProcessY() override { return processy_(Demand()); }
		private:
			std::function<TOutputX&(TInput&)> processx_;
			std::function<TOutputY&(TInput&)> processy_;
		};


		//-----------------------------------------------------------------
		template<typename TKey, typename TInput, typename TOutput> class joiner {
		public:
			joiner() {}
			joiner(joiner<TKey, TInput, TOutput>& peer) : inputs_(peer.inputs_) {}
			joiner(joiner<TKey, TInput, TOutput>&& peer) : inputs_(peer.inputs_) {}
			virtual ~joiner() {}
			plug<TInput>& Inputs(const TKey& key);
			void Remove(const TKey& key);
			std::function<TOutput&(void)> Output() { return std::bind(&joiner<TKey, TInput, TOutput>::Supply, this); }
		protected:
			virtual TOutput& Supply();
			virtual TOutput& Process(std::map<TKey, TInput*>& values) = 0;
			inline TInput& Demand(const TKey& key) { return inputs_.at(key).Request(); }
		private:
			std::map<TKey, plug<TInput>> inputs_;
			std::mutex mutex_;
		};

		template<typename TKey, typename TInput, typename TOutput>
		plug<TInput>& joiner<TKey, TInput, TOutput>::Inputs(const TKey& key)
		{
			std::map<TKey, plug<TInput>>::iterator it = inputs_.find(key);
			if (it != inputs_.end()) {
				return it->second;
			}
			inputs_.emplace(key, plug<TInput>());
			return inputs_.at(key);
		}

		template<typename TKey, typename TInput, typename TOutput>
		void joiner<TKey, TInput, TOutput>::Remove(const TKey& key)
		{
			std::map<TKey, socket<TKey, TInput>>::iterator it = sockets_.find(key);
			if (it == sockets_.end()) {
				return;
			}
			inputs_.erase(it);
		}

		template<typename TKey, typename TInput, typename TOutput>
		TOutput& joiner<TKey, TInput, TOutput>::Supply()
		{
			std::unique_lock<std::mutex> mutex(mutex_);
			std::map<TKey, TInput*> values;
			for (std::pair<const TKey, plug<TInput>>& pair : inputs_) {
				TInput& value = Demand(pair.first);
				values.emplace(pair.first, &value);
			}

			return Process(values);
		}
		//--------------------------------------------------------------------
		template<typename TKey, typename TInput, typename TOutput>
		class SimpleJoiner : public joiner<TKey, TInput, TOutput> {
		public:
			SimpleJoiner() {}
			SimpleJoiner(SimpleJoiner<TKey, TInput, TOutput>& peer) {}
			SimpleJoiner(SimpleJoiner<TKey, TInput, TOutput>&& peer) {}
			SimpleJoiner(const std::function<TOutput&(std::map<TKey, TInput*>&)>& process) : process_(process) {}
			SimpleJoiner& operator=(const std::function<TOutput&(std::map<TKey, TInput*>&)>& process) { process_ = process; return *this; }
			plug<TInput>& operator()(const TKey& key) { return Inputs(key); }
			virtual ~SimpleJoiner() {}
		protected:
			virtual TOutput& Process(std::map<TKey, TInput*>& values)  override { return process_(values); }
		private:
			std::function<TOutput&(std::map<TKey, TInput*>&)> process_;
		};

		//---------------------------------------------------------------------------------
		template<typename T>
		class capacitor : public transceiver<T, T> {
		public:
			capacitor(int capacity = 1) : capacity_(capacity) {}
			inline void SetCapacity(int capacity) { capacity_ = capacity; }
			inline int& GetCapacity() { return capacity_; }
		protected:
			virtual T& Process() override;
		private:
			inline T& Emit() { T* item = items_.front(); items_.pop(); return *item; }
			unsigned int capacity_;
			std::queue<T*> items_;
		};

		template<typename T>
		T& capacitor<T>::Process() {
			if (!items_.empty()) {
				return Emit();
			}
			for (unsigned int i = 0; i < capacity_; ++i) {
				T& item = Demand();
				items_.push(&item);
			}
			return Emit();
		}
		//---------------------------------------------------------------------------------
		template<typename TKey, typename TInput, typename TOutput>
		class detacher {
		public:
			detacher() {}
			detacher(detacher<TKey, TInput, TOutput>& peer) : sockets_(peer.outputs_), initiated_() {}
			detacher(detacher<TKey, TInput, TOutput>&& peer) : sockets_(peer.outputs_), initiated_() {}
			virtual ~detacher() {}
			std::function<TOutput&(void)> Outputs(const TKey& key);
			plug<TOutput>& Input() { return input_; }
			void Remove(const TKey& key);
		protected:
			virtual TOutput& Process(const TKey& key) = 0;
			virtual TInput& Demand();
		private:
			plug<TInput> input_;
			std::map<TKey, socket<TKey, TOutput>> sockets_;
			std::atomic_flag initiated_;
			std::future<TInput&> value_;
		};

		template<typename TKey, typename TInput, typename TOutput>
		TInput& detacher<TKey, TInput, TOutput>::Demand()
		{
			if (!initiated_.test_and_set()) {
				value_ = std::async(std::launch::async, [&]()->TInput& { return input_.Request(); });
			}
			TInput& result = value_.get();
			value_ = std::async(std::launch::async, [&]()->TInput& { return input_.Request(); });
			return result;
		}

		template<typename TKey, typename TInput, typename TOutput>
		std::function<TOutput&(void)> detacher<TKey, TInput, TOutput>::Outputs(const TKey& key)
		{
			std::map<TKey, socket<TKey, TOutput>>::iterator it = sockets_.find(key);
			if (it != sockets_.end()) {
				return it->second();
			}
			sockets_.emplace(key, socket<TKey, TOutput>(key, std::bind(&detacher<TKey, TInput, TOutput>::Process, this, std::placeholders::_1)));
			return sockets_.at(key)();
		}
		template<typename TKey, typename TInput, typename TOutput>
		void detacher<TKey, TInput, TOutput>::Remove(const TKey& key)
		{
			sockets_.erase(key);
		}
		//---------------------------------------------------------------------------------
		template<typename TKey, typename TInput, typename TOutput>
		class SimpleDetacher : public detacher < TKey, TInput, TOutput > {
		public:
			SimpleDetacher() {}
			SimpleDetacher(SimpleDetacher<TKey, TInput, TOutput>& peer) : detacher<TKey, TInput, TOutput>::detacher(peer), process_(peer.process_) { }
			SimpleDetacher(SimpleDetacher<TKey, TInput, TOutput>&& peer) : detacher<TKey, TInput, TOutput>::detacher(peer), process_(peer.process_) { }
			SimpleDetacher(const std::function<TOutput&(const TKey&, TInput&)> process) : process_(process) { }
			SimpleDetacher& operator=(const std::function<TOutput&(const TKey&, TInput&)> process) { process_ = process; return *this; }
			virtual ~SimpleDetacher() {}
		protected:
			virtual TOutput& Process(const TKey& key) override {
				return process_(key, Demand());
			}
		private:
			std::function<TOutput&(const TKey&, TInput&)> process_;
		};
		//---------------------------------------------------------------------------------
		template<typename TKey, typename TInput, typename TOutput> class selector {
		public:
			selector() { }
			selector(selector<TKey, TInput, TOutput>& peer) : inputs_(peer.inputs_) { }
			selector(selector<TKey, TInput, TOutput>&& peer) : inputs_(peer.inputs_) { }
			virtual ~selector() { }
			plug<TInput>& Inputs(const TKey& key);
			std::function<TOutput&(void)> Output() { return std::bind(&selector<TKey, TInput, TOutput>::Process, this); }
			virtual void Remove(const TKey& key);
		protected:
			virtual TInput& Demand(const TKey& key);
			virtual TOutput& Process() = 0;
		private:
			std::map<TKey, plug<TInput>> inputs_;
		};

		template<typename TKey, typename TInput, typename TOutput>
		plug<TInput>& selector<TKey, TInput, TOutput>::Inputs(const TKey& key)
		{
			std::map<TKey, plug<TInput>>::iterator it = inputs_.find(key);
			if (it != inputs_.end()) {
				return inputs_.at(key);
			}
			inputs_.emplace(key, plug<TInput>());
			return inputs_.at(key);
		}

		template<typename TKey, typename TInput, typename TOutput>
		void selector<TKey, TInput, TOutput>::Remove(const TKey& key)
		{
			std::map<TKey, plug<TInput>>::iterator it = inputs_.find(key);
			if (it == inputs_.end()) {
				return;
			}
			inputs_.erase(it);
		}

		template<typename TKey, typename TInput, typename TOutput>
		TInput& selector<TKey, TInput, TOutput>::Demand(const TKey& key)
		{
			return inputs_.at(key).Request();
		}
		//---------------------------------------------------------------------------------
		template<typename TKey, typename TInput, typename TOutput>
		class SimpleSelector : public selector < TKey, TInput, TOutput > {
		public:
			SimpleSelector() : selector<TKey, TInput, TOutput>::selector(peer) {}
			SimpleSelector(SimpleSelector<TKey, TInput, TOutput>& peer) : selector<TKey, TInput, TOutput>::selector(peer), key_(peer.key_), process_(peer.process_) {}
			SimpleSelector(SimpleSelector<TKey, TInput, TOutput>&& peer) : selector<TKey, TInput, TOutput>::selector(peer), key_(peer.key_), process_(peer.process_) {}
			SimpleSelector(const std::function<TOutput&(TInput&)>& process) { process_ = process; }
			SimpleSelector& operator=(const std::function<TOutput&(TInput&)>& process) { process_ = process; return *this; }
			inline plug<TKey>& Key() { return key_; }
			inline plug<TOutput>& operator()(const TKey& key) { return Inputs(key); }
			void operator()(const std::function<TOutput&(TInput&)>& process) { process_ = process; }
			virtual ~SimpleSelector() {}
		protected:
			virtual TOutput& Process() override {
				return process_(Demand(key_.Request()));
			}
		private:
			plug<TKey> key_;
			std::function<TOutput&(TInput&)> process_;
		};

		//---------------------------------------------------------------------------------

		template<typename TKey, typename TInput, typename TOutput> class restrictor {
		public:
			restrictor() {}
			restrictor(restrictor<TKey, TInput, TOutput>& peer)
				: sockets_(peer.sockets_), output_(peer.output_) {}
			restrictor(restrictor<TKey, TInput, TOutput>&& peer)
				: sockets_(peer.sockets_), output_(peer.output_) {}
			virtual ~restrictor() {}
			std::function<TOutput&(void)> Outputs(const TKey& key);
			plug<TInput>& Input() { return input_; }
		protected:
			virtual void Remove(const TKey& key);
			virtual TOutput& Process(const TKey&) = 0;
			inline TInput& Demand() { return input_.Request(); }
		private:
			std::map<TKey, socket<TKey, TOutput>> sockets_;
			plug<TInput> input_;
		};

		template<typename TKey, typename TInput, typename TOutput>
		std::function<TOutput&(void)> restrictor<TKey, TInput, TOutput>::Outputs(const TKey& key)
		{
			std::map<TKey, socket<TKey, TInput>>::iterator it = sockets_.find(key);
			if (it != sockets_.end()) {
				return (it->second)();
			}
			sockets_.emplace(key, socket<TKey, TInput>(key, std::bind(&restrictor<TKey, TInput, TOutput>::Process, this, std::placeholders::_1)));
			return (sockets_.at(key))();
		}

		template<typename TKey, typename TInput, typename TOutput>
		void restrictor<TKey, TInput, TOutput>::Remove(const TKey& key)
		{
			std::map<TKey, socket<TKey, TOutput>>::iterator it = sockets_.find(key);
			if (it == sockets_.end()) {
				return;
			}
			sockets_.erase(key);
		}

		//---------------------------------------------------------------------------------
		template<typename TKey, typename TInput, typename TOutput>
		class SimpleRestrictor : public restrictor < TKey, TInput, TOutput > {
		public:
			SimpleRestrictor() : selector<TKey, TInput, TOutput>::selector(peer) {}
			SimpleRestrictor(SimpleRestrictor<TKey, TInput, TOutput>& peer) : selector<TKey, TInput, TOutput>::selector(peer), key_(peer.key_), process_(peer.process_) {}
			SimpleRestrictor(SimpleRestrictor<TKey, TInput, TOutput>&& peer) : selector<TKey, TInput, TOutput>::selector(peer), key_(peer.key_), process_(peer.process_) {}
			SimpleRestrictor(const std::function<TOutput&(const TKey&, TInput&)>& process) { process_ = process; }
			SimpleRestrictor& operator=(const std::function<TOutput&(const TKey&, TInput&)>& process) { process_ = process; return *this; }
			inline std::function<TOutput&(void)> operator()(const TKey& key) { return Outputs(key); }
			void operator()(const std::function<TOutput&(const TKey&, TInput&)>& process) { process_ = process; }
			virtual ~SimpleRestrictor() {}
		protected:
			virtual TOutput& Process(const TKey& key) override {
				return process_(key, Demand());
			}
		private:
			std::function<TOutput&(const TKey&, TInput&)> process_;
		};
		//---------------------------------------------------------------------------------

		template<typename T> class asyncer {
		public:
			asyncer() { started_.clear(); }
			asyncer(asyncer<T>& peer) : input_(peer.input_),started_() { started_.clear(); }
			asyncer(asyncer<T>&& peer) : input_(peer.input_), started_() { started_.clear(); }
			virtual ~asyncer() {}
			inline plug<T>& Input() { return input_; }
			inline std::function<T&(void)> Output() { return std::bind(&asyncer<T>::Supply, this); }
			inline void Wait() { signal_.wait(); }
		protected:
			virtual T& Supply();
			inline T& Demand() { return input_.Request(); }
		private:
			plug<T> input_;
			std::future<T*> signal_;
			std::atomic_flag started_;
			std::mutex mutex_;
		};

		template<typename T>
		T& asyncer<T>::Supply()
		{
			std::lock_guard<std::mutex> lock(mutex_);
			if (!started_.test_and_set()) {
				signal_ = std::async(std::launch::async, [&]()->T* { return &Demand(); });
			}
			signal_.wait();
			T* result = signal_.get();
			signal_ = std::async(std::launch::async, [&]() { std::lock_guard<std::mutex> lock(mutex_); return &Demand(); });
			return *result;
		}
		//---------------------------------------------------------------------------------
		template<typename T> class mover {
		public:
			mover() { }
			mover(mover<T>& peer) : input_(peer.input_) { }
			mover(mover<T>&& peer) : input_(peer.input_) { }
			virtual ~mover() { }
			inline plug<T>& Input() { return input_; }
			inline std::function<T&(void)> Output() { return std::bind(&mover<T>::Supply, this); }
		protected:
			virtual T& Supply();
			inline T& Demand() { return input_.Request(); }
		private:
			plug<T> input_;
			T value_;
		};

		template<typename T>
		T& mover<T>::Supply()
		{
			T& requested = Demand();
			value_ = std::move(requested);
			return value_;
		}
		//---------------------------------------------------------------------------------

		template<typename TContainer, typename T>
		class iterator : public transceiver<TContainer, T> {

		};

		template<typename T>
		class iterator <std::vector<T>, T > : public transceiver<std::vector<T>, T>{
		public:
			iterator() : items_(nullptr) { }
		protected:
			T& Process() override;
		private:
			std::vector<T>* items_;
			unsigned int index_;
		};

		template<typename T>
		T& iterator<std::vector<T>, T>::Process()
		{
			if (items_ == nullptr || items_->empty()) {
				items_ = &Demand();
				index_ = 0;
			}
			T& result = items_->at(index_++);
			return result;
		}
	}//end of namespace pl

	namespace as
	{
		template<typename T>
		class relayer
		{
		public:
			relayer() {}
			relayer(relayer<T>& peer) : input_(peer.input_), output_(peer.output_) {}
			relayer(relayer<T>&& peer) : input_(peer.input_), output_(peer.output_) {}
			virtual ~relayer() {}
			inline pl::plug<T>& Input() { return input_; }
			inline ps::plug<T>& Output() { return output_; }
			virtual void Process(int&);
		protected:
			inline T& Demand() { return input_.Request(); }
			inline void Send(T& value) { output_.Transmit(value); }
		private:
			pl::plug<T> input_;
			ps::plug<T> output_;
		};

		template<typename T>
		void relayer<T>::Process(int& signal)
		{
			Send(Demand());
		}

		template<typename T>
		class accumulator
		{
		public:
			accumulator(int capacity = 0) : capacity_(capacity) {}
			accumulator(accumulator<T>& peer) : capacity_(peer.capacity_) { }
			accumulator(accumulator<T>&& peer) : capacity_(peer.capacity_) { }
			virtual ~accumulator() { Flush(); }
			virtual void Flush();
			inline std::function<void(T&)> Input() { return std::bind(&accumulator<T>::Receive, this, std::placeholders::_1); }
			inline std::function<T&(void)> Output() { return std::bind(&accumulator<T>::Supply, this); }
		protected:
			virtual void Receive(T& value);
			virtual T& Supply();
			inline const unsigned int Size() { return queue_.size(); }
		private:
			std::condition_variable cv_;
			std::queue<T*> puffer_;
			const int capacity_;
			std::mutex mutex_;
		};

		template<typename T>
		void accumulator<T>::Flush()
		{
			std::unique_lock<std::mutex> lock(mutex_);
			while (!puffer_.empty()) {
				delete puffer_.front();
				puffer_.pop();
			}
		}

		template<typename T>
		void accumulator<T>::Receive(T& value)
		{
			std::unique_lock<std::mutex> lock(mutex_);
			const int count = puffer_.size();
			if (capacity_ > 0 && count >= capacity_) {
				cv_.wait(lock);
			}
			puffer_.push(&value);
			if (count == 0) {
				cv_.notify_all();
			}
		}

		template<typename T>
		T& accumulator<T>::Supply()
		{
			std::unique_lock<std::mutex> lock(mutex_);
			const int count = puffer_.size();
			if (count == 0) {
				cv_.wait(lock);
			}
			T* result = puffer_.front();
			puffer_.pop();
			if (capacity_ > 0 && count >= capacity_) {
				cv_.notify_one();
			}
			return *result;
		}

		//--------------------------------------------------------------------------------------

		template<typename T>
		class recycle
		{
		public:
			recycle(recycle<T>& peer) : factory_(peer.factory_) {}
			recycle(recycle<T>&& peer) : factory_(peer.factory_) {}
			recycle() : factory_([&]()->T* {return new T(); }) {}
			recycle(const std::function<T*(void)>& factory) : factory_(factory) {}
			virtual ~recycle() { Flush(); }
			void virtual Flush();
			inline std::function<void(T&)> Input() { return std::bind(&recycle<T>::Receive, this, std::placeholders::_1); }
			inline std::function<T&(void)> Output() { return std::bind(&recycle<T>::Supply, this); }
		protected:
			virtual void Receive(T& value);
			virtual T& Supply();
			virtual void Clean(T* obj) {}
		private:
			std::function<T*(void)> factory_;
			std::queue<T*> puffer_;
			std::mutex mutex_;
		};

		template<typename T>
		void recycle<T>::Flush()
		{
			std::unique_lock<std::mutex> lock(mutex_);
			while (!puffer_.empty()) {
				delete puffer_.front();
				puffer_.pop();
			}
		}

		template<typename T>
		void recycle<T>::Receive(T& value)
		{
			std::unique_lock<std::mutex> lock(mutex_);
			Clean(&value);
			puffer_.push(&value);
		}

		template<typename T>
		T& recycle<T>::Supply()
		{
			std::unique_lock<std::mutex> lock(mutex_);
			const unsigned int count = puffer_.size();
			if (count == 0) {
				return *factory_();
			}
			T* result = puffer_.front();
			puffer_.pop();
			return *result;
		}

	}//end of namespace as

}//end of namespace dvo


template<typename T>
void operator>>(T& value, dvo::ps::plug<T>& target)
{
	target.Transmit(value);
}

template<typename T>
void operator>>(const T& value, dvo::ps::plug<T>& target)
{
	target.Transmit(value);
}

template<typename T>
void operator<<(const std::function<T&(void)>& source, T& target)
{
	target = source();
}

template<typename T>
void operator<<(const std::function<T&(void)>& source, T*& target)
{
	target = &source();
}

template<typename T>
void operator<<(T& source, dvo::pl::plug<T>& target)
{
	std::function<T&(void)>([&](void)->T& { return source; }) << target;
}

template<typename T>
void operator<<(T&& source, dvo::pl::plug<T>& target)
{
	std::function<T&(void)>([&](void)->T& { return source; }) << target;
}

//----------------------------------------------------------------------------------



#undef DISALLOW_ASSIGN
#undef DISALLOW_COPY
#undef DISALLOW_MOVE
#undef DISALLOW_COPY_AND_ASSIGN
#undef DISALLOW_MOVE_AND_ASSIGN
#undef DISALLOW_COPY_AND_MOVE_AND_ASSIGN
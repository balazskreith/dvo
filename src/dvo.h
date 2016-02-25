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
			virtual void Process() override { transmitter<T>::Send(process_()); }
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
			virtual void Process(TInput& value) override { transceiver<TInput, TOutput>::Send(process_(value)); }
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
			virtual void Process(TInX& x, TInY& y) override { merger<TInX, TInY, TOutput>::Send(process_(x, y)); }
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
				std::tuple<TOutX&, TOutY&> values = process_(value); splitter<TInput, TOutX, TOutY>::SendX(std::get<0>(values)); splitter<TInput, TOutX, TOutY>::SendY(std::get<1>(values));
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
		        typename std::map<TKey, plug<TOutput>>::iterator it = outputs_.find(key);
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
		        typename std::map<TKey, plug<TOutput>>::iterator it = outputs_.find(key);
			if (it != outputs_.end()) {
				return it->second;
			}
			outputs_.emplace(key, plug<TOutput>());
			return outputs_.at(key);
		}

		template<typename TKey, typename TInput, typename TOutput>
		void selector<TKey, TInput, TOutput>::Remove(const TKey& key)
		{
		        typename std::map<TKey, plug<TOutput>>::iterator it = outputs_.find(key);
			if (it == outputs_.end()) {
				return;
			}
			outputs_.erase(key);
		}
		//---------------------------------------------------------------------------------
		template<typename TKey, typename TInput, typename TOutput>
		class SimpleSelector : public selector < TKey, TInput, TOutput > {
		public:
			SimpleSelector() : selector<TKey, TInput, TOutput>::selector() {}
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
			TKey key_;
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
		        typename std::map<TKey, socket<TKey, TInput>>::iterator it = sockets_.find(key);
			if (it != sockets_.end()) {
				return (it->second)();
			}
			sockets_.emplace(key, socket<TKey, TInput>(key, std::bind(&restrictor<TKey, TInput, TOutput>::Process, this, std::placeholders::_1, std::placeholders::_2)));
			return (sockets_.at(key))();
		}

		template<typename TKey, typename TInput, typename TOutput>
		void restrictor<TKey, TInput, TOutput>::Remove(const TKey& key)
		{
		        typename std::map<TKey, socket<TKey, TOutput>>::iterator it = sockets_.find(key);
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
		template<typename T> class mover : public asyncer<T> {
		public:
			mover() {}
			mover(const T& value) : value_(value) {}
			mover(mover<T>& peer) : asyncer<T>::output_(peer.output_), value_(peer.value_) { }
			mover(mover<T>&& peer) : asyncer<T>::output_(peer.output_), value_(peer.value_) { }
			virtual ~mover() {}
			inline void Wait() { asyncer<T>::signal_.wait(); }
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
			virtual ~capacitor() {}
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

//----------------------------------------------------------------------------------



#undef DISALLOW_ASSIGN
#undef DISALLOW_COPY
#undef DISALLOW_MOVE
#undef DISALLOW_COPY_AND_ASSIGN
#undef DISALLOW_MOVE_AND_ASSIGN
#undef DISALLOW_COPY_AND_MOVE_AND_ASSIGN

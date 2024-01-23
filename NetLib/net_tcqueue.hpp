#pragma once
#include "net_common.hpp"

namespace olc
{
	namespace net
	{
		//реализация потокобезопасной очереди
		template<typename T>
		class Tsqueue
		{
		protected:
			std::mutex muxQueue;
			std::deque<T> deqQueu;
		public:
			Tsqueue() = default;
			Tsqueue(const Tsqueue<T>&) = delete;
			virtual ~Tsqueue() { clear(); }

			const T& front()
			{
				std::lock_guard<std::mutex> lock{ muxQueue };
				return deqQueu.front();
			}

			const T& back()
			{
				std::lock_guard<std::mutex> lock{ muxQueue };
				return deqQueu.back();
			}

			const T& push_back(const T & item)
			{
				std::lock_guard<std::mutex> lock{ muxQueue };
				return deqQueu.emplace_back(std::move(item));
			}

			size_t count()
			{
				std::lock_guard<std::mutex> lock{ muxQueue };
				return deqQueu.size();
			}

			void clear()
			{
				std::lock_guard<std::mutex> lock{ muxQueue };
				return deqQueu.clear();
			}

			T pop_front()
			{
				std::lock_guard<std::mutex> lock(muxQueue);
				auto tmp = std::move(deqQueu.front());
				deqQueu.pop_front();
				return tmp;
			}

			T pop_back()
			{
				std::lock_guard<std::mutex> lock{ muxQueue };
				auto tmp = std::move(deqQueu.back());
				deqQueu.pop_back();
				return tmp;
			}

		};
	}
}
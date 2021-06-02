#pragma once

#include <Core/CoreModuleSettings.h>
#include <Core/System.h>
#include <functional>
#include <thread>
#include <vector>
#include <queue>

class ThreadPool {
public:
	ThreadPool(){}

	~ThreadPool() {
		_threads.reserve(_queues.size());
		for (auto& queue : _queues) {
			_threads.emplace_back(queue);
		}
		for (auto& thread : _threads) {
			thread.join();
		}
	}

	template<typename F, typename... Args>
	void enqueueWork(F&& f, Args&&... args) {
		auto work = [f, args...]() { f(args...); };
		_queues.push_back(work);
	}

private:
	using Proc = std::function<void(void)>;
	using Queues = std::vector<Proc>;
	Queues _queues;

	using Threads = std::vector<std::thread>;
	Threads _threads;
};
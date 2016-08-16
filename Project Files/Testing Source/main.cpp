#include <iostream>

#define _ASYNCHRONOUS_TASKS_
#include "../../AsyncTasks.h"

#include <vector>
using std::vector;

class BaseClass {
protected:
	void* mResult;

	BaseClass() : mResult(nullptr) {}
	virtual ~BaseClass() {
		delete mResult;
	}
};

template<class T>
struct Task : public BaseClass {
	std::function<T()> process;
	std::function<void(T&)> callback;

	Task<T>() : BaseClass() {}

	T& getResult() {
		return *(T*)mResult;
	}

	void setResult(T& pRes) {
		mResult = new T(pRes);
	}
};

template<>
struct Task<void> : public BaseClass {
	std::function<void()> process;
	std::function<void()> callback;

	Task() : BaseClass() {}
};

template<typename T>
int ProcessTask(Task<T>& pTask) {
	//Call the process function
	T result = pTask.process();

	//If there is a callback set call that
	if (pTask.callback)
		pTask.callback(result);

	//Set the result
	pTask.setResult(result);

	//Return a random ticket
	return rand();
}

template<>
int ProcessTask(Task<void>& pTask) {
	//Call the process function
	pTask.process();

	//Call the callback
	if (pTask.callback)
		pTask.callback();

	return rand();
}

int main() {
	Task<int> job;

	job.process = []() {
		return 42;
	};
	job.callback = [](int& pNum) {
		printf("%i\n\n\n\n\n", pNum);
	};

	int ticket = ProcessTask(job);
	int firstRes = job.getResult();

	{
		Task<float> job2;

		job2.process = []() {
			return 84.f;
		};
		job2.callback = [](float& pNum) {
			printf("%f\n\n\n\n\n", pNum);
		};

		ticket = ProcessTask(job2);
		float secondRes = job2.getResult();
	}

		Task<void> job3;

		job3.process = []() {
			for (int i = 0; i < 1000000; i++) {
				i++;
				i--;
			}
		};
		job3.callback = []() {
			for (int i = 0; i < 1000000; i++)
				printf("Hello %i\n", i);
		};

		ticket = ProcessTask(job3);

	system("PAUSE");
	return EXIT_SUCCESS;
}
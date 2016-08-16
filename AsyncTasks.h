#pragma once

#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <assert.h>

namespace AsynchTasks {

	/*
	 *		Name: TaskManager
	 *		Author: Mitchell Croft
	 *		Created: 16/08/2016
	 *		Modified: 16/08/2016
	 *
	 *		Purpose:
	 *		Complete tasks in a multi-threaded environment, while providing
	 *		options for order completion, priority and callbacks for the 
	 *		information.
	 *		
	 *		Manager makes use of a Manager thread to maintain the different
	 *		tasks with a number of Worker threads that complete the tasks.
	 *		The number of these workers are defined when creating the 
	 *		singleton instance.
	**/
	class TaskManager {
		#pragma region Internal Objects
		//! Worker class to preform the tasks
		class Worker;
		#pragma endregion

		/*----------Singleton Values----------*/
		static TaskManager* mInstance;
		TaskManager() = delete;
		TaskManager(unsigned int pWorkers);
		~TaskManager() = default;

		/*----------Variables----------*/
		//! Keep as a constant the number of workers in use
		const unsigned int mWorkerCount;

		//! Maintain a pointer to the Workers
		Worker* mWorkers;

		//! Maintain a thread for organising the different tasks
		std::thread mOrganisationThread;

		//! Flag if the Task Manager is operating
		std::atomic_flag mRunning;

		/*----------Functions----------*/
		//! Organise tasks in a separate thread
		void organiseTasks();

	public:
		//! Main operation functionality
		static bool create(unsigned int pWorkers = 5u);
		static void update();
		static void destroy(bool pForce = false);
	};

	/*
	 *		Name: Worker
	 *		Author: Mitchell Croft
	 *		Created: 
	 *		Modified:
	 *		
	 *		Purpose:
	 *		
	**/
	class TaskManager::Worker {

	};

	/*
	 
		Function Definitions:

		Include a define for _ASYNCHRONOUS_TASKS_ in a single .cpp file inside
		of the project to use the AsynchTasks namespace functionality

	*/
	#ifdef _ASYNCHRONOUS_TASKS_
	//! Define static singleton instance
	TaskManager* TaskManager::mInstance = nullptr;

	#pragma region Task Manager
	#pragma region Main Operation Functionality
	/*
		TaskManager : Custom Constructor - Set default pre-creation	singleton values
		Author: Mitchell Croft
		Created: 16/08/2016
		Modified: 16/08/2016

		param[in] pWorkers - A constant value for the number of workers that will be used
		                     by the Task Manager
	*/
	TaskManager::TaskManager(unsigned int pWorkers) :
		/*----------Workers----------*/
		mWorkerCount(pWorkers),
		mWorkers(nullptr)
	{}

	/*
		TaskManager : organiseTasks - Manage the active tasks and close finished jobs
		Author: Mitchell Croft
		Created: 16/08/2016
		Modified:
	*/
	void AsynchTasks::TaskManager::organiseTasks() {
		//Loop so long as the Task Manager is running
		while (mRunning.test_and_set()) {

		}
	}

	/*
		TaskManager : create - Initialise and setup the task manager
		Author: Mitchell Croft
		Created: 16/08/2016
		Modified: 16/08/2016

		param[in] pWorkers - The number of workers that the Task Manager is to create and use
		                     (Default 5)

		return bool - Returns true if the TaskManager was created successfully
	*/
	bool AsynchTasks::TaskManager::create(unsigned int pWorkers) {
		//Assert that the Task Manager doesn't already exist
		assert(!mInstance);

		//Assert that there is at least one worker created
		assert(pWorkers > 0);

		//Create the new Task Manager
		mInstance = new TaskManager(pWorkers);

		//Test to ensure the instance were created
		if (!mInstance) {
			printf("Unable to create the TaskManager singleton instance.");
			return false;
		}

		//Create the workers
		mInstance->mWorkers = new Worker[mInstance->mWorkerCount];

		//Test to ensure the workers were created
		if (!mInstance->mWorkers) {
			printf("Unable to create the Workers for the TaskManager");
			return false;
		}

		//Set the operating flag
		mInstance->mRunning.test_and_set();

		//Start the organisation thread
		mInstance->mOrganisationThread = std::thread([&]() {
			//Call the organisation thread
			mInstance->organiseTasks();
		});

		//Return creation was completed successfully
		return true;
	}

	/*
		TaskManager : update - Update the different tasks and complete main thread tasks
		Author: Mitchell Croft
		Created:
		Modified:
	*/
	void AsynchTasks::TaskManager::update() {}

	/*
		TaskManager : destroy - Close all remaining tasks and delete the TaskManager
		Author: Mitchell Croft
		Created: 16/08/2016
		Modified: 16/08/2016

		param[in] pForce - Flags if all tasks should be forced to exit (Default false)
	*/
	void AsynchTasks::TaskManager::destroy(bool pForce) {
		//Test if the singleton instance was created
		if (mInstance) {
			//Kill the organisation thread
			mInstance->mRunning.clear();

			//Join the organisation thread
			if (mInstance->mOrganisationThread.get_id() != std::thread::id()) 
				mInstance->mOrganisationThread.join();

			//Delete the Workers
			if (mInstance->mWorkers)
				delete[] mInstance->mWorkers;

			//Delete the singleton instance
			delete mInstance;
			mInstance = nullptr;
		}
	}
	#pragma endregion
	#pragma endregion
	#endif
}
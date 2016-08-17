#pragma once

#include <assert.h>

#include <functional>

#include <memory>

#include <thread>
#include <atomic>
#include <mutex>

#include <vector>

namespace AsynchTasks {

	#pragma region Forward Declare Task Classes
	//! Forward declare the base type for Tasks to inherit from
	class Asynch_Task_Base;

	//! Forward declare the template for different Tasks that can be completed
	template<class T> class Asynch_Task_Job;
	#pragma endregion

	#pragma region Type Defines
	//! Define the task ID number types for both configurations
	typedef unsigned long int		taskID_32;
	typedef unsigned long long int  taskID_64;

	//! Define the general Task ID based on the current configuration
	#ifndef _BUILD64
	typedef taskID_32 taskID;
	#else
	typedef taskID_64 taskID;
	#endif

	//! Create an alias for the different Task items that the user can receive
	template<class T> using Task = std::shared_ptr<Asynch_Task_Job<T>>;

	//! Label the different	states the task can be in
	enum class ETaskStatus {
		Error,
		Setup,
		Pending,
		Waiting,
		In_Progress,
		Completed
	};
	#pragma endregion

	#pragma region TaskManager
	/*
	 *		Name: TaskManager
	 *		Author: Mitchell Croft
	 *		Created: 16/08/2016
	 *		Modified: 17/08/2016
	 *
	 *		Purpose:
	 *		Complete tasks in a multi-threaded environment, while providing
	 *		options for order completion, priority and callbacks for the 
	 *		information.
	 *		
	 *		Manager makes use of a manager thread to maintain the different
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
		TaskManager(unsigned int pWorkers);
		~TaskManager() = default;

		TaskManager() = delete;
		TaskManager(const TaskManager&) = delete;
		TaskManager& operator=(const TaskManager&) = delete;

		/*----------Variables----------*/
		//! Keep as a constant the number of workers in use
		const unsigned int mWorkerCount;

		//! Maintain a pointer to the Workers
		Worker* mWorkers;

		//! Maintain a thread for organising the different tasks
		std::thread mOrganisationThread;

		//! Flag if the Task Manager is operating
		std::atomic_flag mRunning;

		//! Create a lock to prevent thread clashes over tasks
		std::mutex mTaskLock;

		//! Keep a vector of all the tasks to be completed
		std::vector<std::shared_ptr<Asynch_Task_Base>> mUncompletedTasks;

		/*----------Functions----------*/
		//! Organise tasks in a separate thread
		void organiseTasks();

	public:
		//! Main operation functionality
		static bool create(unsigned int pWorkers = 5u);
		static void update();
		static void destroy();
	};
	#pragma endregion

	#pragma region Property Objects
	/*
	 *		Name: ReadOnlyProperty
	 *		Author: Mitchell Croft
	 *		Created: 17/08/2016
	 *		Modified: 17/08/2016
	 *		
	 *		Purpose:
	 *		Provide a reference back to a value that cannot be modified
	 *		while allowing regular variable use.
	 *		
	 *		Requires:
	 *		Ensure that the property object is destroyed before or at the
	 *		same time as the value this property encapsulates
	**/
	template<class T>
	class ReadOnlyProperty {
		//! Maintain a constant reference to the value
		const T& mValue;

	public:
		//! Prevent default construction
		ReadOnlyProperty<T>() = delete;

		//! Prevent Copy Construction
		ReadOnlyProperty<T>(const ReadOnlyProperty<T>&) = delete;

		//! Allow for setup with T& value
		inline ReadOnlyProperty<T>(const T& pValue) : mValue(pValue) {}

		//! Provide an implicit method of getting value back
		inline operator const T&() const { return mValue; }

		//! Remove the copy operator
		ReadOnlyProperty<T>& operator=(const ReadOnlyProperty<T>&) = delete;
	};

	/*
	 *		Name: ReadWriteFlaggedProperty
	 *		Author: Mitchell Croft
	 *		Created: 17/08/2016
	 *		Modified: 17/08/2016
	 *		
	 *		Purpose:
	 *		Provide a reference back to a value that can be checked and
	 *		modified until a flag is raised to prevent modification
	 *		
	 *		Requires:
	 *		Ensure that the property object is destroyed before or at the
	 *		same time as the values this property encapsulates
	**/
	template<class T>
	class ReadWriteFlaggedProperty {
		//! Maintain a constant reference to the value
		T& mValue;

		//! Maintain a constance reference to the modification flag
		const bool& mFlag;

	public:
		//! Prevent default construction
		ReadWriteFlaggedProperty<T>() = delete;

		//! Prevent Copy Construction
		ReadWriteFlaggedProperty<T>(const ReadWriteFlaggedProperty<T>&) = delete;

		//! Allow for setup with a T& and bool& value
		inline ReadWriteFlaggedProperty<T>(T& pValue, const bool& pFlag) : mValue(pValue), mFlag(pFlag) {}

		//! Allow for the property to be set
		inline ReadWriteFlaggedProperty<T>& operator=(const T& pVal) {
			//Check if the flag has been raised
			if (!mFlag) mValue = pVal;

			//Return itself
			return *this;
		}

		//! Provide a implicit method of getting the value back
		inline operator const T&() const { return mValue; }
	};
	#pragma endregion

	#pragma region Task Objects
	/*
	 *		Name: Asynch_Task_Base
	 *		Author: Mitchell Croft
	 *		Created: 17/08/2016
	 *		Modified: 17/08/2016
	 *		
	 *		Purpose:
	 *		An abstract base class to allow for the creation and
	 *		storing of template Task Jobs that can be managed and
	 *		executed by the Task Manager. Stores a number of base 
	 *		values that are shared by all Tasks.
	**/
	class Asynch_Task_Base {
	protected:
		//! Store the ID of the current Task
		taskID mID;

		//! Store the current state of the Task
		ETaskStatus mStatus;

		//! Store a flag to indicate if the callback function should be called from the 'main' thread
		//! (Main being the thread the Task Manager was created on)
		bool mCallbackOnMain;

		//! Maintain a flag to indicated if values can be edited
		bool mLockValues;

		/*----------Functions----------*/
		Asynch_Task_Base();
		virtual ~Asynch_Task_Base() = default;
		Asynch_Task_Base(const Asynch_Task_Base&) = delete;
		Asynch_Task_Base& operator=(const Asynch_Task_Base&) = delete;

		//! Provide main functions used to complete threaded jobs
		virtual void completeProcess() = 0;
		virtual void completeCallback() = 0;
		
	public:
		//! Expose the ID and status values to the user	for reading
		ReadOnlyProperty<taskID> id;
		ReadOnlyProperty<ETaskStatus> status;

		//! Expose the execute callback on main flag
		ReadWriteFlaggedProperty<bool> callbackOnMain;
	};

	/*
	 *		Name: Asynch_Task_Job (General)
	 *		Author: Mitchell Croft
	 *		Created: 17/08/2016
	 *		Modified: 17/08/2016
	 *
	 *		Purpose:
	 *		Provide the Task item to allow for the completion of jobs 
	 *		in a multi threaded environment with a generic return 
	 *		type from the process.
	**/
	template<class T> 
	class Asynch_Task_Job : public Asynch_Task_Base {
		//! Set as a friend of the Task Manager to allow for construction and use
		friend class TaskManager;

		//! Store a pointer of type T to keep the return result in
		T* mResult;

		//! Store the function calls to process
		std::function<T()> mProcess;
		std::function<void(T&)> mCallback;

		/*----------Functions----------*/
		//! Restrict Job creation to the Task Manager
		Asynch_Task_Job<T>();

		//! Override the base class abstract functions
		void completeProcess()	override;
		void completeCallback()	override;

	public:
		//! Expose the destructor to allow for the shared pointers to delete used jobs
		~Asynch_Task_Job<T>() override;

		//! Expose the functions function calls as properties
		ReadWriteFlaggedProperty<std::function<T()>> process;
		ReadWriteFlaggedProperty<std::function<void(T&)>> callback;
	};

	#pragma region Asynch_Task_Job Function Defines
	/*
		Asynch_Task_Job<T> : Constructor - Initialise with default values
		Author: Mitchell Croft
		Created: 17/08/2016
		Modified: 17/08/2016
	*/
	template<class T>
	inline Asynch_Task_Job<T>::Asynch_Task_Job() :
		Asynch_Task_Base::Asynch_Task_Base(),
		mResult(nullptr),
		process(mProcess, Asynch_Task_Base::mLockValues),
		callback(mCallback, Asynch_Task_Base::mLockValues)
	{}

	/*
		Asynch_Task_Job<T> : completeProcess - Preform the threaded process
		Author: Mitchell Croft
		Created: 17/08/2016
		Modified: 17/08/2016
	*/
	template<class T>
	inline void Asynch_Task_Job<T>::completeProcess() {
		//Create a pointer to a new object of T and call the copy constructor on the return from the process
		mResult = new T(mProcess());
	}

	/*
		Asynch_Task_Job<T> : completeCallback - Preform the callback process
		Author: Mitchell Croft
		Created: 17/08/2016
		Modified: 17/08/2016
	*/
	template<class T>
	inline void Asynch_Task_Job<T>::completeCallback() {
		//Run the callback and pass a reference to the previous result in
		if (mCallback) mCallback(*mResult);
	}

	/*
		Asynch_Task_Job<T> : Destructor - Delete any memory used by the Task
		Author: Mitchell Croft
		Created: 17/08/2016
		Modified: 17/08/2016
	*/
	template<class T>
	inline Asynch_Task_Job<T>::~Asynch_Task_Job() {
		delete mResult;
	}
	#pragma endregion

	#pragma region Void Task Specilisation
	/*
	 *		Name: Asynch_Task_Job (void)
	 *		Author: Mitchell Croft
	 *		Created: 17/08/2016
	 *		Modified: 17/08/2016
	 *
	 *		Purpose:
	 *		Provide the Task item to allow for the completion of jobs
	 *		in a multi threaded environment with a void return
	 *		type from the process.
	**/
	template<>
	class Asynch_Task_Job<void> : public Asynch_Task_Base {
		//! Set as a friend of the Task Manager to allow for construction and use
		friend class TaskManager;

		//Store the function calls to process
		std::function<void()> mProcess;
		std::function<void()> mCallback;

		/*----------Functions----------*/
		//! Restrict Job creation to the Task Manager
		Asynch_Task_Job();

		//! Override the base class abstract functions
		virtual void completeProcess() override;
		virtual void completeCallback() override;

	public:
		//! Expose the destructor to allow for the shared pointers to delete used jobs
		~Asynch_Task_Job() override = default;

		//! Expose the functions function calls as properties
		ReadWriteFlaggedProperty<std::function<void()>> process;
		ReadWriteFlaggedProperty<std::function<void()>> callback;
	};

	/*
		Asynch_Task_Job<void> : Constructor - Initialise with default values
		Author: Mitchell Croft
		Created: 17/08/2016
		Modified: 17/08/2016
	*/
	inline Asynch_Task_Job<void>::Asynch_Task_Job() :
		Asynch_Task_Base::Asynch_Task_Base(),
		process(mProcess, Asynch_Task_Base::mLockValues),
		callback(mCallback, Asynch_Task_Base::mLockValues)
	{}

	/*
		Asynch_Task_Job<void> : completeProcess - Preform the threaded process
		Author: Mitchell Croft
		Created: 17/08/2016
		Modified: 17/08/2016
	*/
	inline void Asynch_Task_Job<void>::completeProcess() {
		//Call the process
		mProcess();
	}

	/*
		Asynch_Task_Job<void> : completeCallback - Preform the callback process
		Author: Mitchell Croft
		Created: 17/08/2016
		Modified: 17/08/2016
	*/
	inline void Asynch_Task_Job<void>::completeCallback() {
		//Call the callback
		if (mCallback) mCallback();
	}
	#pragma endregion
	#pragma endregion

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
	public:
		Worker() = default;
	};
}

/*

	Function Definitions:

	Include a define for _ASYNCHRONOUS_TASKS_ in a single .cpp file inside
	of the project to use the AsynchTasks namespace functionality

*/
#ifdef _ASYNCHRONOUS_TASKS_
//! Define static singleton instance
AsynchTasks::TaskManager* AsynchTasks::TaskManager::mInstance = nullptr;

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
AsynchTasks::TaskManager::TaskManager(unsigned int pWorkers) :
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
*/
void AsynchTasks::TaskManager::destroy() {
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

#pragma region Task Objects
/*
	Asynch_Task_Base : Constructor - Initialise the Task_Base values
	Author: Mitchell Croft
	Created: 17/08/2016
	Modified: 17/08/2016
*/
AsynchTasks::Asynch_Task_Base::Asynch_Task_Base() :
	mID(0),
	mStatus(AsynchTasks::ETaskStatus::Setup),
	mCallbackOnMain(false),
	mLockValues(false),
	id(mID),
	status(mStatus),
	callbackOnMain(mCallbackOnMain, mLockValues)
{}
#pragma endregion
#endif
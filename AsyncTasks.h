#pragma once

#include <assert.h>

#include <functional>

#include <algorithm>

#include <memory>

#include <thread>
#include <atomic>
#include <mutex>

#include <vector>
#include <string>

/*
 *      Namespace: AsynchTasks
 *      Author: Mitchell Croft
 *      Created: 20/08/2016
 *      Modified: 11/01/2017
 *      
 *      Purpose:
 *      Provide a method of preforming various different tasks 
 *      and functions in a multi-threaded environment, while 
 *      being able to provide a method for the user to organise 
 *      and manage different tasks.
**/
namespace AsynchTasks {

    #pragma region Property Objects
    /*
     *      Namespace: Properties
     *      Author: Mitchell Croft
     *      Created: 20/08/2016
     *      Modified: 10/08/2016
     *      
     *      Purpose:
     *      Abstract the Property objects from the main focus
     *      of the AsynchTasks namespace. They are not directly
     *      related to the purpose of the AsynchTasks namespace but
     *      provide useful benefits in the implementation.
     */
    namespace Properties {
        /*
        *       Name: ReadOnlyProperty
        *       Author: Mitchell Croft
        *       Created: 17/08/2016
        *       Modified: 17/08/2016
        *
        *       Purpose:
        *       Provide a reference back to a value that cannot be modified
        *       while allowing regular variable use.
        *
        *       Requires:
        *       Ensure that the property object is destroyed before or at the
        *       same time as the value this property encapsulates. Property
        *       type must implement comparative operations (== and !=)
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

            //! Create logical comparator operators
            inline bool operator==(const T& pVal) { return mValue == pVal; }
            inline bool operator!=(const T& pVal) { return mValue != pVal; }

            //! Remove the copy operator
            ReadOnlyProperty<T>& operator=(const ReadOnlyProperty<T>&) = delete;
        };

        /*
        *       Name: ReadWriteFlaggedProperty
        *       Author: Mitchell Croft
        *       Created: 17/08/2016
        *       Modified: 17/08/2016
        *
        *       Purpose:
        *       Provide a reference back to a value that can be checked and
        *       modified until a flag is raised to prevent modification
        *
        *       Requires:
        *       Ensure that the property object is destroyed before or at the
        *       same time as the values this property encapsulates.Property
        *       type must implement comparative operations (== and !=)
        **/
        template<class T>
        class ReadWriteFlaggedProperty {
            //! Maintain a constant reference to the value
            T& mValue;

            //! Maintain a constant reference to the modification flag
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

            //! Create logical comparator operators
            inline bool operator==(const T& pVal) { return mValue == pVal; }
            inline bool operator!=(const T& pVal) { return mValue != pVal; }
        };
    }
    #pragma endregion

    #pragma region Forward Declare Task Classes
    //! Forward declare the base type for Tasks to inherit from
    class Asynch_Task_Base;

    //! Forward declare the template for different Tasks that can be completed
    template<class T> class Asynch_Task_Job;
    #pragma endregion

    #pragma region Type Defines
    //! Define the task ID number 
    typedef unsigned long long int taskID;

    //! Create an alias for the different Task items that the user can receive
    template<class T> using Task = std::shared_ptr<Asynch_Task_Job<T>>;

    //! Label the different states the task can be in
    enum class ETaskStatus : char {
        //! An error occurred when trying to process the Task, check the Tasks error
        Error = -1,     

        //! The Task is in a stage where it is being setup for processing
        Setup,

        //! The Task has been added to the Task Manager and is waiting to be processed
        Pending,

        //! The Task is currently being processed
        In_Progress,

        //! The Task is waiting for the "update" function to call the callback
        Callback_On_Update,

        //! The Task has been completed
        Completed
    };

    //! Highlight the different priorities of the Task objects
    enum ETaskPriority : unsigned int {
        Low_Priority = 0x00000000,
        Medium_Priority = 0x7FFFFFFF,
        High_Priority = 0xFFFFFFFF
    };
    #pragma endregion

    #pragma region Task Manager Decleration
    /*
     *      Name: TaskManager
     *      Author: Mitchell Croft
     *      Created: 16/08/2016
     *      Modified: 11/01/2017
     *
     *      Purpose:
     *      Complete tasks in a multi-threaded environment, while providing
     *      options for order completion, priority and callbacks for the 
     *      information.
     *      
     *      Manager makes use of a manager thread to maintain the different
     *      tasks with a number of Worker threads that complete the tasks.
     *      The number of these workers are defined when creating the 
     *      singleton instance.
    **/
    class TaskManager {
        //! Prototype the Worker class as a private object
        class Worker;

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

        //! Store the values dictating the Worker threads sleeping behavior
        unsigned int mWorkerInactiveTimeout;    //Milliseconds
        unsigned int mWorkerSleepLength;        //Milliseconds

        //! Maintain a pointer to the Workers
        Worker* mWorkers;

        //! Maintain a thread for organising the different tasks
        std::thread mOrganisationThread;

        //! Flag if the Task Manager is operating
        std::atomic_flag mRunning;

        //! Create a lock to prevent thread clashes over tasks
        std::mutex mTaskLock;

        //! Track the current ID to distribute to new Tasks
        taskID mNextID;

        //! Store the maximum number of Tasks that can have their callbacks executed on update per call
        unsigned int mMaxCallbacksOnUpdate;

        //! Keep a vector of all the Tasks to be completed
        std::vector<std::shared_ptr<Asynch_Task_Base>> mUncompletedTasks;

        //! Keep a vector of all the Tasks to have their callback called on an update call
        std::vector<std::shared_ptr<Asynch_Task_Base>> mToCallOnUpdate;

        /*----------Functions----------*/
        //! Organise tasks in a separate thread
        void organiseTasks();

    public:
        //! Main operation functionality
        static bool create(unsigned int pWorkers = 5u);
        static void update();
        static void destroy();

        //! Task options
        template<class T> static Task<T> createTask();
        template<class T> static bool addTask(Task<T>& pTask);

        /*----------Setters----------*/
        static inline void setWorkerTimeout(unsigned int pTime);
        static inline void setWorkerSleep(unsigned int pTime);
        static inline void setMaxCallbacks(unsigned int pMax);
    };
    #pragma endregion

    #pragma region Task Objects
    /*
     *      Name: Asynch_Task_Base
     *      Author: Mitchell Croft
     *      Created: 17/08/2016
     *      Modified: 10/01/2017
     *      
     *      Purpose:
     *      An abstract base class to allow for the creation and
     *      storing of template Task Jobs that can be managed and
     *      executed by the Task Manager. Stores a number of base 
     *      values that are shared by all Tasks.
    **/
    class Asynch_Task_Base {
    protected:
        //! Set as a friend of the Task Manager to allow for construction and use
        friend class TaskManager;

        //! Set as friend of the Worker to allow for use
        friend class Worker;

        //! Store the ID of the current Task
        taskID mID;

        //! Store the current state of the Task
        ETaskStatus mStatus;

        //! Store the priority of the Task
        ETaskPriority mPriority;

        //! Store a flag to indicate if the callback function should be called from the TaskManager's update function call
        bool mCallbackOnUpdate;

        //! Maintain a flag to indicated if values can be edited
        bool mLockValues;

        //! String used to contain and display error messages to the user
        std::string mErrorMsg;

        /*----------Functions----------*/
        Asynch_Task_Base();
        virtual ~Asynch_Task_Base() = default;
        Asynch_Task_Base(const Asynch_Task_Base&) = delete;
        Asynch_Task_Base& operator=(const Asynch_Task_Base&) = delete;

        //! Provide main functions used to complete threaded jobs
        virtual void completeProcess() = 0;
        virtual void completeCallback() = 0;
        virtual void cleanupData() = 0;
        
    public:
        //! Expose the ID and status values to the user for reading
        Properties::ReadOnlyProperty<taskID> id;
        Properties::ReadOnlyProperty<ETaskStatus> status;

        //! Expose the priority value to the user
        Properties::ReadWriteFlaggedProperty<ETaskPriority> priority;

        //! Expose the execute callback on update flag
        Properties::ReadWriteFlaggedProperty<bool> callbackOnUpdate;

        //! Expose the error string to the user for reading
        Properties::ReadOnlyProperty<std::string> error;
    };

    /*
     *      Name: Asynch_Task_Job (General)
     *      Author: Mitchell Croft
     *      Created: 17/08/2016
     *      Modified: 11/01/2017
     *
     *      Purpose:
     *      Provide the Task item to allow for the completion of jobs 
     *      in a multi threaded environment with a generic return 
     *      type from the process.
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
        void completeProcess() override;
        void completeCallback() override;
        void cleanupData() override;

    public:
        //! Expose the destructor to allow for the shared pointers to delete used jobs
        ~Asynch_Task_Job<T>() override;

        //! Expose the functions function calls as properties
        Properties::ReadWriteFlaggedProperty<std::function<T()>> process;
        Properties::ReadWriteFlaggedProperty<std::function<void(T&)>> callback;
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
        Asynch_Task_Job<T> : cleanupData - Cleanup memory allocated by the Task when completing the process
        Author: Mitchell Croft
        Created: 19/08/2016
        Modified: 10/01/2017
    */
    template<class T>
    inline void AsynchTasks::Asynch_Task_Job<T>::cleanupData() {
        //Check if the data has been set
        if (mResult) delete mResult;

        //Reset the pointer
        mResult = nullptr;
    }

    /*
        Asynch_Task_Job<T> : Destructor - Delete any memory used by the Task
        Author: Mitchell Croft
        Created: 17/08/2016
        Modified: 24/08/2016
    */
    template<class T>
    inline Asynch_Task_Job<T>::~Asynch_Task_Job() { cleanupData(); }
    #pragma endregion

    #pragma region Void Task Specilisation
    /*
     *      Name: Asynch_Task_Job (void)
     *      Author: Mitchell Croft
     *      Created: 17/08/2016
     *      Modified: 20/08/2016
     *
     *      Purpose:
     *      Provide the Task item to allow for the completion of jobs
     *      in a multi threaded environment with a void return
     *      type from the process.
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
        void completeProcess() override;
        void completeCallback() override;
        void cleanupData() override;

    public:
        //! Expose the destructor to allow for the shared pointers to delete used jobs
        ~Asynch_Task_Job() override = default;

        //! Expose the functions function calls as properties
        Properties::ReadWriteFlaggedProperty<std::function<void()>> process;
        Properties::ReadWriteFlaggedProperty<std::function<void()>> callback;
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

    /*
        Asynch_Task_Job<void> : cleanupData - Empty function as void Tasks allocate no memory
        Author: Mitchell Croft
        Created: 19/08/2016
        Modified: 19/08/2016
    */
    inline void Asynch_Task_Job<void>::cleanupData() {}
    #pragma endregion
    #pragma endregion

    #pragma region Worker Definition
    /*
     *      Name: Worker
     *      Author: Mitchell Croft
     *      Created: 18/08/2016
     *      Modified: 20/08/2016
     *
     *      Purpose:
     *      Execute the Tasks provided to it by the Task Manager
    **/
    class TaskManager::Worker {
        /*----------Variables----------*/
        //! Flags if the processing thread is running
        std::atomic_flag mRunning;

        //! The thread that is running the Task processes
        std::thread mProcessingThread;

        //! Used to indicate how many seconds of inactivity must pass before the Worker goes to sleep
        Properties::ReadOnlyProperty<unsigned int> mInactiveTimeout;

        //! Used to indicate how long the processing thread should sleep when inactive
        Properties::ReadOnlyProperty<unsigned int> mSleepLength;

        /*----------Functions----------*/

        //! Function run on the Worker Thread to process Tasks given to it
        void doWork();

    public:
        /*----------Variables----------*/
        //! Used to store an active Task to complete
        std::shared_ptr<Asynch_Task_Base> task;

        //! Used to flag when the Worker has finished their Task and protect modification clashes
        std::mutex taskLock;

        /*----------Functions----------*/
        Worker();
        ~Worker();
    };
    #pragma endregion

    #pragma region Task Manager Templated Definitions
    /*
        TaskManager : createTask - Return a new Task object with a return type of T
        Author: Mitchell Croft
        Created: 18/08/2016
        Modified: 18/08/2016

        return Task<T> - Returns a Task<T> shared pointer to a new Task object
    */
    template<class T>
    inline Task<T> TaskManager::createTask() {
        //Create a new Task
        Task<T> newTask = Task<T>(new Asynch_Task_Job<T>());

        //ID stamp the new task
        newTask->mID = ++mInstance->mNextID;

        //Return the task
        return newTask;
    }

    /*
        TaskManager : addTask - Add a new Task to the Task Manager for processing
        Author: Mitchell Croft
        Created: 18/08/2016
        Modified: 18/08/2016

        Note:
        Priority of the Task does not ensure execution before lower priority tasks.
        If a lower priority Task was added before the higher priority Task it is 
        possible for the lower priority Task to begin processing before the higher
        priority Task is added to the Task Manager.

        param[in/out] pTask - A Task<T> object to be added to the list. Once added to the
                              Task Manager the property values will be uneditable.

        return bool - Returns a flag determining if the Task was added to the manager
                      successfully.
    */
    template<class T>
    inline bool TaskManager::addTask(Task<T>& pTask) {
        //Ensure that the pointer is valid
        if (!pTask) return false;

        //Ensure that the task is in the setup or complete state
        switch (pTask->mStatus) {
        case ETaskStatus::Setup:
        case ETaskStatus::Completed:
            break;
        default: return false;
        }

        //Ensure that the task has at minimum a process functions set
        if (!pTask->mProcess) return false;

        //Lock down the tasks values
        pTask->mLockValues = true;

        //Change the state to indicate pending processing
        pTask->mStatus = ETaskStatus::Pending;

        //Lock the Task list
        mInstance->mTaskLock.lock();

        //Add the task to the vector
        mInstance->mUncompletedTasks.push_back(pTask);

        //Sort the list based on the priority of the Tasks
        std::sort(mInstance->mUncompletedTasks.begin(),
            mInstance->mUncompletedTasks.end(),
            [&](const std::shared_ptr<Asynch_Task_Base>& pFirst, const std::shared_ptr<Asynch_Task_Base>& pSecond) {
            return pFirst->mPriority > pSecond->mPriority;
        });

        //Unlock the task list
        mInstance->mTaskLock.unlock();

        //Return success
        return true;
    }

    /*
        TaskManager : setWorkerTimeout - Set the time each Worker waits for work before 
                                         going to sleep
        Author: Mitchell Croft
        Created: 18/08/2016
        Modified: 11/01/2017

        param[in] pTime - The amount of time (in milliseconds) to wait
    */
    inline void TaskManager::setWorkerTimeout(unsigned int pTime) {
        mInstance->mWorkerInactiveTimeout = pTime;
    }

    /*
        TaskManager : setWorkerSleep - Set the time each worker is asleep for in between 
                                       checking for new Tasks
        Author: Mitchell Croft
        Created: 18/08/2016
        Modified: 18/08/2016

        param[in] pTime - The amount of time (in milliseconds) to sleep
    */
    inline void TaskManager::setWorkerSleep(unsigned int pTime) {
        mInstance->mWorkerSleepLength = pTime;
    }

    /*
        TaskManager : setMaxCallbacks - Set the maximum number of callbacks that can
                                        occur every time TaskManager::update is called
        Author: Mitchell Croft
        Created: 18/08/2016
        Modified: 10/01/2017

        param[in] pMax - The maximum number of callbacks that can be executed
    */
    inline void TaskManager::setMaxCallbacks(unsigned int pMax) {
        mInstance->mMaxCallbacksOnUpdate = pMax;
    }
    #pragma endregion
}

/*

    Function Definitions:

    Include a define for _ASYNCHRONOUS_TASKS_ in a single .cpp file inside
    of the project to use the AsynchTasks namespace functionality

*/
#ifdef _ASYNCHRONOUS_TASKS_
//! Define static singleton instance
AsynchTasks::TaskManager* AsynchTasks::TaskManager::mInstance = nullptr;

#pragma region Task Manager Function Definitions
/*
    TaskManager : Custom Constructor - Set default pre-creation singleton values
    Author: Mitchell Croft
    Created: 16/08/2016
    Modified: 18/08/2016

    param[in] pWorkers - A constant value for the number of workers that will be used
                         by the Task Manager
*/
AsynchTasks::TaskManager::TaskManager(unsigned int pWorkers) :
    /*----------Workers----------*/
    mWorkerCount(pWorkers),
    mWorkers(nullptr),
    mWorkerInactiveTimeout(2000),
    mWorkerSleepLength(100),

    /*----------Tasks----------*/
    mMaxCallbacksOnUpdate(10),
    mNextID(0)
{}

/*
    TaskManager : organiseTasks - Manage the active tasks and close finished jobs
    Author: Mitchell Croft
    Created: 16/08/2016
    Modified: 10/01/2017
*/
void AsynchTasks::TaskManager::organiseTasks() {
    //Loop so long as the Task Manager is running
    while (mRunning.test_and_set()) {
        //Lock the data
        mTaskLock.lock();

        //Loop through all of the workers and check their progress
        for (unsigned int i = 0; i < mWorkerCount; i++) {
            //Lock the workers Task
            if (mWorkers[i].taskLock.try_lock()) {

                //Check if the worker has a task
                if (mWorkers[i].task) {
                    //Check if the task is finished
                    switch (mWorkers[i].task->mStatus) {
                    case ETaskStatus::Callback_On_Update:
                        //Add the Task to the on update callback vector
                        mToCallOnUpdate.push_back(mWorkers[i].task);

                        //Sort the vector based on priority
                        std::sort(mToCallOnUpdate.begin(), mToCallOnUpdate.end(),
                            [&](const std::shared_ptr<Asynch_Task_Base>& pFirst, const std::shared_ptr<Asynch_Task_Base>& pSecond) {
                            return pFirst->mPriority < pSecond->mPriority;
                        });
                    case ETaskStatus::Error:
                    case ETaskStatus::Completed:
                        //Clear the Workers Task
                        mWorkers[i].task = nullptr;
                        break;
                    }
                }

                //Check if there are any Tasks to handout and Worker isn't busy
                if (mUncompletedTasks.size() && !mWorkers[i].task) {
                    //Give the Worker the next Task
                    mWorkers[i].task = mUncompletedTasks[0];

                    //Clear that task from the uncompleted list
                    mUncompletedTasks.erase(mUncompletedTasks.begin());
                }

                //Unlock the data
                mWorkers[i].taskLock.unlock();
            }
        }

        //Unlock the data
        mTaskLock.unlock();
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
    assert(pWorkers);

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
    TaskManager : update - Update the different tasks and complete on update callbacks
    
    Requires:
    This function will call all of the Tasks that have callbacks on the main thread. For
    this to work properly you must only call this function from the main thread.

    Author: Mitchell Croft
    Created: 18/08/2016
    Modified: 10/01/2017
*/
void AsynchTasks::TaskManager::update() {
    //Lock the Tasks
    mInstance->mTaskLock.lock();

    //Check if there are any Tasks to complete
    if (mInstance->mToCallOnUpdate.size()) {
        //Loop through the Tasks that need executing
        for (int i = (int)mInstance->mToCallOnUpdate.size() - 1, count = 0;
             i >= 0 && (unsigned int)count < mInstance->mMaxCallbacksOnUpdate; 
             i--, count++) {

            //Get a reference to the task
            std::shared_ptr<Asynch_Task_Base>& task = mInstance->mToCallOnUpdate[i];

            //Try to execute the Task 
            try {
                //Run the callback process
                task->completeCallback();

                //Flag the Task as completed
                task->mStatus = ETaskStatus::Completed;

                //Allow editing of Task values
                task->mLockValues = false;
            }

            //If an error occurs, store the error message inside of the Task
            catch (const std::exception& pExc) {
                //Store the message
                task->mErrorMsg = pExc.what();

                //Flag the Task with an error flag
                task->mStatus = ETaskStatus::Error;

                //Allow editing of Task values
                task->mLockValues = false;
            } catch (const std::string& pExc) {
                //Store the message
                task->mErrorMsg = pExc;

                //Flag the Task with an error flag
                task->mStatus = ETaskStatus::Error;

                //Allow editing of Task values
                task->mLockValues = false;
            } catch (...) {
                //Store generic message
                task->mErrorMsg = "An unknown error occurred while executing the Task. Error thrown did not provide any information as to the cause\n";

                //Flag the Task with an error flag
                task->mStatus = ETaskStatus::Error;

                //Allow editing of Task values
                task->mLockValues = false;
            }

            //Clear Task's allocated memory
            task->cleanupData();

            //Remove the task from the list
            mInstance->mToCallOnUpdate.erase(mInstance->mToCallOnUpdate.begin() + i);
        }
    }

    //Unlock the Tasks
    mInstance->mTaskLock.unlock();
}

/*
    TaskManager : destroy - Close all threads and delete the TaskManager
    Author: Mitchell Croft
    Created: 16/08/2016
    Modified: 18/08/2016
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

#pragma region Task Objects Function Definition
/*
    Asynch_Task_Base : Constructor - Initialise the Task_Base values
    Author: Mitchell Croft
    Created: 17/08/2016
    Modified: 17/08/2016
*/
AsynchTasks::Asynch_Task_Base::Asynch_Task_Base() :
    mID(0),
    mStatus(AsynchTasks::ETaskStatus::Setup),
    mPriority(AsynchTasks::Low_Priority),
    mCallbackOnUpdate(false),
    mLockValues(false),
    id(mID),
    status(mStatus),
    priority(mPriority, mLockValues),
    callbackOnUpdate(mCallbackOnUpdate, mLockValues),
    error(mErrorMsg)
{}
#pragma endregion

#pragma region Worker Object Function Definitions
/*
    TaskManager::Worker : doWork - Complete the Task objects assigned by the 
                                   Task Manager
    Author: Mitchell Croft
    Created: 18/08/2016
    Modified: 19/08/2016
*/
void AsynchTasks::TaskManager::Worker::doWork() {
    //Track the period in time where the Worker will sleep
    auto workerSleepPoint = std::chrono::system_clock::now() + std::chrono::milliseconds(mInactiveTimeout);

    //Loop while the thread is running
    while (mRunning.test_and_set()) {
        //Lock the Task
        taskLock.lock();

        //Check if there is a job to do
        if (!task || (task && task->mStatus != ETaskStatus::Pending)) {
            //Get the current time 
            auto currentTime = std::chrono::system_clock::now();

            //Calculate the difference between the time points
            auto difference = std::chrono::duration_cast<std::chrono::milliseconds>(workerSleepPoint - currentTime);

            //Check to see if the Worker is still awake
            if (difference.count() > 0) {
                //Unlock the Task
                taskLock.unlock();

                //Yield to other threads
                std::this_thread::yield();
            }

            //If the worker is sleeping, sleep for that period of time
            else {
                //Unlock the Task
                taskLock.unlock();
            
                //Sleep the thread
                std::this_thread::sleep_for(std::chrono::milliseconds(mSleepLength));
            }

            //Continue executing the loop
            continue;
        }

        //Set the new sleep time
        workerSleepPoint = std::chrono::system_clock::now() + std::chrono::milliseconds(mInactiveTimeout);

        //Try to execute the Task 
        try {
            //Update the tasks current state
            task->mStatus = ETaskStatus::In_Progress;

            //Run the process
            task->completeProcess();

            //Check if the callback doesn't need to be run on main
            if (!task->mCallbackOnUpdate) {
                //Run the callback process
                task->completeCallback();

                //Flag the Task as completed
                task->mStatus = ETaskStatus::Completed;

                //Allow editing of Task values
                task->mLockValues = false;

                //Clear Tasks allocated memory
                task->cleanupData();
            }

            //Otherwise flag the Task as needing to be called in main
            else task->mStatus = ETaskStatus::Callback_On_Update;
        } 
        
        //If an error occurs, store the error message inside of the Task
        catch (const std::exception& pExc) {
            //Store the message
            task->mErrorMsg = pExc.what();

            //Flag the Task with an error flag
            task->mStatus = ETaskStatus::Error;

            //Allow editing of Task values
            task->mLockValues = false;
        } catch (const std::string& pExc) {
            //Store the message
            task->mErrorMsg = pExc;

            //Flag the Task with an error flag
            task->mStatus = ETaskStatus::Error;

            //Allow editing of Task values
            task->mLockValues = false;
        } catch (...) {
            //Store generic message
            task->mErrorMsg = "An unknown error occurred while executing the Task. Error thrown did not provide any information as to the cause\n";

            //Flag the Task with an error flag
            task->mStatus = ETaskStatus::Error;

            //Allow editing of Task values
            task->mLockValues = false;
        }

        //Unlock the Task
        taskLock.unlock();
    }
}

/*
    TaskManager::Worker : Constructor - Initialise with default values and start
                                        the Worker thread
    Author: Mitchell Croft
    Created: 18/08/2016
    Modified: 18/08/2016
*/
inline AsynchTasks::TaskManager::Worker::Worker() :
    mInactiveTimeout(AsynchTasks::TaskManager::mInstance->mWorkerInactiveTimeout),
    mSleepLength(AsynchTasks::TaskManager::mInstance->mWorkerSleepLength),
    task(nullptr) {
    mProcessingThread = std::thread([&]() {
        //Set the running flag
        mRunning.test_and_set();

        //Start the processing function
        doWork();
    });
}

/*
    TaskManager::Worker : Destructor - Join the Worker thread 
    Author: Mitchell Croft
    Created: 18/08/2016
    Modified: 18/08/2016
*/
inline AsynchTasks::TaskManager::Worker::~Worker() {
    //Clear the running flag
    mRunning.clear();

    //Join the worker thread
    if (this->mProcessingThread.get_id() != std::thread::id())
        mProcessingThread.join();
}
#pragma endregion
#endif
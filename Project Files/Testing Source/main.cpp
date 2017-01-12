#include "../../AsyncTasks.h"

#include "BasicInput.h"
#include "Random.h"

/*
    statusToName - Convert a AsynchTask status value into a printable string
    Author: Mitchell Croft
    Created: 10/01/2017
    Modified: 10/01/2017

    param[in] pStatus - The AsynchTasks::ETaskStatus to convert to a string

    return const char* - Returns a c-string describing the status
*/
inline const char* statusToName(const AsynchTasks::ETaskStatus& pStatus) {
    switch (pStatus) {
    case AsynchTasks::ETaskStatus::Error: return "ERROR";
    case AsynchTasks::ETaskStatus::Setup: return "SETUP";
    case AsynchTasks::ETaskStatus::Pending: return "PENDING";
    case AsynchTasks::ETaskStatus::In_Progress: return "IN_PROGRESS";
    case AsynchTasks::ETaskStatus::Callback_On_Update: return "CALLBACK_ON_UPDATE";
    case AsynchTasks::ETaskStatus::Completed: return "COMPLETE";
    default: return "Unknown Status";
    }
}

/*
    normalisingVectors - Normalise a large number of randomly sized vectors to test speed
    Author: Mitchell Croft
    Created: 24/08/2016
    Modified: 12/01/2017
*/
void normalisingVectors() {
    //Define basic Vec3 struct
    struct Vec3 { float x, y, z; };

    //Store the number of worker threads to create
    unsigned int threadCount;

    //Loop until valid input
    do {
        //Clear the screen
        system("CLS");

        //Display prompt to the user
        getInput(threadCount, "Enter the number of Worker threads to create (32 maximum): ");
    } while (threadCount > 32);

    //Add some space on screen
    printf("\n\n\n");

    //Create the Task Manager
    if (AsynchTasks::TaskManager::create(threadCount)) {
        //Create the basic input manager
        BasicInput input = BasicInput(VK_ESCAPE, VK_SPACE);

        //Display initial instructions
        printf("Hold 'SPACE' to add a new Task to normalise 3,000,000 Vector 3 objects (Multiple Task Test)\n\n");

        //Loop until the input breaks the loop
        while (!input.keyPressed(VK_ESCAPE)) {
            //Update the input
            input.update();

            //Update the Task Manager
            AsynchTasks::TaskManager::update();

            //Output a single '.'
            printf(".");

            //Determine if space was pressed
            if (input.keyDown(VK_SPACE)) {
                //Create a new Task
                AsynchTasks::Task<std::pair<unsigned int, float>> newTask = AsynchTasks::TaskManager::createTask<std::pair<unsigned int, float>>();

                //Force callback to print on main
                newTask->callbackOnUpdate = true;

                //Set the functions
                newTask->process = [&]() -> std::pair<unsigned int, float> {
                    //Define the number of vectors to create
                    const unsigned int ARRAY_SIZE = 3000000;

                    //Create an array of 1 mill vectors
                    Vec3* vectors = new Vec3[ARRAY_SIZE];

                    //Store the number of vectors that were successfully normalised
                    unsigned int totalNormal = 0;
                    float averageNonNormal = 0.f;
                    
                    //Give the vectors random values
                    for (unsigned int i = 0; i < ARRAY_SIZE; i++) {
                        //Assign random axis values
                        vectors[i].x = randomRange(-500.f, 500.f);
                        vectors[i].y = randomRange(-500.f, 500.f);
                        vectors[i].z = randomRange(-500.f, 500.f);

                        //Get the magnitude of the vector
                        float mag = sqrtf(SQU(vectors[i].x) + SQU(vectors[i].y) + SQU(vectors[i].z));

                        //Check the magnitude is valid
                        if (mag) {
                            //Divide along the axis
                            vectors[i].x /= mag;
                            vectors[i].y /= mag;
                            vectors[i].z /= mag;
                        }

                        //Check if the magnitude has been normalised
                        mag = sqrtf(SQU(vectors[i].x) + SQU(vectors[i].y) + SQU(vectors[i].z));
                        if (mag == 0.f || mag == 1.f)
                            totalNormal++;
                        else averageNonNormal += mag;
                    }

                    //Delete the vector objects
                    delete[] vectors;

                    //Return the final count
                    return std::pair<unsigned int, float>(totalNormal, averageNonNormal /= (float)(ARRAY_SIZE - totalNormal));
                };
                newTask->callback = [&](std::pair<unsigned int, float>& pVal) {
                    printf("\nNormalized %u of 3,000,000 Vector 3 objects. The average non-normalised magnitude was %f\n", pVal.first, pVal.second);
                };

                //Add the task to the Task Manager
                if (AsynchTasks::TaskManager::addTask(newTask))
                    printf("\n\nAdded new Task to the Manager. Processing...\n");
                else printf("\n\nFailed to add the new task the Manager.\n");
            }

            //Slow main down to viewable pace
            Sleep(100);
        }
    }

    //Display error message
    else printf("Failed to create the Asynchronous Task Manager\n");

    //Destroy the the Task Manager
    AsynchTasks::TaskManager::destroy();
}

/*
    reusableTask - Test reusing Task objects for similar tasks repeatedly
    Author: Mitchell Croft
    Created: 24/08/2016
    Modified: 12/01/2017
*/
void reusableTask() {
    //Clear the screen
    system("CLS");

    //Create the Task Manager
    if (AsynchTasks::TaskManager::create(1)) {
        //Create the basic input manager
        BasicInput input = BasicInput(VK_ESCAPE, VK_SPACE);

        //Create the base job
        AsynchTasks::Task<unsigned int> stringTask = AsynchTasks::TaskManager::createTask<unsigned int>();

        //Set the callback to occur on main
        stringTask->callbackOnUpdate = true;

        //Set the process function
        stringTask->process = [&]() -> unsigned int {
            //Create a random character string
            unsigned int sum = 0;

            //Loop through and add the totals from 0 to UINT_MAX
            for (unsigned int i = 0; i < UINT_MAX; i++)
                sum++;

            //Return the sum
            return sum;
        };

        //Attach the callback function
        stringTask->callback = [&](unsigned int& pNum) {
            printf("\n\nCounted to: %u\n\n", pNum);
        };

        //Display initial instructions
        printf("Press 'SPACE' to count to UINT_MAX when Task object is available (Reusable Task Test)\n\n");

        //Loop till escape is pressed
        while (!input.keyPressed(VK_ESCAPE)) {
            //Update the input
            input.update();

            //Update the Task Manager
            AsynchTasks::TaskManager::update();

            //Output a single '.'
            if (stringTask->status != AsynchTasks::ETaskStatus::Completed && stringTask->status != AsynchTasks::ETaskStatus::Setup)
                printf(".");

            //Determine if the user has pressed space
            if (input.keyPressed(VK_SPACE)) {
                //Add the task to the Task Manager
                if (AsynchTasks::TaskManager::addTask(stringTask))
                    printf("Starting to count now:\n");
            }

            //Slow main down to viewable pace
            Sleep(100);
        }
    }

    //Display error message
    else printf("Failed to create the Asynchronous Task Manager\n");

    //Destroy the the Task Manager
    AsynchTasks::TaskManager::destroy();
}

/*
    errorReporting - Test task error reporting to the user upon exception thrown
    Author: Mitchell Croft
    Created: 12/01/2017
    Modified: 12/01/2017
*/
void errorReporting() {
    //Create the Task Manager
    if (AsynchTasks::TaskManager::create(1)) {
        //Store a user string
        char usrError[256];

        //Get a custom error message from the user
        do {
            //Clear the screen
            system("CLS");

            //Get the message from the user
            getInput(usrError, "Enter an error message to be reported on error thrown (Max 256 characters): ");
        } while (!strlen(usrError));

        //Add some space on screen
        printf("\n\n\n");

        //Create the basic input manager
        BasicInput input = BasicInput(VK_ESCAPE, VK_SPACE);

        //Store a flag for describing the position the error should be called
        bool errorFlag = false;

        //Setup the basic task to throw the error after a period of time
        AsynchTasks::Task<void> errorTask = AsynchTasks::TaskManager::createTask<void>();

        //Setup the error task with its values
        errorTask->callbackOnUpdate = false;
        errorTask->process = [&]() {
            //Sleep the thread for a period of time
            std::this_thread::sleep_for(std::chrono::milliseconds(randomRange(2000U, 5000U)));

            //Test to see if the error should be thrown here
            if (!errorFlag) {
                //Toggle the flag
                errorFlag = true;

                //Throw the users error
                throw std::runtime_error(std::string("Task Process function threw the error: ") + usrError);
            }
        };
        errorTask->callback = [&]() {
            //Toggle the flag
            errorFlag = false;

            //Throw the users error
            throw std::runtime_error(std::string("Task Callback function threw the error: ") + usrError);
        };

        //Store the previous status of the Task
        AsynchTasks::ETaskStatus prevStatus = AsynchTasks::ETaskStatus::Error;

        //Output the starting message
        printf("Press 'SPACE' to start the Task. Task progress and error output will be displayed\n\n");

        //Loop till escape is pressed
        while (!input.keyPressed(VK_ESCAPE)) {
            //Update the Input
            input.update();

            //Update the Task Manager
            AsynchTasks::TaskManager::update();

            //Check the Task status
            if (errorTask->status != prevStatus) {
                //Set the new status value
                prevStatus = errorTask->status;

                //Output the new status of the Task
                printf("Task Status: %s (%i)\n", statusToName(prevStatus), prevStatus);

                //If the task has failed
                if (prevStatus == AsynchTasks::ETaskStatus::Error) {
                    //Output the error message
                    printf("%s\n\n\n", errorTask->error.value().c_str());

                    //Force the task to reset itself
                    errorTask->callbackOnUpdate = (bool)errorTask->callbackOnUpdate;
                }
            }

            //Check to see if the user wants to add the Task to the manager
            if (input.keyPressed(VK_SPACE) && (errorTask->status == AsynchTasks::ETaskStatus::Setup || errorTask->status == AsynchTasks::ETaskStatus::Error))
                AsynchTasks::TaskManager::addTask(errorTask);
        }
    }

    //Display error message
    else printf("Failed to create the Asynchronous Task Manager\n");

    //Destroy the the Task Manager
    AsynchTasks::TaskManager::destroy();
}

/*
    main - Test the current implementation of the AsynchTasks namespace
    Author: Mitchell Croft
    Created: 18/08/2016
    Modified: 12/01/2017
*/
int main() {
    //Create a simple struct to describe possible tests
    struct ExecutableTest { const char* label; void(*const functionPtr)(); };

    //Create an array of the possible Executable Tests
    const ExecutableTest POSSIBLE_TESTS[] = {
        {"Normalising Vectors", normalisingVectors},
        {"Reusable Task", reusableTask},
        {"Error Reporting", errorReporting}
    };

    //Store the number of possible tests to select from
    const unsigned int TEST_COUNT = sizeof(POSSIBLE_TESTS) / sizeof(ExecutableTest);

    //Store the user input
    char usrChoice;

    //Loop so the user can choose the different tests
    do {
        //Clear the screen
        system("CLS");

        //Display the options
        printf("Implemented Tests (%u):\n", TEST_COUNT);
        for (unsigned int i = 0; i < TEST_COUNT; i++)
            printf("%i. %s\n", i + 1, POSSIBLE_TESTS[i].label);

        //Receive input selection from the user
        getInput(usrChoice, "\nEnter the desired test (Invalid character to quit): ");  

        //Adjust for character/digit offset
        usrChoice -= '1';

        //Check the selection is within range
        if (usrChoice >= 0 && usrChoice < TEST_COUNT) {
            //Call the function
            POSSIBLE_TESTS[usrChoice].functionPtr();

            //Allow the user to see the final output
            printf("\n\n\n\n\n");
            system("PAUSE");
        }

        //Otherwise exit the program
        else break;
    } while (true);
    return EXIT_SUCCESS;
}
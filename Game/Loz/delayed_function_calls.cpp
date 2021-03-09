#include <stdlib.h>
#include <string.h>
#include <queue>
#include <pthread.h>
#include <mutex>
#include "delayed_function_calls.hpp"
#include "gameLoop.hpp"
#include "getGameLoop.hpp"
#include "variableWrapper.hpp"
#include "variableWrapperDeclaration.hpp"

typedef struct
{
    double number;
    char*  text;
    int    type;
} delayedInput;

typedef struct delayedFunctionCall
{
    delayedInput                input[14];
    int                         function;
    double*                     delayedOutput;
    int                         hasOutput;
} delayedFunctionCall;

std::queue<delayedFunctionCall> functionQueue;
std::mutex functionQueueMutex;

GMEXPORT double removeDelayedFunctionCall()
{
    functionQueueMutex.lock();
    if (!functionQueue.empty())
    {
        delayedFunctionCall temp = functionQueue.front();
        functionQueue.pop();
        for (int i = 0; i < 14; i++)
        {
            if (temp.input[i].type == 1)
                free(temp.input[i].text);
        }
    }
    functionQueueMutex.unlock();
    return 1.0;
}

GMEXPORT char* getInputText(double input)
{
    functionQueueMutex.lock();
    char* returnValue = functionQueue.front().input[(int)input].text;
    functionQueueMutex.unlock();
    return returnValue;
}

GMEXPORT double getInputNumber(double input)
{
    functionQueueMutex.lock();
    double returnValue = functionQueue.front().input[(int)input].number;
    functionQueueMutex.unlock();
    return returnValue;
}

GMEXPORT double getInputType(double input)
{
    functionQueueMutex.lock();
    double returnValue = functionQueue.front().input[(int)input].type;
    functionQueueMutex.unlock();
    return returnValue;
}

GMEXPORT double getFunction()
{
    functionQueueMutex.lock();
    double returnValue = functionQueue.front().function;
    functionQueueMutex.unlock();
    return returnValue;
}

GMEXPORT double setDelayedOutput(double value)
{
    functionQueueMutex.lock();
    *(functionQueue.front().delayedOutput) = value;
    functionQueueMutex.unlock();
    return value;
}

GMEXPORT double getHasOutput()
{
    functionQueueMutex.lock();
    double returnValue = functionQueue.front().hasOutput;
    functionQueueMutex.unlock();
    return returnValue;
}

GMEXPORT double isThereDelayedFunctionCall()
{
    functionQueueMutex.lock();
    double returnValue = !functionQueue.empty();
    functionQueueMutex.unlock();
    return returnValue;
}

static delayedInput convertToDelayedInput(double input)
{
    delayedInput returnValue;
    returnValue.number = input;
    returnValue.type = 0;
    return returnValue;
}

static delayedInput convertToDelayedInput(const char* input)
{
    delayedInput returnValue;
    returnValue.text = (char*) malloc((strlen(input) + 1)*sizeof(char));
    strcpy(returnValue.text, input);
    returnValue.type = 1;
    return returnValue;
}

template <typename T> void addDelayedFunctionCall_helper(int argument, delayedFunctionCall* call, T input)
{
    call->input[argument] = convertToDelayedInput(input);
}

template <typename T, typename... Input> void addDelayedFunctionCall_helper(int argument, delayedFunctionCall* call, T input0, Input... input)
{
    call->input[argument] = convertToDelayedInput(input0);
    argument++;
    addDelayedFunctionCall_helper(argument, call, input...);
}

static double addDelayedFunctionCall(int function, int hasOutput)
{
    delayedFunctionCall call;
    memset(&call, 0, sizeof(delayedFunctionCall));
    double delayedOutput = 0;
    call.function      = function;
    call.delayedOutput = &delayedOutput;
    call.hasOutput     = hasOutput;
    functionQueueMutex.lock();
    functionQueue.push(call);
    functionQueueMutex.unlock();

    if (call.hasOutput)
    {
        while (isThereDelayedFunctionCall() && !is_dll_function_call_complete())
            ;
    }

    return delayedOutput;
}

template <typename... Input> double addDelayedFunctionCall(int function, int hasOutput, Input... input)
{
    delayedFunctionCall call;
    memset(&call, 0, sizeof(delayedFunctionCall));
    double delayedOutput = 0;
    call.function      = function;
    call.delayedOutput = &delayedOutput;
    call.hasOutput     = hasOutput;

    addDelayedFunctionCall_helper(0, &call, input...);
    functionQueueMutex.lock();
    functionQueue.push(call);
    functionQueueMutex.unlock();

    if (call.hasOutput)
    {
        while ((int)isThereDelayedFunctionCall() && !(int)is_dll_function_call_complete())
            ;
    }

    return delayedOutput;
}

#define ADD_FUNCTION(name)\
static int FP_##name;\
GMEXPORT double export_##name(double functionPointer);\
GMEXPORT double export_##name(double functionPointer)\
{\
    FP_##name = functionPointer;\
    return functionPointer;\
}

#include "gameMakerLibrary.hpp"
#include "gameMakerGenLibrary.hpp"
#include "variableWrapperCallback.hpp"

std::mutex functionCallMutex;
volatile void* function_to_call;
volatile delayedInput dll_input[14];
volatile int function_returned;
volatile double function_return_value;

void *dll_function_call_loop(void *vargp)
{
    while (1)
    {
        functionCallMutex.lock();
        if (function_returned == 0)
        {
            #include "callGameLoop.hpp"
            function_returned = 1;
            function_to_call = NULL;
        }
        functionCallMutex.unlock();
    }
    return NULL;
}

GMEXPORT double prime_argument_real(double index, double value)
{
    functionCallMutex.lock();
    dll_input[(int)index].number = value;
    functionCallMutex.unlock();
    return 0;
}

GMEXPORT double prime_argument_string(double index, char* value)
{
    functionCallMutex.lock();
    dll_input[(int)index].text = (char*) malloc((strlen(value) + 1)*sizeof(char));
    strcpy(dll_input[(int)index].text, value);
    functionCallMutex.unlock();
    return 0;
}

GMEXPORT double call_dll_function(double function_pointer)
{
    functionCallMutex.lock();
    function_to_call = (void*)(int)function_pointer;
    function_returned = 0;
    functionCallMutex.unlock();
    return 0;
}

GMEXPORT double is_dll_function_call_complete()
{
    return function_returned;
}

GMEXPORT double get_dll_function_call_return_value()
{
    return function_return_value;
}

GMEXPORT double init_dll_function_call_loop()
{
    #include "initVariableWrapper.hpp"

    function_returned = 1;
    function_to_call = NULL;

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, dll_function_call_loop, NULL);

    return 0;
}

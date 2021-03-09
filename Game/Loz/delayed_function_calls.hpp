#define GMEXPORT extern "C" __declspec(dllexport)

GMEXPORT double removeDelayedFunctionCall();
GMEXPORT double getInputDelayedVariable(double input);
GMEXPORT char* getInputText(double input);
GMEXPORT double getInputNumber(double input);
GMEXPORT double getInputType(double input);
GMEXPORT double getFunction();
GMEXPORT double setDelayedOutput(double value);
GMEXPORT double getHasOutput();
GMEXPORT double isThereDelayedFunctionCall();
GMEXPORT double is_dll_function_call_complete();
GMEXPORT double get_dll_function_call_return_value();

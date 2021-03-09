#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdbool.h>

char* getFunctionName(FILE* inputFile)
{
    char* functionName = (char*)malloc(sizeof(*functionName));
    functionName[0] = 0;
    int functionNameLength = 0;
    int c;

    while ((c = fgetc(inputFile)) != EOF)
    {
        if (c == '(' || c == ';')
            break;
        else
        {
            functionName = (char*)realloc(functionName, sizeof(functionName)*(functionNameLength+2));
            functionName[functionNameLength] = c;
            functionName[functionNameLength+1] = 0;
            functionNameLength++;
        }
    }

    return functionName;
}

bool walk_DLL_function(FILE* inputFile, FILE* outputFile, FILE* outputFile2)
{
    if (fscanf(inputFile, "%*[^ ])") == EOF) return 1;
    if (fscanf(inputFile, "%*1[ ]%*[^ ])") == EOF) return 1;
    if (fgetc(inputFile) != ' ') return 1;

    int c;
    char* functionName = getFunctionName(inputFile);

    fprintf(outputFile, "GMEXPORT double get_%s()\n", functionName);
    fprintf(outputFile, "{\n");
    fprintf(outputFile, "    return (double)(int)%s;\n", functionName);
    fprintf(outputFile, "}\n");

    fprintf(outputFile2, "if (function_to_call == (void*)%s)\n", functionName);
    fprintf(outputFile2, "    function_return_value = %s(", functionName);

    int argumentCount = 0;
    int waitingTillComma = 0;

    while ((c = fgetc(inputFile)) != EOF)
    {
        if (c == ')')
            break;
        else if (c == 'd' && !waitingTillComma)
        {
            fprintf(outputFile2, "dll_input[%u].number", argumentCount);
            waitingTillComma = 1;
            argumentCount++;
        } else if (c == 'c' && !waitingTillComma)
        {
            fprintf(outputFile2, "dll_input[%u].text", argumentCount);
            waitingTillComma = 1;
            argumentCount++;
        } else if (c == ',' && waitingTillComma)
        {
            fprintf(outputFile2, ", ");
            waitingTillComma = 0;
        }
    }

    fprintf(outputFile2, ");\n");
    return c == EOF;
}

bool walk_GML_function_to_make_exporter(FILE* inputFile, FILE* outputFile, FILE* projectFileOut)
{
    int functionType = 2;
    int c;

    while ((c = fgetc(inputFile)) != EOF)
    {
        if (c == ' ')
            break;
        else if (c == 'v' && functionType == 2)
            functionType = 0;
        else if (c == 'd' && functionType == 2)
            functionType = 1;
    }

    if (c == EOF)
        return c == EOF;

    char* functionName = getFunctionName(inputFile);

    fprintf(outputFile, "ADD_FUNCTION(%s)\n", functionName);

    fprintf(projectFileOut, "global.delayCallDLL_export_%s = external_define(\"CallDelayedFunctions.dll\", \"export_%s\", dll_cdecl, ty_real, 1, ty_real);\n", functionName, functionName);
    fprintf(projectFileOut, "external_call(global.delayCallDLL_export_%s, scr_wrapper_%s);\n", functionName, functionName);

    int argumentCount = 0;
    int* argumentType = NULL;
    int waitingTillComma = 0;

    while ((c = fgetc(inputFile)) != EOF)
    {
        if (c == ')')
            break;
        else if (c == 'd' && !waitingTillComma)
        {
            argumentType = (int*)realloc(argumentType, sizeof(*argumentType)*(argumentCount+1));
            argumentType[argumentCount] = 0;
            waitingTillComma = 1;
            argumentCount++;
        } else if (c == 'c' && !waitingTillComma)
        {
            argumentType = (int*)realloc(argumentType, sizeof(*argumentType)*(argumentCount+1));
            argumentType[argumentCount] = 1;
            waitingTillComma = 1;
            argumentCount++;
        } else if (c == ',' && waitingTillComma)
            waitingTillComma = 0;
    }

    if (functionType == 0)
        fprintf(outputFile, "void %s(", functionName);
    else if (functionType == 1)
        fprintf(outputFile, "double %s(", functionName);

    for (int i = 0; i < argumentCount; i++)
    {
        if (argumentType[i] == 0)
            fprintf(outputFile, "double input%u", i);
        else if (argumentType[i] == 1)
            fprintf(outputFile, "const char* input%u", i);

        if (i+1 < argumentCount)
            fprintf(outputFile, ", ");
    }

    fprintf(outputFile, ")\n");
    fprintf(outputFile, "{\n");

    if (functionType == 0)
        fprintf(outputFile, "    addDelayedFunctionCall(FP_%s, 0", functionName);
    else if (functionType == 1)
        fprintf(outputFile, "    return addDelayedFunctionCall(FP_%s, 1", functionName);

    for (int i = 0; i < argumentCount; i++)
        fprintf(outputFile, ", input%u", i);

    fprintf(outputFile, ");\n");
    fprintf(outputFile, "}\n");

    return c == EOF;
}

bool walk_GML_function_to_make_built_in_variables_export(FILE* inputFile, FILE* projectFileOut, FILE* variableWrapperDeclaration, FILE* variableWrapperCallback, FILE* initVariableWrapper)
{
    int c;

    while ((c = fgetc(inputFile)) != EOF && c != ' ');
    while ((c = fgetc(inputFile)) != EOF && c != ' ');

    if (c == EOF)
        return c == EOF;

    char* functionName = getFunctionName(inputFile);

    fprintf(variableWrapperDeclaration, "variableWrapper* %s;\n", functionName);

    fprintf(projectFileOut, "global.delayCallDLL_export_get_%s = external_define(\"CallDelayedFunctions.dll\", \"export_get_%s\", dll_cdecl, ty_real, 1, ty_real);\n", functionName, functionName);
    fprintf(projectFileOut, "external_call(global.delayCallDLL_export_get_%s, scr_wrapper_get_%s);\n", functionName, functionName);
    fprintf(projectFileOut, "global.delayCallDLL_export_set_%s = external_define(\"CallDelayedFunctions.dll\", \"export_set_%s\", dll_cdecl, ty_real, 1, ty_real);\n", functionName, functionName);
    fprintf(projectFileOut, "external_call(global.delayCallDLL_export_set_%s, scr_wrapper_set_%s);\n", functionName, functionName);

    int argumentCount = 0;
    int* argumentType = NULL;
    int waitingTillComma = 0;

    fprintf(variableWrapperCallback, "ADD_FUNCTION(get_%s)\n", functionName);
    fprintf(variableWrapperCallback, "ADD_FUNCTION(set_%s)\n", functionName);

    fprintf(variableWrapperCallback, "double get_%s()\n", functionName);
    fprintf(variableWrapperCallback, "{\n", functionName);
    fprintf(variableWrapperCallback, "    return addDelayedFunctionCall(FP_get_%s, 1);\n", functionName);
    fprintf(variableWrapperCallback, "}\n", functionName);

    fprintf(variableWrapperCallback, "void set_%s(double value)\n", functionName);
    fprintf(variableWrapperCallback, "{\n", functionName);
    fprintf(variableWrapperCallback, "    addDelayedFunctionCall(FP_set_%s, 0, value);\n", functionName);
    fprintf(variableWrapperCallback, "}\n", functionName);

    fprintf(initVariableWrapper, "%s = new variableWrapper(get_%s, set_%s);\n", functionName, functionName, functionName);

    return c == EOF;
}

bool walk_GML_function_to_make_wrapper(FILE* inputFile, FILE* projectFileOut)
{
    int functionType = 2;
    int c;

    while ((c = fgetc(inputFile)) != EOF)
    {
        if (c == ' ')
            break;
        else if (c == 'v' && functionType == 2)
            functionType = 0;
        else if (c == 'd' && functionType == 2)
            functionType = 1;
    }

    if (c == EOF)
        return c == EOF;

    char* functionName = getFunctionName(inputFile);

    fprintf(projectFileOut, "#define scr_wrapper_%s\n", functionName);

    int argumentCount = 0;
    int* argumentType = NULL;
    int waitingTillComma = 0;

    while ((c = fgetc(inputFile)) != EOF)
    {
        if (c == ')')
            break;
        else if (c == 'd' && !waitingTillComma)
        {
            argumentType = (int*)realloc(argumentType, sizeof(*argumentType)*(argumentCount+1));
            argumentType[argumentCount] = 0;
            waitingTillComma = 1;
            argumentCount++;
        } else if (c == 'c' && !waitingTillComma)
        {
            argumentType = (int*)realloc(argumentType, sizeof(*argumentType)*(argumentCount+1));
            argumentType[argumentCount] = 1;
            waitingTillComma = 1;
            argumentCount++;
        } else if (c == ',' && waitingTillComma)
            waitingTillComma = 0;
    }

    if (functionType == 0)
        fprintf(projectFileOut, "%s(", functionName);
    else if (functionType == 1)
        fprintf(projectFileOut, "return %s(", functionName);

    for (int i = 0; i < argumentCount; i++)
    {
        fprintf(projectFileOut, "argument[%u]", i);

        if (i+1 < argumentCount)
            fprintf(projectFileOut, ",");
    }

    fprintf(projectFileOut, ");\n\n");

    return c == EOF;
}

bool walk_GML_function_to_make_built_in_variables_import(FILE* inputFile, FILE* projectFileOut)
{
    int c;

    while ((c = fgetc(inputFile)) != EOF)
    {
        if (c == ' ')
            break;
    }

    while ((c = fgetc(inputFile)) != EOF)
    {
        if (c == ' ')
            break;
    }

    if (c == EOF)
        return c == EOF;

    char* functionName = getFunctionName(inputFile);

    fprintf(projectFileOut, "#define scr_wrapper_get_%s\n", functionName);
    fprintf(projectFileOut, "return %s;\n\n", functionName);

    fprintf(projectFileOut, "#define scr_wrapper_set_%s\n", functionName);
    fprintf(projectFileOut, "//%s = argument[0];\n\n", functionName);

    return c == EOF;
}

void walk_GML_header_to_make_exporter(FILE* outputFile, FILE* projectFileOut, const char* inputFileName)
{
    FILE* inputFile = fopen(inputFileName, "r");

    while (!walk_GML_function_to_make_exporter(inputFile, outputFile, projectFileOut))
        ;

    fclose(inputFile);
}

void walk_GML_header_to_make_built_in_variables_export(FILE* projectFileOut, FILE* variableWrapperDeclaration, FILE* variableWrapperCallback, FILE* initVariableWrapper, const char* inputFileName)
{
    FILE* inputFile = fopen(inputFileName, "r");

    while (!walk_GML_function_to_make_built_in_variables_export(inputFile, projectFileOut, variableWrapperDeclaration, variableWrapperCallback, initVariableWrapper))
        ;

    fclose(inputFile);
}

void walk_GML_header_to_make_wrapper(FILE* projectFileOut, const char* inputFileName)
{
    FILE* inputFile = fopen(inputFileName, "r");

    while (!walk_GML_function_to_make_wrapper(inputFile, projectFileOut))
        ;

    fclose(inputFile);
}

void walk_GML_header_to_make_built_in_variables_import(FILE* projectFileOut, const char* inputFileName)
{
    FILE* inputFile = fopen(inputFileName, "r");

    while (!walk_GML_function_to_make_built_in_variables_import(inputFile, projectFileOut))
        ;

    fclose(inputFile);
}

void walk_GML_libraries(FILE* projectFileOut, FILE* variableWrapperDeclaration, FILE* variableWrapperCallback, FILE* initVariableWrapper)
{
    FILE* outputFile = fopen("gameMakerGenLibrary.hpp", "w");

    fprintf(projectFileOut,"#define scr_delayCallDLL_exportWrapperFunctions\n");
    walk_GML_header_to_make_exporter(outputFile, projectFileOut, "gameMakerFunctions\\3DGraphics\\d3d_model.hpp");
    walk_GML_header_to_make_exporter(outputFile, projectFileOut, "gameMakerFunctions\\3DGraphics\\d3d_primitive.hpp");
    walk_GML_header_to_make_exporter(outputFile, projectFileOut, "gameMakerFunctions\\3DGraphics\\d3d_shape.hpp");
    walk_GML_header_to_make_exporter(outputFile, projectFileOut, "gameMakerFunctions\\3DGraphics\\d3d_transform.hpp");
    walk_GML_header_to_make_exporter(outputFile, projectFileOut, "gameMakerFunctions\\gameGraphics\\fontsAndText.hpp");
    walk_GML_header_to_make_exporter(outputFile, projectFileOut, "gameMakerFunctions\\gameGraphics\\drawingShapes.hpp");
    //walk_GML_header_to_make_exporter(outputFile, projectFileOut, "gameMakerFunctions\\gameGraphics\\theWindow.hpp");
    //walk_GML_header_to_make_exporter(outputFile, projectFileOut, "gameMakerFunctions\\gameGraphics\\theDisplay.hpp");
    walk_GML_header_to_make_exporter(outputFile, projectFileOut, "gameMakerFunctions\\userInteraction\\mouse.hpp");
    walk_GML_header_to_make_exporter(outputFile, projectFileOut, "gameMakerFunctions\\userInteraction\\keyboard.hpp");
    walk_GML_header_to_make_exporter(outputFile, projectFileOut, "gameMakerFunctions\\userInteraction\\joystick.hpp");
    walk_GML_header_to_make_exporter(outputFile, projectFileOut, "gameMakerFunctions\\gamePlay\\rooms.hpp");
    walk_GML_header_to_make_exporter(outputFile, projectFileOut, "gameMakerFunctions\\customFunctions.hpp");
    walk_GML_header_to_make_built_in_variables_export(projectFileOut, variableWrapperDeclaration, variableWrapperCallback, initVariableWrapper, "gameMakerFunctions\\builtInVariables.hpp");
    fprintf(projectFileOut,"\n");

    walk_GML_header_to_make_wrapper(projectFileOut, "gameMakerFunctions\\3DGraphics\\d3d_model.hpp");
    walk_GML_header_to_make_wrapper(projectFileOut, "gameMakerFunctions\\3DGraphics\\d3d_primitive.hpp");
    walk_GML_header_to_make_wrapper(projectFileOut, "gameMakerFunctions\\3DGraphics\\d3d_shape.hpp");
    walk_GML_header_to_make_wrapper(projectFileOut, "gameMakerFunctions\\3DGraphics\\d3d_transform.hpp");
    walk_GML_header_to_make_wrapper(projectFileOut, "gameMakerFunctions\\gameGraphics\\fontsAndText.hpp");
    walk_GML_header_to_make_wrapper(projectFileOut, "gameMakerFunctions\\gameGraphics\\drawingShapes.hpp");
    //walk_GML_header_to_make_wrapper(projectFileOut, "gameMakerFunctions\\gameGraphics\\theWindow.hpp");
    //walk_GML_header_to_make_wrapper(projectFileOut, "gameMakerFunctions\\gameGraphics\\theDisplay.hpp");
    walk_GML_header_to_make_wrapper(projectFileOut, "gameMakerFunctions\\userInteraction\\mouse.hpp");
    walk_GML_header_to_make_wrapper(projectFileOut, "gameMakerFunctions\\userInteraction\\keyboard.hpp");
    walk_GML_header_to_make_wrapper(projectFileOut, "gameMakerFunctions\\userInteraction\\joystick.hpp");
    walk_GML_header_to_make_wrapper(projectFileOut, "gameMakerFunctions\\gamePlay\\rooms.hpp");
    walk_GML_header_to_make_wrapper(projectFileOut, "gameMakerFunctions\\customFunctions.hpp");
    walk_GML_header_to_make_built_in_variables_import(projectFileOut, "gameMakerFunctions\\builtInVariables.hpp");
}

int main()
{
    FILE* inputFile = fopen("gameLoop.hpp", "r");
    FILE* outputFile = fopen("getGameLoop.hpp", "w");
    FILE* outputFile2 = fopen("callGameLoop.hpp", "w");
    FILE* variableWrapperDeclaration = fopen("variableWrapperDeclaration.hpp", "w");
    FILE* variableWrapperCallback = fopen("variableWrapperCallback.hpp", "w");
    FILE* initVariableWrapper = fopen("initVariableWrapper.hpp", "w");
    FILE* projectFileOut = fopen("..\\importLibraries.gml", "w");

    while (!walk_DLL_function(inputFile, outputFile, outputFile2))
        ;

    walk_GML_libraries(projectFileOut, variableWrapperDeclaration, variableWrapperCallback, initVariableWrapper);
}

#ifndef VARIABLE_WRAPPER_H
#define VARIABLE_WRAPPER_H
class variableWrapper
{
    public:
        variableWrapper(double (*g_value)(), void (*s_value)(double))
        : get_value(g_value), set_value(s_value)
        {

        }

        operator double() const
        {
            return get_value();
        }

        void operator=(const double& value)
        {
            set_value(value);
        }

    private:
        double (*get_value)();
        void (*set_value)(double);
};
#endif

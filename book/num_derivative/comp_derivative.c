#include <stdio.h>
#include <complex.h>
#include <tgmath.h>

double _Complex F(double _Complex x) {
    return csin(x);
}

double _Complex numerical_derivative(double _Complex(*func)(double _Complex), double _Complex x, double h) {
    return cimag(func(x + I * h)) / h;
}

int main(void) {
    /* works with real numbers */
    double _Complex x = M_PI / 4 + I * 0.0;
    double h = 1e-5;
    double pi = 3.14;

    double _Complex derivative = numerical_derivative(F, x, h);
    double analytical_derivative = creal(ccos(x));

    printf("Numerical derivative at  x = %f: %f\n", creal(x), derivative);
    printf("Analytical derivative at x = %f: %f\n", creal(x), analytical_derivative);

    return 0;
}

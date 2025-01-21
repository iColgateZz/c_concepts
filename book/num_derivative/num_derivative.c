#include <stdio.h>
#include <math.h>

double F(double x) {
    return sin(x);
}

double numerical_derivative(double (*func)(double), double x, double h) {
    return (func(x + h) - func(x - h)) / (2 * h);
}

int main(void) {
    double x = M_PI / 4;
    double h = 1e-5;

    double derivative = numerical_derivative(F, x, h);
    double analytical_derivative = cos(x);

    printf("Numerical derivative at  x = %f: %f\n", x, derivative);
    printf("Analytical derivative at x = %f: %f\n", x, analytical_derivative);

    return 0;
}

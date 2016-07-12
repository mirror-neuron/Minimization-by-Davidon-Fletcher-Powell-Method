#include <iostream>
#include <cmath>
#include "matrix.h"
#include "functionals.h"

/// оптимальные значения констант:
/// "const" >= 1e-9, иначе 1 + const = 1
/// EPSILON >= 2e-9
/// DIFF_STEP >= 1e-6, иначе погрешность диффер-ия сильно возрастает
/// MATRIX_EPS ~~ 1e-1, иначе интервал унимодальности находиться не верно,
///     min_s -> infiniry и программа может думать очень долго
///
/// оптимальные константы, если включены штрафы:
/// 1/TAX ~~ EPSILON, так как 1/TAX ~~ точность решения, если 1/TAX > EPSILON;
///     однако когда алгоритму надо идти вдоль границы области 1/TAX >> EPSILON
/// если 1/TAX > 2*DIFF_STEP, то будут ошибки определения производной на границе
#define GOLD_EPS 1e-9           ///точность одномерной минимизации
#define DIFF_STEP 1e-5          ///шаг дифференцирования
#define EPSILON 5e-9            ///правило останова
#define NORM_EPS 1e-4           ///для определения нулевой нормы[в s_unimodal]
#define UNIM_STEP 0.1           ///шаг для определения интервала унимодальности
#define TAX 1e4                 ///параметр штрафа

#define _X_0_ -2.0, -2.0         ///начальная точна[обязательно double, double]
#define DEBUG (0)               ///отладочный\итоговый вывод программы

/// глобальные переменные
vector x_k(2);
matrix D(2, 2);                 ///приближенная матрица Гессе

/// штрафная функция (g[i] <= 0)
double penalty(vector x)
{
    double pnt = 0;
    pnt += pow(max(0, x[0]*x[0] + 2*(x[1]-2) - 8), 2);        //УНЛО
    pnt += pow(max(0, 1 - x[0]*x[0] - (x[1]-2)*(x[1]-2)), 2);

    //pnt += pow(max(0, -x[0]), 2);                             //проверка штрафов
    //pnt += pow(max(0, -x[1]), 2);
    //pnt += pow(max(0, x[0]*x[0] + x[1]*x[1] - 1), 2);

    //pnt += pow(max(0, 1 + 2*x[0] + x[1]*x[1]), 2);        //исправленое задание
    return TAX * pnt;
}

/// целевая функция
double f(vector x)
{
    return x[0]*x[0] + x[1]*x[1] + penalty(x);                //УНЛО
    //return pow(x[0] + 1, 2) + pow(x[1] - 1, 2) + penalty(x);  //проверка штрафов
    //return 100 * pow(x[1] - x[0]*x[0], 2) + pow(1 - x[0], 2); //тест Розенброка
    //return 100 * pow(x[1] - 2*pow(x[0], 3) + 2*x[0], 2) +  x[0]*x[0]; //мой тест
    //return - (pow(x[0]*x[0]+x[1]-11, 2) + pow(x[0]+x[1]*x[1]-7, 2));      //Химмельблау
    //return exp(x[0]*x[0] +  2*x[1]*x[1]) - x[0] - x[1] + penalty(x);  //исп задание
    //return sqrt((1 + 2*x[0] + x[1]*x[1])) + exp(x[0]*x[0] + \
    //                            2*x[1]*x[1]) - x[0] - x[1];           //задание
}

/// градиент целевой функции
/// дифференцирование проиводиться с точностью до 4-го порядка малости от h
vector grad_f(vector x)
{
    double h = DIFF_STEP;
    vector g(2);
    vector dx0(2);
    vector dx1(2);
    init(dx0, h, 0.0);
    init(dx1, 0.0, h);
    g[0] = (f(x - 2*dx0) - 8*f(x - dx0) + 8*f(x + dx0) - f(x + 2*dx0)) / 12 / h;
    g[1] = (f(x - 2*dx1) - 8*f(x - dx1) + 8*f(x + dx1) - f(x + 2*dx1)) / 12 / h;
    return g;
}

/// функция возвращает f(x<k+1>)[нужна для золотого сечения]
inline double f_x_kk(double s)
{
    extern vector x_k;
    extern matrix D;
    return f(x_k - s*D*grad_f(x_k));
}

/// опр-ет интервал унимодальности, для применения в нем метода золотого сечения
/// для этого увеличивает s до тех пор, пока f(x) не начнет возрастать
void s_unimodal(double& s_min, double& s_max, double unim_step)
{
    extern vector x_k;
    extern matrix D;
    double s0 = 0;
    double s1 = unim_step;
    double s2 = 2*unim_step;
    int n = 0;
    while ((f_x_kk(s1) > f_x_kk(s2)) && norm(D*grad_f(x_k)) > NORM_EPS && (++n < 1e4))
    {
        s0 = s1;
        s1 = s2;
        s2 += unim_step;
    }
    s_min = s0;
    s_max = s2;
}

int main()
{
/// 0-й шаг
    extern vector x_k;                  ///x<k>
    extern matrix D;                    ///приближенная матрица Гессе (D<o> = E)
    int k;
    matrix A(2, 2);                     ///A = u * (u)^t / ((u)^t * v)
    matrix B(2, 2);                     ///B = -(D * v * (v)^t * D) / ((v)^t*D*v)
    vector x_kk(2);                     ///x<k+1>
    vector u(2);                        ///u<k> = x<k+1> - x<k>
    vector v(2);                        ///v<k> = grad_f(x<k+1>) - grad_f(x<k>)
    double s;                           ///параметр одномерной минимизации
    double s_min, s_max;                ///границы унимодальности

    k = 1;
    x_k = vector(2);
    init(x_k, _X_0_);
    D = identity(2);

/// 1 шаг
    s_unimodal(s_min, s_max, UNIM_STEP / norm(grad_f(x_k)));
    s = golden_section(f_x_kk, s_min, s_max, GOLD_EPS); ///метод золотого сечения
	x_kk = x_k - s*D*grad_f(x_k);
	printf("#       X               Y               Z        n\n");
    printf("%15.12lf %15.12lf %15.12lf  %d\n", x_k[0], x_k[1], f(x_k), k-1);
    printf("%15.12lf %15.12lf %15.12lf  %d\n", x_kk[0], x_kk[1], f(x_kk), k);

/// 2..n шаг
    do
    {
        u = x_kk - x_k;                                                 ///u<k>
        v = grad_f(x_kk) - grad_f(x_k);                                 ///v<k>
        A = u * transpose(u) / (transpose(u) * v);                      ///A<k>
        B = (D * v * transpose(v) * D) / (- (transpose(v) * D * v));    ///B<k>
        D += A + B;                                                     ///D<k+1>
        k++;
        x_k = x_kk;                                                     ///x<k+1>
        s_unimodal(s_min, s_max, UNIM_STEP);
        s = golden_section(f_x_kk, s_min, s_max, GOLD_EPS);
        x_kk = x_k - s*D*grad_f(x_k);
        printf("%15.12lf %15.12lf %15.12lf  %d\n", x_kk[0], x_kk[1], f(x_kk), k);
    } while (norm(x_kk - x_k) > EPSILON);

    putchar('\n');
    return 0;
}

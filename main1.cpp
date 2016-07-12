#include <iostream>
#include <cmath>
#include "matrix.h"
#include "functionals.h"

/// ����������� �������� ��������:
/// "const" >= 1e-9, ����� 1 + const = 1
/// EPSILON >= 2e-9
/// DIFF_STEP >= 1e-6, ����� ����������� ������-�� ������ ����������
/// MATRIX_EPS ~~ 1e-1, ����� �������� �������������� ���������� �� �����,
///     min_s -> infiniry � ��������� ����� ������ ����� �����
///
/// ����������� ���������, ���� �������� ������:
/// 1/TAX ~~ EPSILON, ��� ��� 1/TAX ~~ �������� �������, ���� 1/TAX > EPSILON;
///     ������ ����� ��������� ���� ���� ����� ������� ������� 1/TAX >> EPSILON
/// ���� 1/TAX > 2*DIFF_STEP, �� ����� ������ ����������� ����������� �� �������
#define GOLD_EPS 1e-9           ///�������� ���������� �����������
#define DIFF_STEP 1e-5          ///��� �����������������
#define EPSILON 5e-9            ///������� ��������
#define NORM_EPS 1e-4           ///��� ����������� ������� �����[� s_unimodal]
#define UNIM_STEP 0.1           ///��� ��� ����������� ��������� ��������������
#define TAX 1e4                 ///�������� ������

#define _X_0_ -2.0, -2.0         ///��������� �����[����������� double, double]
#define DEBUG (0)               ///����������\�������� ����� ���������

/// ���������� ����������
vector x_k(2);
matrix D(2, 2);                 ///������������ ������� �����

/// �������� ������� (g[i] <= 0)
double penalty(vector x)
{
    double pnt = 0;
    pnt += pow(max(0, x[0]*x[0] + 2*(x[1]-2) - 8), 2);        //����
    pnt += pow(max(0, 1 - x[0]*x[0] - (x[1]-2)*(x[1]-2)), 2);

    //pnt += pow(max(0, -x[0]), 2);                             //�������� �������
    //pnt += pow(max(0, -x[1]), 2);
    //pnt += pow(max(0, x[0]*x[0] + x[1]*x[1] - 1), 2);

    //pnt += pow(max(0, 1 + 2*x[0] + x[1]*x[1]), 2);        //����������� �������
    return TAX * pnt;
}

/// ������� �������
double f(vector x)
{
    return x[0]*x[0] + x[1]*x[1] + penalty(x);                //����
    //return pow(x[0] + 1, 2) + pow(x[1] - 1, 2) + penalty(x);  //�������� �������
    //return 100 * pow(x[1] - x[0]*x[0], 2) + pow(1 - x[0], 2); //���� ����������
    //return 100 * pow(x[1] - 2*pow(x[0], 3) + 2*x[0], 2) +  x[0]*x[0]; //��� ����
    //return - (pow(x[0]*x[0]+x[1]-11, 2) + pow(x[0]+x[1]*x[1]-7, 2));      //�����������
    //return exp(x[0]*x[0] +  2*x[1]*x[1]) - x[0] - x[1] + penalty(x);  //��� �������
    //return sqrt((1 + 2*x[0] + x[1]*x[1])) + exp(x[0]*x[0] + \
    //                            2*x[1]*x[1]) - x[0] - x[1];           //�������
}

/// �������� ������� �������
/// ����������������� ������������ � ��������� �� 4-�� ������� ������� �� h
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

/// ������� ���������� f(x<k+1>)[����� ��� �������� �������]
inline double f_x_kk(double s)
{
    extern vector x_k;
    extern matrix D;
    return f(x_k - s*D*grad_f(x_k));
}

/// ���-�� �������� ��������������, ��� ���������� � ��� ������ �������� �������
/// ��� ����� ����������� s �� ��� ���, ���� f(x) �� ������ ����������
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
/// 0-� ���
    extern vector x_k;                  ///x<k>
    extern matrix D;                    ///������������ ������� ����� (D<o> = E)
    int k;
    matrix A(2, 2);                     ///A = u * (u)^t / ((u)^t * v)
    matrix B(2, 2);                     ///B = -(D * v * (v)^t * D) / ((v)^t*D*v)
    vector x_kk(2);                     ///x<k+1>
    vector u(2);                        ///u<k> = x<k+1> - x<k>
    vector v(2);                        ///v<k> = grad_f(x<k+1>) - grad_f(x<k>)
    double s;                           ///�������� ���������� �����������
    double s_min, s_max;                ///������� ��������������

    k = 1;
    x_k = vector(2);
    init(x_k, _X_0_);
    D = identity(2);

/// 1 ���
    s_unimodal(s_min, s_max, UNIM_STEP / norm(grad_f(x_k)));
    s = golden_section(f_x_kk, s_min, s_max, GOLD_EPS); ///����� �������� �������
	x_kk = x_k - s*D*grad_f(x_k);
	printf("#       X               Y               Z        n\n");
    printf("%15.12lf %15.12lf %15.12lf  %d\n", x_k[0], x_k[1], f(x_k), k-1);
    printf("%15.12lf %15.12lf %15.12lf  %d\n", x_kk[0], x_kk[1], f(x_kk), k);

/// 2..n ���
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

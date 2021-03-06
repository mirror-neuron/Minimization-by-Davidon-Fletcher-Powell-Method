#include "matrix.h"
#include <iostream>
#include <cmath>

#define EPSILON 1e-16

void errormessage(char* from, char* errormessage)
{
  std::cerr << "Error! Message from " << from << " : " \
	    << errormessage << ".\n";
  exit(666);
};

/////////////////////////////////// matrices /////////////////////////////////////

matrix::matrix()
{
  lines = 0;
  colomns = 0;
  data = new double* [0];
  data[0] = new double [0];
}

matrix::matrix(int lines, int colomns)
{
  matrix::lines = lines;
  matrix::colomns = colomns;
  data = new double* [lines];
  for(int i = 0; i < lines; i++)
    data[i] = new double [colomns];
}

matrix::~matrix()
{
  for(int i = 0; i < lines; i++)
    delete [] data[i];
  delete [] data;
}

matrix::matrix(const matrix& a)
{
  //initialization this
  lines = a.lines;
  colomns = a.colomns;
  data = new double* [lines];
  for(int i = 0; i < lines; i++)
    data[i] = new double [colomns];
  //copying
  for(int i = 0; i < lines; i++)
    for(int j = 0; j < colomns; j++)
      data[i][j] = a.data[i][j];
}

void print(const matrix& a)
{
  for(int i = 0; i < a.lines; i++)
    {
      for(int j = 0; j < a.colomns; j++)
	printf("%7.3lf ", a.data[i][j]);
      printf("\n");
    }
  printf("\n");
}

matrix init(matrix &a, double a00, ...)
{
  double *p = &a00;
  if ( a.colomns < 1 || a.lines < 1)
    errormessage("init", "bad size");
  for(int i = 0; i < a.lines; i++)
    for(int j = 0; j < a.colomns; j++)
      a.data[i][j] = *(p++);
  return a;
}

matrix zero(int lines, int colomns)
{
  matrix temp(lines, colomns);
  for(int i = 0; i < lines; i++)
    for(int j = 0; j < colomns; j++)
      temp.data[i][j] = 0;
  return temp;
}

matrix identity(int n)
{
  matrix temp = zero(n, n);
  for(int i = 0; i < n; i++)
    temp.data[i][i] = 1;
  return temp;
}

matrix& matrix::operator=(const matrix& a)
{
  if (this != &a) //not danger when do a = a
    {
      //deling this->data
      for(int i = 0; i < lines; i++)
	delete [] data[i];
      delete [] data;
      //initialization this
      lines = a.lines;
      colomns = a.colomns;
      data = new double* [lines];
      for(int i = 0; i < lines; i++)
	data[i] = new double [colomns];
      //copying
      for(int i = 0; i < lines; i++)
	for(int j = 0; j < colomns; j++)
	  data[i][j] = a.data[i][j];
    }
  return *this;
}

matrix& matrix::operator+=(const matrix& a)
{
  if (lines != a.lines || colomns != a.colomns)
    errormessage("matrix::operator+=", "different size");
  for(int i = 0; i < lines; i++)
    for(int j = 0; j < colomns; j++)
      data[i][j] += a.data[i][j];
  return *this;
}

matrix& matrix::operator-=(const matrix& a)
{
  if (lines != a.lines || colomns != a.colomns)
    errormessage("matrix::operator-=", "different size");
  for(int i = 0; i < lines; i++)
    for(int j = 0; j < colomns; j++)
      data[i][j] -= a.data[i][j];
  return *this;
}

matrix& matrix::operator*=(double c)
{
  for(int i = 0; i < lines; i++)
    for(int j = 0; j < colomns; j++)
      data[i][j] *= c;
  return *this;
}

matrix& matrix::operator*=(const matrix& a)
{
  if (colomns != a.lines)
    errormessage("matrix::operator*=", "different size");
  matrix temp(lines, a.colomns);
  for(int i = 0; i < temp.lines; i++)
    for(int j = 0; j < temp.colomns; j++)
      {
	temp.data[i][j] = 0;
	for(int k = 0; k < colomns; k++)
	  temp.data[i][j] += data[i][k] * a.data[k][j];
      }
  return *this = temp;
}

matrix& matrix::operator/=(double c)
{
  for(int i = 0; i < lines; i++)
    for(int j = 0; j < colomns; j++)
      data[i][j] /= c;
  return *this;
}

matrix& matrix::changelines(int i, int j)
{
  double* p = data[i];
  data[i] = data[j];
  data[j] = p;
  return *this;
}

matrix operator+(matrix a, const matrix& b)
{
  return a+=b;
}

matrix operator-(matrix a, const matrix& b)
{
  return a-=b;
}

matrix operator-(const matrix& a)
{
  matrix temp(a.lines, a.colomns);
  for(int i = 0; i < temp.lines; i++)
    for(int j = 0; j < temp.colomns; j++)
      temp.data[i][j] = 0;
  return temp-=a;
}

matrix operator*(matrix a, double c)
{
  return a*=c;
}

matrix operator*(double c, matrix a)
{
  return a*=c;
}

matrix operator*(matrix a, const matrix& b)
{
  return a*=b;
}

matrix operator/(matrix a, double c)
{
  return a/=c;
}

matrix transpose(const matrix& a)
{
  matrix temp(a.colomns, a.lines);
  for(int i = 0; i < a.lines; i++)
    for(int j = 0; j < a.colomns; j++)
      temp.data[j][i] = a.data[i][j];
  return temp;
}

matrix concat(const matrix& a, const matrix& b)
{
  if (a.lines != b.lines)
    errormessage("concat", "different size");
  matrix temp(a.lines, a.colomns + b.colomns);
  for(int i = 0; i < a.lines; i++)
    for(int j = 0; j < a.colomns; j++)
      temp.data[i][j] = a.data[i][j];
  for(int i = 0; i < b.lines; i++)
    for(int j = 0; j < b.colomns; j++)
      temp.data[i][j + a.colomns] = b.data[i][j];
  return temp;
}

bool nonzero(const matrix& a, double epsilon)
{
  for(int i = 0; i < a.lines; i++)
    for(int j = 0; j < a.colomns; j++)
      if (a.data[i][j] > epsilon)
	return 1;
  return 0;
};

/////////////////////////////////// vectors ///////////////////////////////////

vector::vector(int length):
  matrix(length, 1)
{}

vector::vector(const vector& a):
  matrix(a)
{}

vector::vector(const matrix& a):
  matrix(a)
{
  if (a.colomns != 1)
    errormessage("vector::vector(const matrix&)", "different size");
}

vector::~vector()
{}

int vector::length()
{
  return lines;
}

double& vector::operator[](int i)
{
  return data[i][0];
}

covector transpose(const vector& a)
{
  return covector(transpose(matrix(a)));
}

vector transpose(const covector& a)
{
  return vector(transpose(matrix(a)));
}

double sqr(const vector& a)
{
  return transpose(a)*a;
}

double norm(const vector& a)
{
  return sqrt(sqr(a));
}

vector operator+(const vector& a, const vector& b)
{
  return vector(matrix(a) + matrix(b));
}

vector operator-(const vector& a, const vector& b)
{
  return vector(matrix(a) - matrix(b));
}

vector operator*(const matrix& a, const vector& b)
{
  return vector(a * matrix(b));
}

double operator*(const covector& a, const vector& b)
{
  matrix temp = matrix(a)*matrix(b);
  return temp.data[0][0];
}

vector operator*(double c, const vector& a)
{
  return vector(c*matrix(a));
}

vector operator*(const vector& a, double c)
{
  return vector(c*matrix(a));
}

//gauss algaritm
vector operator/(const vector& v, const matrix& m)
{
  if (m.lines != m.colomns || v.lines != m.colomns)
    errormessage("operator/(const vector&, const matrix&)", "different size");
  matrix tm = concat(m, v);  //tm = Temp Matrix
  //row echelon form alroritm
  int i=0, j, isub, jsub, imax;
  for (j = 0; j < tm.colomns && i < tm.lines-1; j++)
    {
      imax = i;
      for (isub = i + 1; isub < tm.lines; isub++)
	if (fabs(tm.data[isub][j]) > fabs(tm.data[imax][j]))
	  imax = isub;
      if (fabs(tm.data[imax][j]) > EPSILON && i < tm.lines-1)
	{
	  tm.changelines(i, imax);
	  for (isub = i + 1; isub < tm.lines; isub++)
	    for (jsub = tm.colomns - 1; jsub >= j; jsub--)
	      tm.data[isub][jsub] -= tm.data[i][jsub] * \
		tm.data[isub][j] / tm.data[i][j];
	  ++i;
	}
    }
  if (fabs(tm.data[tm.lines-1][tm.lines-1]) < EPSILON)
    errormessage("operator/(const  vector&,  const  matrix&)",\
		 "small  rang of matrix, infinity count of solution");
  //backword elimination
  vector tv(tm.lines); //Temp Vector
  for (i = tm.lines - 1; i >= 0; --i)
    {
      tv[i] = tm.data[i][tm.colomns - 1];
      for (j = tm.lines - 1; j > i; --j)
	tv[i] -= tm.data[i][j] * tv[j];
      tv[i] /= tm.data[i][i];
    }
  return tv;
}

/////////////////////////////////// covectors (transposed vectors) //////////////////////////////////

covector::covector(int length):
  matrix(1, length)
{}

covector::covector(const covector& a):
  matrix(a)
{}

covector::covector(const matrix& a):
  matrix(a)
{
  if (a.lines != 1)
    errormessage("covector::covector(const matrix&)", "different size");
}

covector::~covector()
{}

int covector::length()
{
  return colomns;
}

double& covector::operator[](int i)
{
  return data[0][i];
}

double sqr(const covector& a)
{
  return a*transpose(a);
}

double norm(const covector& a)
{
  return sqrt(sqr(a));
}

covector operator+(const covector& a, const covector& b)
{
  return covector(matrix(a) + matrix(b));
}

covector operator-(const covector& a, const covector& b)
{
  return covector(matrix(a) - matrix(b));
}

covector operator*(const covector& a, const matrix& b)
{
  return covector(matrix(a) * b);
}

covector operator*(double c, const covector& a)
{
  return covector(c*matrix(a));
}

covector operator*(const covector& a, double c)
{
  return covector(c*matrix(a));
}

#include "utils.h"

bool
colors_equal (Color a, Color b)
{
  return a.a == b.a && a.r == b.r && a.g == b.g && a.b == b.b;
}

int
binomial (int n, int k)
{
  int i, j, s;
  if (n < k)
    return 0;
  if (k > n / 2)
    k = n - k;
  for (s = 1, i = n, j = 1; i != n - k; i--, j++)
    {
      s *= i;
      s /= j;
    }
  return s;
}

#include <iostream>

#include "add.h"

float sum(float a, float b)
{
  std::cout <<" Called sum with " << a << " and " << b << std::endl;
  float s = add(a, b);
  return(s);
}

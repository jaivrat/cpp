#include <iostream>

float add(float a, float b)
{
  std::cout <<" Called add with " << a << " and " << b << std::endl;
  float s = a+b;
  return(s);
}

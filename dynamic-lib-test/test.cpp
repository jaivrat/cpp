#include <iostream>



#include "sum.h"

int main()
{
  float x = 2.0;
  float y = 5.0;
  float val = sum(x,y);
  std::cout << "Calculated result " << val << std::endl;
  return(0); 
}

#include <iostream>

int main ( int, const char**, char** )
{
	std::cout << "Undetected\n";

	new int;
}
// g++ -std=c++17 -L. -lxepl intro0.cpp -o intro0

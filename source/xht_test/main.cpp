#include <xht/all.hpp>
#include <iostream>

int main() {
	double dnan = std::numeric_limits<double>::quiet_NaN();
	float fnan = std::numeric_limits<float>::quiet_NaN();

	xht::StdHashMap<double, char> hm;

	hm.Insert(dnan, 'x');
	std::cout << hm.Find(0.0) << std::endl;
	
	return 0;
}



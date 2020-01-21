#include <xht/all.hpp>
#include <iostream>

namespace std {
const std::string_view SimplifyKeyExt(const std::string& key) {
	return key;
}
}

int main() {
	std::string s("test");

	xht::StdHashMap<std::string_view, int> hm2;
	hm2.Insert(s, 177);

	return 0;
}



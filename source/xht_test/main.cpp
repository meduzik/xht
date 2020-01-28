#include <xht/all.hpp>
#include <iostream>

namespace std {
const std::string_view SimplifyKeyExt(const std::string& key) {
	return key;
}
}

int main() {
	std::string s("test");

	xht::StdHashMap<std::string_view, int> hm;
	hm.Insert(std::string_view("abc"), 59);
	hm.Insert(std::string_view("tree"), 60);
	hm.Insert(std::string_view("qwer"), 61);

	return *hm.FindValue(std::string_view("tree")) + 40;
}



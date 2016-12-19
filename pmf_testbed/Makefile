all: gfp
	./$<

.PHONY: pmf-format clean gfp-format

clean:
	rm -f pmf gfp

pmf-format: pmf.cpp
	clang-format -i $<

pmf: pmf.cpp catch.hpp
	$(CXX) $< -g -std=c++11 -Wno-pmf-conversions -Wall -o $@

gfp: gfp.cpp gfp.hpp catch.hpp
	$(CXX) $< -g -std=c++11 -Wno-pmf-conversions -Wall -o $@

gfp-format: gfp.cpp gfp.hpp
	clang-format -i $^

gfp-dumpfs: gfp
	nm -C $< |grep -e '\(T \|::\)f('

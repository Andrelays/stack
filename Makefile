CXX = g++

CXXFLAGS = -g -D -DCANARY_PROTECT_INCLUDED -DHASH_PROTECT_INCLUDED -DDEBUG_OUTPUT_STACK_DUMP -DDEBUG_OUTPUT_STACK_OK -DDEBUG -fsanitize=address -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wno-missing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel -Wtype-limits -Wwrite-strings -Werror=vla -D_DEBUG -D_EJUDGE_CLIENT_SIDE -DDEBUG

release: stack.o operation_files.o main.o
	@echo [CXX] [CXXFLAGS] $^ -o $@
	@$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	@echo [CXX] [CXXFLAGS] $^ -o $@
	@$(CXX) $(CXXFLAGS) -c $^

clean:
	rm -rf *.o stack


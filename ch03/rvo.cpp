#include <iostream>

struct LargeClass {
    int i;
    char c;
    double d;
};

auto rvoExample(int i, char c, double d) {
    return LargeClass{i, c, d};
}

int main(int argc, char const *argv[])
{
    LargeClass lc_obj = rvoExample(10, 'c', 3.14);
    printf("Success\n");
    return 0;
}

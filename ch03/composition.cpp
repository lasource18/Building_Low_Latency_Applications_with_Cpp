#include <cstdio>
#include <vector>

struct Order { int id; double price; };
class InheritanceOrderBook : public std::vector<Order> {};
class CompositionOrderBook {
    std::vector<Order> orders;

    public:
        auto size() const noexcept {
            return orders.size();
        }
};

int main(int argc, char const *argv[])
{
    InheritanceOrderBook i_book;
    CompositionOrderBook c_book;
    printf("InheritanceOrderBook::size(): %lu, CompositionOrderBook::size(): %lu\n", i_book.size(), c_book.size());
    return 0;
}

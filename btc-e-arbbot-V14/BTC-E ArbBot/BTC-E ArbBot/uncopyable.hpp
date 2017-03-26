#ifndef UNCOPYABLE_HPP
#define UNCOPYABLE_HPP

class uncopyable {
    uncopyable(const uncopyable&) {}
    uncopyable& operator=(const uncopyable&) { return *this; }
public:
    uncopyable() {}
    ~uncopyable() {}
};

#endif // UNCOPYABLE_HPP

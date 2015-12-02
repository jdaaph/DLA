#include <cmath>
using namespace std;

struct Vec2D{
    
    int x;
    int y;

    Vec2D(const int& _x, const int& _y, const int& _z) : x(_x), y(_y);
    Vec2D(const Vec2D& a): x(a.x), y(a.y);
};

// plus
inline Vec2D operator+(const Vec2D& a, const Vec2D& b){
    return Vec2D(a.x + b.x, a.y + b.y);
}

// substract
inline Vec2D operator-(const Vec2D& a, const Vec2D& b){
    return Vec2D(a.x - b.x, a.y - b.y);
}


// assignment-add
inline Vec2D operator+=(const Vec2D& a, const Vec2D& b){
    a.x += b.x;
    a.y += b.y;
    return a;
}

// assignment-substraction
inline Vec2D operator-=(const Vec2D& a, const Vec2D& b){
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

// equality test
inline bool operator==(const Vec2D& a, const Vec2D& b){
    return (  (a.x == b.x) && (a.y == b.y) ) ;
}

// inequality test
inline bool operator!=(const Vec2D& a, const Vec2D& b){
    return (  (a.x != b.x) || (a.y != b.y) ) ;
}




/*
    Vector Math Section
*/


// dot product
inline int dot(const Vec2D& a, const Vec2D& b){
    return (  a.x * b.x + a.y * b.y ) ;
}


/*
    Special Operations
*/

// min vector constructed from the min of each component
inline Vec2D min(const Vec2D& a, const Vec2D& b){
    return Vec2D (  min(a.x, b.x),
                    min(a.y, b.y),
                 );
}

// max vector constructed from the max of each component
inline Vec2D max(const Vec2D& a, const Vec2D& b){
    return Vec2D (  max(a.x, b.x),
                    max(a.y, b.y),
                 );
}

// midpoint vector is the midpoint of two lovely vectors
inline Vec2D midpoint(const Vec2D& a, const Vec2D& b){
    return Vec2D (  (a.x + b.x)/2.0,
                    (a.y + b.y)/2.0,
                 );
}













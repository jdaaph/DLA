#include <cmath>
#include <algorithm>
using namespace std;

#ifndef __VECTOR_MATH_H__
#define __VECTOR_MATH_H__

struct Vec2D{
public: 
    int x;
    int y;

    Vec2D(const int& _x, const int& _y) : x(_x), y(_y)
    {
    }
    
    // Vec2D(const Vec2D& a): x(a.x), y(a.y)
    // {
    // }
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
inline Vec2D operator+=(Vec2D& a, const Vec2D& b){
    a.x += b.x;
    a.y += b.y;
    return a;
}

// assignment-substraction
inline Vec2D operator-=(Vec2D& a, const Vec2D& b){
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
    return Vec2D (  std::min(a.x, b.x),
                    std::min(a.y, b.y)
                 );
}

// max vector constructed from the max of each component
inline Vec2D max(const Vec2D& a, const Vec2D& b){
    return Vec2D (  std::max(a.x, b.x),
                    std::max(a.y, b.y)
                 );
}


// midpoint vector is the midpoint of two lovely vectors
inline Vec2D midpoint(const Vec2D& a, const Vec2D& b){
    return Vec2D (  (a.x + b.x)/2.0,
                    (a.y + b.y)/2.0
                 );
}


// get area from upper and lower
inline int get_area(const Vec2D& a, const Vec2D& b){
    return ( floor(fabs((a.x - b.x) * (a.y - b.y) )));
}


// get the square of distance between two vectors
inline int get_distance2(const Vec2D& a, const Vec2D& b){
    return ( (a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y));
}

// get the distance between two vectors
inline int get_distance(const Vec2D& a, const Vec2D& b){
    return sqrt( (a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y));
}

// get distance to the origin
inline float get_r(const Vec2D& a){
    return sqrt( a.x * a.x + a.y * a.y);
}


// get distance to the origin, int form
inline float get_r(const int& x, const int& y){
    return sqrt( x * x + y * y);
}


// inline Vec2D rank2xy(int rank, int num_active_core);
// inline int xy2rank(Vec2D xy, int num_active_core);

// helper function for processor grid <-> rank conversion
inline Vec2D rank2xy(int rank, int num_active_core){
    int l = floor(sqrt(num_active_core));
    // if (l != sqrt(num_active_core)){
    //     cout << "Error! not perfect square number of cores are allocated @.@?" << endl;
    //     return;
    // }
    return Vec2D( rank / l, rank % l);
}


// helper function for processor grid <-> rank conversion
inline int xy2rank(Vec2D xy, int num_active_core){
    int l = floor(sqrt(num_active_core));
    int rank = (xy.x * l + xy.y);

    // if core does not exist, return -1
    if (rank < 0 || rank >= num_active_core) return -1;
    return rank;
}


#endif //__VECTOR_MATH_H__









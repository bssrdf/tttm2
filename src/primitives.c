#include "primitives.h"

/////////////
// POINT_T //
/////////////

// converts an point p from cartesian space
// to baricentric space of triangle abc
//    b
//   / \
//  / p \
// a-----c
// cost: *8 +9

point_t
point_to_baricentric(
    const point_t* p,
    const point_t* a, 
    const point_t* b, 
    const point_t* c){
    
    point_t d1 = (point_t) {a->x - c->x, a->y - c->y};
    point_t d2 = (point_t) {b->x - c->x, b->y - c->y};
    point_t d3 = (point_t) {p->x - c->x, p->y - c->y};
    
    double k = (d2.y*d1.x-d2.x*d1.y);
    double u = (d2.y*d3.x-d2.x*d3.y)/k;
    double v = (d1.x*d3.y-d1.y*d3.x)/k;
    
    return (point_t) {u, v};
}

// converts an point p(u,w) 
// from an baricentric triangle space abc
// to cartesian space
point_t
point_to_cartesian(
    const point_t* p,
    const point_t* a,
    const point_t* b,
    const point_t* c){

    double w = 1 - p->x - p->y;
    return (point_t) {
        a->x * p->x + b->x * p->y + c->x*w,
        a->y * p->x + b->y * p->y + c->y*w};
}

bool
point_is_inside_bc(
    const point_t* p,
    const point_t* a,
    const point_t* b,
    const point_t* c) {

    point_t bc = point_to_baricentric(p, a, b, c);
    return (bc.x >= 0 && bc.y >= 0 && bc.x+bc.y<=1) ? true : false;
}

//////////////
// TRIANGLE //
//////////////

void
triangle_by_incircle(point_t t[3], point_t o, double r){
    t[0].x = o.x          ; t[0].y = o.y + r*2;
    t[1].x = o.x + r*SQRT3; t[1].y = o.y - r  ;
    t[2].x = o.x - r*SQRT3; t[2].y = o.y - r  ;
}
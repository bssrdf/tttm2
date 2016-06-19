#include <assert.h>
#include <stdio.h>

#include "mesh.h"

mesh_t*
mesh_new() {
    mesh_t* m = calloc(1, sizeof(mesh_t));
    kv_init(m->points);
    kv_init(m->triangles);
    return m;
}

void
mesh_free(mesh_t* m) {
    assert(m!=NULL);
    kv_destroy(m->triangles);
    kv_destroy(m->points);
    free(m);
}

uint32_t
mesh_add_point(mesh_t* m, point_t p) {
    kv_push(point_t, m->points, p);
    return kv_size(m->points)-1;
}

uint32_t
mesh_add_triangle(mesh_t* m, triangle_t t) {
    kv_push(triangle_t, m->triangles, t);
    return kv_size(m->triangles)-1;
}

////////////////
/// EAR CLIP ///
////////////////

typedef struct part_point_t part_point_t;
typedef struct part_point_t {
    point_t p;
    bool is_active;
    bool is_convex;
    bool is_ear;
    double angle;
    part_point_t* prev;
    part_point_t* next;
} part_point_t;

static void
update_point(
    part_point_t* v,
    part_point_t* points,
    uint32_t points_count) {

    part_point_t *v1, *v3;
    point_t vec1, vec3;

    v1 = v->prev;
    v3 = v->next;

    v->is_convex = point_is_convex(&v1->p, &v->p, &v3->p);
    vec1 = point_dif(&v1->p, &v->p);
    vec3 = point_dif(&v3->p, &v->p);
    vec1 = point_nrm(&vec1);
    vec3 = point_nrm(&vec3);

    v->angle = vec1.x*vec3.x + vec1.y*vec3.y;

    if(v->is_convex) {
        v->is_ear = true;
        for(uint32_t j=0; j<points_count; j++) {
            if((points[j].p.x ==  v->p.x) && (points[j].p.y ==  v->p.y)) continue;
            if((points[j].p.x == v1->p.x) && (points[j].p.y == v1->p.y)) continue;
            if((points[j].p.x == v3->p.x) && (points[j].p.y == v3->p.y)) continue;
            if(point_is_inside(&v1->p, &v->p, &v3->p, &points[j].p)) {
                v->is_ear = false;
                break;
            }
        }
    } else {
        v->is_ear = false;
    }
}

mesh_t*
mesh_from_shape(shape_t* shape) {
    mesh_t* mesh = mesh_new();
    for(uint32_t i=0; i<kv_size(shape->polys); i++) {
        poly_t* poly = &kv_A(shape->polys, i);
        int8_t err = mesh_poly(mesh, &shape->points, poly);
        if(err) {
            fprintf(stderr, "Something wrong with part %d of shape\n", i);
            mesh_free(mesh);
            return NULL;
        }
    }
    return mesh;
}

mesh_t*
mesh_from_hull(shape_t* shape) {
    
}

// sensitive to direction cw/ccw
int8_t 
mesh_poly(mesh_t*         mesh, 
          const points_v* p, 
          const poly_t*   poly) {
    
    uint32_t s = poly->s;
    uint32_t l = poly->l;

    if(l < 3) return 1;
    
    if(l == 3) {
        mesh_add_triangle(mesh, (triangle_t) {
            mesh_add_point(mesh, kv_A(*p, s+0)),
            mesh_add_point(mesh, kv_A(*p, s+1)),
            mesh_add_point(mesh, kv_A(*p, s+2)),
        });
        return 0;
    }

    // setup points
    part_point_t points[l];
    for(uint32_t i=0; i<l; i++) {
        points[i].is_active = true;
        points[i].p = kv_A(*p, s+i);
        if(i==l-1) points[i].next = &(points[  0]);
        else       points[i].next = &(points[i+1]);
        if(i==0)   points[i].prev = &(points[l-1]);
        else       points[i].prev = &(points[i-1]);
    }

    for(uint32_t i=0; i<l; i++) {
        update_point(&points[i], points, l);
    }

    for(uint32_t i=0; i<l-3; i++) {

        part_point_t* ear;
        bool earfound = false;
        //find the most extruded ear
        for(uint32_t j=0;j<l;j++) {
            if(!points[j].is_active) continue;
            if(!points[j].is_ear   ) continue;
            if(!earfound) {
                earfound = true;
                ear = &(points[j]);
            } else {
                if(points[j].angle > ear->angle) {
                    ear = &(points[j]);
                }
            }
        }
        // printf("%d %d earsfound %d\n", i, l, earfound);
        if(!earfound) {
            fprintf(stderr, "Err, ears not found\nAt point %d / %d\n", i, l);
            return 1;
        }

        mesh_add_triangle(mesh, (triangle_t) {
            mesh_add_point(mesh, ear->prev->p),
            mesh_add_point(mesh, ear->p      ),
            mesh_add_point(mesh, ear->next->p),
        });

        ear->is_active = false;
        ear->prev->next = ear->next;
        ear->next->prev = ear->prev;

        if(i==l-4) break;

        update_point(ear->prev, points, l);
        update_point(ear->next, points, l);
    }

    for(uint32_t i=0; i<l; i++) {
        if(points[i].is_active) {
            // triangle.Triangle(points[i].prev->p,points[i].p,points[i].next->p);
            // triangles->push_back(triangle);
            mesh_add_triangle(mesh, (triangle_t) {
                mesh_add_point(mesh, points[i].prev->p),
                mesh_add_point(mesh, points[i].p      ),
                mesh_add_point(mesh, points[i].next->p),
            });
            break;
        }
    }
    return 0;
}

void
mesh_write(const mesh_t* mesh, FILE* fp) {
    
    assert(mesh != NULL && fp != NULL);

    point_t* p;
    for(uint32_t i=0; i<kv_size(mesh->triangles); i++) {
        triangle_t* t = &kv_A(mesh->triangles, i);
        p = &kv_A(mesh->points, t->a); fprintf(fp, "%f %f\n", p->x, p->y);
        p = &kv_A(mesh->points, t->b); fprintf(fp, "%f %f\n", p->x, p->y);
        p = &kv_A(mesh->points, t->c); fprintf(fp, "%f %f\n", p->x, p->y);
        fprintf(fp, "\n");
    }    
}
#include "main.h"

#define BUFFERS 10
#define max(a,b) ((a)>(b)?(a):(b))

typedef struct {
    float x, y;
    float r, g, b;
} vertex_t;
typedef kvec_t(vertex_t) vertices_v;

static void load_mesh(char* filename);

int current_buffer;
GLuint vertex_buffers[BUFFERS];
vertices_v vertices[BUFFERS];

void setBuffer();

// initpoint
void app_init(int argc, char** argv) {
    // MESH //
    if(argc>1){
        for(int i=0; i<BUFFERS; i++) {
            kv_init(vertices[i]);
        }
        load_mesh(argv[1]);
        for(int i=0; i<BUFFERS; i++) {
            printf("Buffer Data %d, %p\n",i, vertices[i].a);
        }
        glGenBuffers(BUFFERS, vertex_buffers);


        for(int i=0; i<BUFFERS; i++) {
            if(vertices[i].n == 0) continue;

            printf("Loaded %d: %zu points\n", i, vertices[i].n);
            printf("Bind %d buffer id: %d\n", i, vertex_buffers[i]);
            glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[i]);
            
            printf("Buffer Data %d, %p\n",i, vertices[i].a);
            glBufferData(GL_ARRAY_BUFFER, 
                vertices[i].n * sizeof(vertex_t), 
                vertices[i].a, GL_STATIC_DRAW);
            // printf("Unbind %d\n", i);
            // glBindBuffer(GL_ARRAY_BUFFER, 0);

        }

        current_buffer = 5;
        setBuffer();
    }
}



// draw loop
void app_draw() {
    glDrawArrays(GL_TRIANGLES, 0, vertices[current_buffer].n);
}

// cleanup at the end
void app_cleanup() {
    // cleanup
    fprintf(stderr, "Cleanup");
    glDeleteBuffers(BUFFERS, vertex_buffers);
    for(int i=0;i<10; i++) {
        kv_destroy(vertices[i]);
    }
}

void on_error(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

void on_key(GLFWwindow* window, int key, int scancode, int action, int mods) {
    
    // fprintf(stderr, "key: %d scan: %d act: %d mods: %d\n", key, scancode, action, mods);
    // fprintf(stderr, "%d %d\n", posx, posy);

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (key == GLFW_KEY_LEFT  && (action == GLFW_PRESS || action == GLFW_REPEAT)) posx-=0.05;
    if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)) posx+=0.05;
    if (key == GLFW_KEY_UP    && (action == GLFW_PRESS || action == GLFW_REPEAT)) posy+=0.05;
    if (key == GLFW_KEY_DOWN  && (action == GLFW_PRESS || action == GLFW_REPEAT)) posy-=0.05;
    if (key == GLFW_KEY_MINUS && (action == GLFW_PRESS || action == GLFW_REPEAT)) scale-=0.2;
    if (key == GLFW_KEY_EQUAL && (action == GLFW_PRESS || action == GLFW_REPEAT)) scale+=0.2;

    if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9 && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        current_buffer = key - GLFW_KEY_0;
        setBuffer();
    }

}

void on_mouse(GLFWwindow* window, double xpos, double ypos){
}

void on_click(GLFWwindow* window, int button, int action, int mods) {
}

void on_scroll(GLFWwindow* window, double xoffset, double yoffset) {
    // printf("%f, %f\n", xoffset, yoffset);
    scale+=0.2*yoffset;
}

void setBuffer() {
    // change buffer object
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[current_buffer]);        
    setVertexAttr();
}

static void 
load_mesh(char* filename) {

    FILE* fp;
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(filename, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    float minx, miny, maxx, maxy, cx, cy, w, h;
    minx = HUGE_VAL; miny = HUGE_VAL;
    maxx =-HUGE_VAL; maxy =-HUGE_VAL;
    while ((read = getline(&line, &len, fp)) != -1) {
        if(line[0]=='\n') continue;
        char* e;
        double x = strtod(line, &e); 
        double y = strtod(   e, &e);
        int    d = strtol(   e, &e, 10);
        if(d<0 || d>9) continue;

        if(x>maxx) maxx=x; if(y>maxy) maxy=y;
        if(x<minx) minx=x; if(y<miny) miny=y;
        vertex_t v = (vertex_t) { x, y };
        kv_push(vertex_t, vertices[d], v);
    }

    w = maxx-minx;
    h = maxy-miny;
    cx = minx + w/2.0;
    cy = miny + h/2.0;

    printf("min %f %f\n", minx, miny);
    printf("max %f %f\n", maxx, maxy);
    printf("siz %f %f\n", w, h);
    printf("cnt %f %f\n", cx, cy);

    // FIT model to 1x1 box
    double scale = 2.0/max(w,h);
    printf("scale %f\n", scale);
    for(uint i=0; i<BUFFERS; i+=1){
        for(uint j=0; j<vertices[i].n; j++) {
            vertices[i].a[j].x = (vertices[i].a[j].x - cx)*scale; 
            vertices[i].a[j].y = (vertices[i].a[j].y - cy)*scale;
            vertices[i].a[j].r = ((j    )%256)/255.0;
            vertices[i].a[j].g = ((j+64 )%256)/255.0;
            vertices[i].a[j].b = ((j+128)%256)/255.0;
            // printf("%f %f %s",vertices->a[i].x, vertices->a[i].y, (i%8)?"":"\n");
        }
    }

    fclose(fp);
    if (line)
        free(line);
}

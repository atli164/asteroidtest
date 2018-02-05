#include <iostream>
#include <vector>
#include <algorithm>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

const double FPS = 60;
const double pi = acos(-1);

double frand(double fmin, double fmax) {
    double f = (double)rand() / RAND_MAX;
    return fmin + f * (fmax - fmin);
}

double zoom = 1.0;
int scr_x, scr_y;

ALLEGRO_DISPLAY *dsp = NULL;
ALLEGRO_EVENT_QUEUE *evq = NULL;
ALLEGRO_TIMER *tmr = NULL;
ALLEGRO_COLOR white;
ALLEGRO_COLOR brown;
ALLEGRO_COLOR red;

struct ship;
struct asteroid;
struct astfield;
struct laser;

// No x, y position since ship is by
// default origin of coordinates
struct ship {
    double dx, dy, dv, ddv, phi, dphi, cooldown;
    bool firing, hit;
    ship() {
        dx = 0;
        dy = 0;
        dv = 0;
        ddv = 0;
        phi = pi / 2;
        dphi = 0;
        firing = false;
        hit = false;
        cooldown = 0;
    }

    void update(astfield &a) {
        dv += ddv;
        dv = std::max(dv, -1.0);
        dv = std::min(dv, 5.0);
        phi += dphi;
        dx = cos(phi) * dv;
        dy = sin(phi) * dv;
        cooldown = std::max(0.0, cooldown - 1 / FPS);
        if(cooldown == 0.0) {
            a.add_lsr();
            cooldown = 1;
        }
    }

    void draw() {
        double topx = 30 * cos(phi);
        double topy = 30 * sin(phi);
        double bot1x = 30 * cos(phi + pi + pi / 12);
        double bot1y = 30 * sin(phi + pi + pi / 12);
        double bot2x = 30 * cos(phi + pi - pi / 12);
        double bot2y = 30 * sin(phi + pi - pi / 12);
        al_draw_filled_triangle(topx, topy, bot1x, bot1y, bot2x, bot2y, white);
    }
};

ship player;

struct asteroid {
    double x, y, r, dx, dy;
    bool todel;
    asteroid(double _x, double _y, double _r) {
        x = _x;
        y = _y;
        r = frand(25, _r);
        dx = frand(0.5, 2);
        dy = frand(0.5, 2);
        todel = false;
    }

    // Returns whether it should be deleted
    void update() {
        x -= player.dx;
        y -= player.dy;
        double dist = hypot(x, y);
        if(dist < r) {
            player.hit = true;
        }
        todel = dist > 10000;
    }

    void draw() {
        al_draw_filled_circle(x, y, r, brown);
    }
};

struct laser {
    double x, y, dx, dy;
    bool todel;
    laser() {
        x = 0;
        y = 0;
        dx = 10 * cos(player.phi);
        dy = 10 * sin(player.phi);
        todel = false;
    }
    
    // Returns whether it should be deleted
    bool update() {
        x -= player.dx;
        y -= player.dy;
        double dist = hypot(x, y);
        todel = dist > 10000; 
    }

    void draw() {
        al_draw_filled_circle(x, y, 5, red);
    }
};

struct astfield {
    std::vector<asteroid> asts;
    std::vector<laser> lsrs;
    astfield() {}

    void add_ast(double _x, double _y) {
        asts.push_back(asteroid(_x, _y, 150));
    }

    void add_lsr() {
        lsrs.push_back(laser());
    }
    
    void erase_asts() {
        asts.erase(std::remove_if(asts.begin(), asts.end(),
                    [](const asteroid &a) { return a.todel; }), asts.end());
    }

    void erase_lsrs() {
        lsrs.erase(std::remove_if(lsrs.begin(), lsrs.end(),
                    [](const laser &l) { return l.todel; }), lsrs.end());
    }

    void update() {
        for(asteroid a : asts) {
            a.update();
        }
        for(laser l : lsrs) {
            l.update();
        }
        erase_asts();
        erase_lsrs();
        std::vector<asteroid> newasts;
        for(asteroid a : asts) {
            for(laser l : lsrs) {
                if(hypot(l.x - a.x, l.y - a.y) < a.r) {
                    a.todel = true;
                    l.todel = true;
                    if(a.r >= 50.0) {
                        double randang = frand(0, 2 * pi);
                        double posx1 = a.x + a.r * cos(randang) / 2;
                        double posy1 = a.y + a.r * sin(randang) / 2;
                        randang += pi;
                        double posx2 = a.x + a.r * cos(randang) / 2;
                        double posy2 = a.y + a.r * sin(randang) / 2;
                        newasts.push_back(asteroid(posx1, posy1, a.r / 2));
                        newasts.push_back(asteroid(posx2, posy2, a.r / 2));
                    }
                }
            }
        }
        erase_asts();
        erase_lsrs();
        for(asteroid a : newasts) {
            asts.push_back(a);
        }
    }

    void draw() {
        for(asteroid a : asts) {
            a.draw();
        }
        for(laser l : lsrs) {
            l.draw();
        }
    }
};

astfield field;

void destruction() {
    al_destroy_timer(tmr);
    al_destroy_display(dsp);
    al_destroy_event_queue(evq);
}

void drawframe(bool &draw) {
    if(draw && al_is_event_queue_empty(evq)) {
        al_clear_to_color(al_map_rgb(0, 0, 0));
        field.draw();
        player.draw();
        al_flip_display();
        draw = false;
    }
}

int main(int argc, char **argv) {
    srand(time(NULL));
    
    if(!al_init()) {
        std::cerr << "Failed to initialize Allegro." << std::endl;
        destruction();
        return -1;
    }

    if(!al_init_image_addon()) {
        std::cerr << "Failed to initialize image addon." << std::endl;
        destruction();
        return -1;
    }

    if(!al_init_primitives_addon()) {
        std::cerr << "Failed to initialize primitives addon." << std::endl;
        destruction();
        return -1;
    }

    if(!al_install_keyboard()) {
        std::cerr << "Failed to install keyboard." << std::endl;
        destruction();
        return -1;
    }

    tmr = al_create_timer(1.0 / FPS);
    if(!tmr) {
        std::cerr << "Failed to make timer." << std::endl;
        destruction();
        return -1;
    }

    scr_x = al_get_display_width(dsp);
    scr_y = al_get_display_height(dsp);
    al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
    dsp = al_create_display(scr_x, scr_y);
    if(!dsp) {
        std::cerr << "Failed to create display." << std::endl;
        destruction();
        return -1;
    }

    evq = al_create_event_queue();
    if(!evq) {
        std::cerr << "Failed to create event queue." << std::endl;
        destruction();
        return -1;
    }

    al_register_event_source(evq, al_get_display_event_source(dsp));
    al_register_event_source(evq, al_get_timer_event_source(tmr));
    al_register_event_source(evq, al_get_keyboard_event_source());

    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_flip_display();

    al_start_timer(tmr);

    white = al_map_rgb(255, 255, 255);
    brown = al_map_rgb(150, 85, 0);
    red = al_map_rgb(255, 0, 0);

    bool draw = true;

    while(true) {
        ALLEGRO_EVENT evt;
        ALLEGRO_TIMEOUT tmout;

        al_init_timeout(&tmout, 1 / FPS);

        bool gevt = al_wait_for_event_until(evq, &evt, &tmout);

        if(gevt) {
            switch(evt.type) {
                case ALLEGRO_EVENT_TIMER:
                    draw = true;
                    break;
                case ALLEGRO_EVENT_DISPLAY_CLOSE:
                    destruction();
                    return 0;
                case ALLEGRO_EVENT_KEY_DOWN:
                    switch(evt.keyboard.keycode) {
                        case ALLEGRO_KEY_UP:
                            player.ddv = 0.01;
                            break;
                        case ALLEGRO_KEY_DOWN:
                            player.ddv = -0.01;
                            break;
                        case ALLEGRO_KEY_LEFT:
                            player.dphi = 0.1;
                            break;
                        case ALLEGRO_KEY_RIGHT:
                            player.dphi = -0.1;
                            break;
                        case ALLEGRO_KEY_SPACE:
                            player.firing = true;
                            break;
                    }
                    break;
                case ALLEGRO_EVENT_KEY_UP:
                    switch(evt.keyboard.keycode) {
                        case ALLEGRO_KEY_UP:
                            if(player.ddv > 0) player.ddv = 0;
                            break;
                        case ALLEGRO_KEY_DOWN:
                            if(player.ddv < 0) player.ddv = 0;
                            break;
                        case ALLEGRO_KEY_LEFT:
                            if(player.dphi > 0) player.dphi = 0;
                            break;
                        case ALLEGRO_KEY_RIGHT:
                            if(player.dphi < 0) player.dphi = 0;
                            break;
                        case ALLEGRO_KEY_SPACE:
                            player.firing = false;
                            break;
                        case ALLEGRO_KEY_ESCAPE:
                            destruction();
                            return 0;
                    }
                    break;
            }
        }
        drawframe(draw);
    }
}


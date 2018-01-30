#include <iostream>
#include <vector>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

const double FPS = 60;
const double pi = acos(-1);
const int init_sz = 200;

enum ctrlkeys {
    up, down, left, right, fire
};

bool keys[5] = {false, false, false, false, false};

double x = 0, y = 0;
double dx = 0, dy = 0;
double theta = 0; 
bool firing = false;
double zoom = 1.0;
int scr_x, scr_y;
double scale_w, scale_h, scale_x, scale_y;

ALLEGRO_DISPLAY *dsp = NULL;
ALLEGRO_BITMAP *buf = NULL;
ALLEGRO_EVENT_QUEUE *evq = NULL;
ALLEGRO_TIMER *tmr = NULL;
ALLEGRO_BITMAP *shp = NULL;

void destruction() {
    al_destroy_timer(tmr);
    al_destroy_display(dsp);
    al_destroy_bitmap(buf);
    al_destroy_event_queue(evq);
    al_destroy_bitmap(shp);
}

void drawframe(bool &draw) {
    if(draw && al_is_event_queue_empty(evq)) {
        al_set_target_bitmap(buf);
        al_clear_to_color(al_map_rgb(0, 0, 0));
        al_draw_bitmap(shp, x, y, 0);
        al_set_target_backbuffer(dsp);
        al_clear_to_color(al_map_rgb(0, 0, 0));
        al_draw_scaled_bitmap(buf, 0, 0, x - scr_x / 2, y - scr_y / 2, scale_x, scale_y, scale_w, scale_h, 0);
        draw = false;
    }
}

int main(int argc, char **argv) {
    
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

    al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
    dsp = al_create_display(init_sz, init_sz);
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

    shp = al_load_bitmap("ship.png");
    if(!shp) {
        std::cerr << "Failed to ship load bitmap." << std::endl;
        destruction();
        return -1;
    }

    scr_x = al_get_display_width(dsp);
    scr_y = al_get_display_height(dsp);
    double scale_x = scr_x / init_sz;
    double scale_y = scr_y / init_sz;
    double scale_mn = std::min(scale_x, scale_y);
    scale_w = init_sz * scale_mn;
    scale_h = init_sz * scale_mn;
    scale_x = (scr_x - scale_w) / 2;
    scale_y = (scr_y - scale_h) / 2;

    buf = al_create_bitmap(scr_x, scr_y);
    if(!buf) {
        std::cerr << "Failed to initialize buffer bitmap." << std::endl;
        destruction();
        return -1;
    }

    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_flip_display();

    al_start_timer(tmr);

    bool run = true;
    bool draw = true;

    while(run) {
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
                    run = false;
                    break;
                case ALLEGRO_EVENT_KEY_DOWN:
                    switch(evt.keyboard.keycode) {
                        case ALLEGRO_KEY_UP:
                            keys[up] = true;
                            break;
                        case ALLEGRO_KEY_DOWN:
                            keys[down] = true;
                            break;
                        case ALLEGRO_KEY_LEFT:
                            keys[left] = true;
                            break;
                        case ALLEGRO_KEY_RIGHT:
                            keys[right] = true;
                            break;
                        case ALLEGRO_KEY_SPACE:
                            keys[fire] = true;
                            break;
                    }
                    break;
                case ALLEGRO_EVENT_KEY_UP:
                    switch(evt.keyboard.keycode) {
                        case ALLEGRO_KEY_UP:
                            keys[up] = false;
                            break;
                        case ALLEGRO_KEY_DOWN:
                            keys[down] = false;
                            break;
                        case ALLEGRO_KEY_LEFT:
                            keys[left] = false;
                            break;
                        case ALLEGRO_KEY_RIGHT:
                            keys[right] = false;
                            break;
                        case ALLEGRO_KEY_SPACE:
                            keys[fire] = false;
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

    al_destroy_timer(tmr);
    al_destroy_display(dsp);
    al_destroy_bitmap(buf);
    al_destroy_event_queue(evq);
    al_destroy_bitmap(shp);
    return 0;
}


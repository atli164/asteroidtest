#include <iostream>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

const float FPS = 60;

int main(int argc, char **argv) {
    
    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;
    ALLEGRO_TIMER *timer = NULL;
    ALLEGRO_BITMAP *image = NULL;
    
    if(!al_init()) {
        std::cerr << "Failed to initialize Allegro." << std::endl;
        return -1;
    }

    if(!al_init_image_addon()) {
        std::cerr << "Failed to initialize image addon." << std::endl;
        return -1;
    }
    
    timer = al_create_timer(1.0 / FPS);
    if(!timer) {
        std::cerr << "Failed to make timer." << std::endl;
        return -1;
    }

    display = al_create_display(400, 400);
    if(!display) {
        std::cerr << "Failed to create display." << std::endl;
        al_destroy_timer(timer);
        return -1;
    }

    event_queue = al_create_event_queue();
    if(!event_queue) {
        std::cerr << "Failed to create event queue." << std::endl;
        al_destroy_display(display);
        al_destroy_timer(timer);
    }

    image = al_load_bitmap("ship.png");
    if(!image) {
        std::cerr << "Failed to load bitmap." << std::endl;
        al_destroy_display(display);
        al_destroy_timer(timer);
        al_destroy_event_queue(event_queue);
        return -1;
    }

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));

    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_flip_display();

    al_start_timer(timer);

    bool running = true;
    bool draw = true;

    float x = 200, y = 200;
    float dx = 0, dy = 0;

    while(running) {
        ALLEGRO_EVENT event;
        ALLEGRO_TIMEOUT timeout;

        al_init_timeout(&timeout, 0.06);

        bool get_event = al_wait_for_event_until(event_queue, &event, &timeout);

        if(get_event) {
            switch(event.type) {
                case ALLEGRO_EVENT_TIMER:
                    draw = true;
                    break;
                case ALLEGRO_EVENT_DISPLAY_CLOSE:
                    running = false;
                    break;
                default:
                    std::cerr << "Unsupported event: " << event.type << std::endl;
                    break;
            }
        }

        if(draw && al_is_event_queue_empty(event_queue)) {
            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_draw_bitmap(image, x, y, 0);
            al_flip_display();
            draw = false;
        }
    }

    al_destroy_timer(timer);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
    al_destroy_bitmap(image);

    return 0;
}


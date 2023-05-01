#define RANGINE_IMPLEMENTATION
#include "rangine.h"

static bool quit = false;

vec4 color = {{{0.5, 0.8, 0.3, 1}}};

AABB aabb = {
    .pos = {{{400/3, 300/3}}},
    .size = {{{32, 32}}},
};
AABB aabb2 = {
    .pos = {{{400/2, 300/2}}},
    .size = {{{32, 32}}},
};
Line line = {
    .start = {{{0, 100}}},
    .end = {{{100, 100}}},
    .width = 3
};

i32 main(i32 argc, char *argv[]) {
    SDL_Window *window = rg_init("GAME", 800, 600);

    while(!quit) {

        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            default:
                break;
            }
        }

        rg_render_begin();

        rg_aabb_draw(aabb, color);
        rg_line_draw(line, color);
        rg_aabb_line_draw(aabb2, 3, color);

        aabb.pos.x = rm_wrapf(aabb.pos.x+1, 0, -aabb.size.x/2 + 800/3);

        rg_render_end(window);
    }

    rg_exit(window);

    return 0;
}

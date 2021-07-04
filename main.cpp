#include <SDL2/SDL_image.h>
#include <chrono>

#define LENGTH 12288
#define ROWLEN 128
#define DEFAULT_DELAY 1000
#define SCALE 8

void tick(bool *&cells, bool *&new_cells);
void update_buffer(bool *cells, uint32_t *buffer);
void draw(bool *cells, uint32_t *buffer, SDL_Texture *texture, SDL_Renderer *renderer);

int main() {
    bool *cells = new bool[LENGTH];
    bool *new_cells = new bool[LENGTH];
    uint32_t buffer[LENGTH] = {};

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Game of Life", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, ROWLEN*SCALE, LENGTH/ROWLEN*SCALE, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, ROWLEN, LENGTH/ROWLEN);

    auto prev_cycle = std::chrono::high_resolution_clock::now();
    bool running = true;
    bool paused = false;
    float delay = DEFAULT_DELAY;
    draw(cells, buffer, texture, renderer);
    while(running) {
        auto current_time = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(current_time - prev_cycle).count();

        if(dt > delay) {
            prev_cycle = current_time;
            if(!paused)
                tick(cells, new_cells);
            draw(cells, buffer, texture, renderer);
        }

        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT:
                    running = false;
                    SDL_Quit();
                    break;
                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym) {
                        case SDLK_c:
                            delete[] cells;
                            cells = new bool[LENGTH]{};
                            break;
                        case SDLK_p:
                            paused = !paused;
                            break;
                        case SDLK_PERIOD:
                            delay*=2;
                            break;
                        case SDLK_COMMA:
                            delay/=2;
                            break;
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    unsigned int x_pos = event.button.x/SCALE;
                    unsigned int y_pos = event.button.y/SCALE;
                    unsigned int index = ROWLEN*y_pos + x_pos;
                    cells[index] = !cells[index];
                    draw(cells, buffer, texture, renderer);
                    break;
            }
        }
    }
    delete[] cells;
    delete[] new_cells;
}

void tick(bool *&cells, bool *&new_cells) {
    for(int i = 0; i < LENGTH/ROWLEN; i++) {
        for(int j = 0; j < ROWLEN; j++) {
            int index = i*ROWLEN + j;
            bool cell = cells[index];

            int sur_cells = 0;
            for(int x = -1; x < 2; x++) {
                for(int y = -1; y < 2; y++) {
                    int cell_index = index+(ROWLEN*x)+y;
                    if(!(x==0 && y==0) && cell_index<ROWLEN*(i+x+1) 
                        && cell_index>ROWLEN*(i+x)-1 && cell_index<LENGTH 
                        && cell_index>=0 && cells[cell_index])
                        sur_cells++;
                }
            }
            if(cell && !(sur_cells == 3 || sur_cells == 2))
                new_cells[index] = false;
            else if(!cell && sur_cells == 3)
                new_cells[index] = true;
            else if(cell)
                new_cells[index] = true;
            else 
                new_cells[index] = false;
        }
    }
    

    bool *temp = cells;
    cells = new_cells;
    new_cells = temp;
}

void update_buffer(bool *cells, uint32_t *buffer) {
    for(int i = 0; i < LENGTH/ROWLEN; i++) {
        for(int j = 0; j < ROWLEN; j++) {
            if(cells[i*ROWLEN + j])
                buffer[i*ROWLEN + j] = 0xFFFFFFFF;
            else
                buffer[i*ROWLEN + j] = 0x0000;
        }
    }
}

void draw(bool *cells, uint32_t *buffer, SDL_Texture *texture, SDL_Renderer *renderer) {
    update_buffer(cells, buffer);
    SDL_UpdateTexture(texture, nullptr, buffer, ROWLEN*4);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

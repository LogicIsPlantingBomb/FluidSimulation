#include<stdio.h>
#include<SDL2/SDL.h>
#include<string.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_BLACK 0x00000000
#define COLOR_BLUE 0x3498db
#define COLOR_LIGHT_BLUE 0x85c1e9
#define COLOR_DARK_BLUE 0x1f618d
#define COLOR_GRAY 0x7f7f7f
#define COLOR_DARK_GRAY 0x2c3e50
#define CELL_SIZE 10
#define LINE_WIDTH 1
#define COLUMNS (SCREEN_WIDTH/CELL_SIZE)
#define ROWS (SCREEN_HEIGHT/CELL_SIZE)

#define EMPTY_TYPE 0
#define WATER_TYPE 1
#define SOLID_TYPE 2

#define MAX_WATER 100
#define FLOW_RATE 25
#define GRAVITY_RATE 50

struct Cell {
    int type;
    int water_level;
    int new_water_level; // For double buffering
};

struct Cell grid[ROWS][COLUMNS];
struct Cell new_grid[ROWS][COLUMNS];

void init_grid() {
    for(int y = 0; y < ROWS; y++) {
        for(int x = 0; x < COLUMNS; x++) {
            grid[y][x].type = EMPTY_TYPE;
            grid[y][x].water_level = 0;
            grid[y][x].new_water_level = 0;
            
            new_grid[y][x].type = EMPTY_TYPE;
            new_grid[y][x].water_level = 0;
            new_grid[y][x].new_water_level = 0;
        }
    }
    
    // Add bottom boundary
    for(int x = 0; x < COLUMNS; x++) {
        grid[ROWS-1][x].type = SOLID_TYPE;
        new_grid[ROWS-1][x].type = SOLID_TYPE;
    }
    
    // Add side boundaries
    for(int y = 0; y < ROWS; y++) {
        grid[y][0].type = SOLID_TYPE;
        grid[y][COLUMNS-1].type = SOLID_TYPE;
        new_grid[y][0].type = SOLID_TYPE;
        new_grid[y][COLUMNS-1].type = SOLID_TYPE;
    }
}

void draw_grid(SDL_Surface* surface) {
    // Clear screen
    SDL_FillRect(surface, NULL, COLOR_BLACK);
    
    for(int y = 0; y < ROWS; y++) {
        for(int x = 0; x < COLUMNS; x++) {
            int pixel_x = x * CELL_SIZE;
            int pixel_y = y * CELL_SIZE;
            SDL_Rect cell_rect = {pixel_x, pixel_y, CELL_SIZE, CELL_SIZE};
            
            Uint32 color = COLOR_BLACK;
            
            if(grid[y][x].type == SOLID_TYPE) {
                color = COLOR_DARK_GRAY;
            } else if(grid[y][x].water_level > 0) {
                // Color based on water level
                if(grid[y][x].water_level >= MAX_WATER * 0.8) {
                    color = COLOR_DARK_BLUE;
                } else if(grid[y][x].water_level >= MAX_WATER * 0.4) {
                    color = COLOR_BLUE;
                } else {
                    color = COLOR_LIGHT_BLUE;
                }
            }
            
            if(color != COLOR_BLACK) {
                SDL_FillRect(surface, &cell_rect, color);
            }
        }
    }
    
    // Draw grid lines (optional, can be removed for better performance)
    /*
    for(int i = 0; i <= COLUMNS; i++) {
        SDL_Rect line = {i * CELL_SIZE, 0, LINE_WIDTH, SCREEN_HEIGHT};
        SDL_FillRect(surface, &line, COLOR_GRAY);
    }
    for(int i = 0; i <= ROWS; i++) {
        SDL_Rect line = {0, i * CELL_SIZE, SCREEN_WIDTH, LINE_WIDTH};
        SDL_FillRect(surface, &line, COLOR_GRAY);
    }
    */
}

void add_water(int x, int y, int amount) {
    if(x >= 0 && x < COLUMNS && y >= 0 && y < ROWS) {
        if(grid[y][x].type == EMPTY_TYPE) {
            grid[y][x].water_level += amount;
            if(grid[y][x].water_level > MAX_WATER) {
                grid[y][x].water_level = MAX_WATER;
            }
        }
    }
}

void add_solid(int x, int y) {
    if(x >= 0 && x < COLUMNS && y >= 0 && y < ROWS) {
        grid[y][x].type = SOLID_TYPE;
        grid[y][x].water_level = 0;
    }
}

void remove_cell(int x, int y) {
    if(x >= 0 && x < COLUMNS && y >= 0 && y < ROWS) {
        // Don't remove boundary walls
        if(x == 0 || x == COLUMNS-1 || y == ROWS-1) return;
        
        grid[y][x].type = EMPTY_TYPE;
        grid[y][x].water_level = 0;
    }
}

int min(int a, int b) {
    return a < b ? a : b;
}

int can_flow_down(int x, int y) {
    if(y >= ROWS - 1) return 0;
    return (grid[y + 1][x].type == EMPTY_TYPE && grid[y + 1][x].water_level < MAX_WATER);
}

int can_flow_horizontally(int from_x, int from_y, int to_x, int to_y) {
    if(to_x < 0 || to_x >= COLUMNS || to_y < 0 || to_y >= ROWS) return 0;
    if(grid[to_y][to_x].type != EMPTY_TYPE) return 0;
    
    // Water flows horizontally if there's a level difference
    return (grid[from_y][from_x].water_level > grid[to_y][to_x].water_level + 5);
}

void simulate_fluid() {
    // Copy current state to new grid
    memcpy(new_grid, grid, sizeof(grid));
    
    // Process each cell from bottom to top, left to right
    for(int y = ROWS - 2; y >= 0; y--) {
        for(int x = 1; x < COLUMNS - 1; x++) {
            if(grid[y][x].water_level <= 0) continue;
            
            // Priority 1: Apply gravity - water flows down first
            if(can_flow_down(x, y)) {
                struct Cell* current = &grid[y][x];
                struct Cell* below = &grid[y + 1][x];
                
                int available_space = MAX_WATER - below->water_level;
                int flow = min(min(current->water_level, GRAVITY_RATE), available_space);
                
                if(flow > 0) {
                    new_grid[y][x].water_level -= flow;
                    new_grid[y + 1][x].water_level += flow;
                    continue; // Don't flow horizontally if flowing down
                }
            }
            
            // Priority 2: Horizontal flow - water always tries to level out
            if(grid[y][x].water_level > 10) { // Flow if more than minimal amount
                int left_flow = 0, right_flow = 0;
                
                // Check left flow
                if(can_flow_horizontally(x, y, x - 1, y)) {
                    int pressure_diff = grid[y][x].water_level - grid[y][x - 1].water_level;
                    if(pressure_diff > 5) {
                        left_flow = min(pressure_diff / 3, FLOW_RATE);
                        left_flow = min(left_flow, MAX_WATER - grid[y][x - 1].water_level);
                    }
                }
                
                // Check right flow
                if(can_flow_horizontally(x, y, x + 1, y)) {
                    int pressure_diff = grid[y][x].water_level - grid[y][x + 1].water_level;
                    if(pressure_diff > 5) {
                        right_flow = min(pressure_diff / 3, FLOW_RATE);
                        right_flow = min(right_flow, MAX_WATER - grid[y][x + 1].water_level);
                    }
                }
                
                // Apply horizontal flows
                if(left_flow > 0) {
                    new_grid[y][x].water_level -= left_flow;
                    new_grid[y][x - 1].water_level += left_flow;
                }
                if(right_flow > 0) {
                    new_grid[y][x].water_level -= right_flow;
                    new_grid[y][x + 1].water_level += right_flow;
                }
            }
        }
    }
    
    // Copy new state back to main grid
    memcpy(grid, new_grid, sizeof(grid));
    
    // Clean up very small amounts of water
    for(int y = 0; y < ROWS; y++) {
        for(int x = 0; x < COLUMNS; x++) {
            if(grid[y][x].water_level < 3) {
                grid[y][x].water_level = 0;
            }
        }
    }
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Fluid Simulation", 
                                        SDL_WINDOWPOS_CENTERED, 
                                        SDL_WINDOWPOS_CENTERED, 
                                        SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    
    SDL_Surface* surface = SDL_GetWindowSurface(window);
    
    init_grid();
    
    int simulation_running = 1;
    int simulation_paused = 0;
    SDL_Event event;
    int current_type = WATER_TYPE;
    int mouse_held = 0;
    
    printf("Controls:\n");
    printf("Left Click + Drag: Add water/solid\n");
    printf("Right Click + Drag: Remove cells\n");
    printf("SPACE: Toggle between water and solid\n");
    printf("P: Pause/Resume simulation\n");
    printf("R: Reset simulation\n");
    printf("ESC: Quit\n\n");
    
    while(simulation_running) {
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                simulation_running = 0;
            }
            
            if(event.type == SDL_MOUSEBUTTONDOWN) {
                mouse_held = 1;
                int cell_x = event.button.x / CELL_SIZE;
                int cell_y = event.button.y / CELL_SIZE;
                
                if(event.button.button == SDL_BUTTON_LEFT) {
                    if(current_type == WATER_TYPE) {
                        add_water(cell_x, cell_y, MAX_WATER);
                    } else {
                        add_solid(cell_x, cell_y);
                    }
                } else if(event.button.button == SDL_BUTTON_RIGHT) {
                    remove_cell(cell_x, cell_y);
                }
            }
            
            if(event.type == SDL_MOUSEBUTTONUP) {
                mouse_held = 0;
            }
            
            if(event.type == SDL_MOUSEMOTION && mouse_held) {
                int cell_x = event.motion.x / CELL_SIZE;
                int cell_y = event.motion.y / CELL_SIZE;
                
                Uint32 mouse_state = SDL_GetMouseState(NULL, NULL);
                if(mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                    if(current_type == WATER_TYPE) {
                        add_water(cell_x, cell_y, MAX_WATER);
                    } else {
                        add_solid(cell_x, cell_y);
                    }
                } else if(mouse_state & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
                    remove_cell(cell_x, cell_y);
                }
            }
            
            if(event.type == SDL_KEYDOWN) {
                switch(event.key.keysym.sym) {
                    case SDLK_SPACE:
                        current_type = (current_type == WATER_TYPE) ? SOLID_TYPE : WATER_TYPE;
                        printf("Switched to: %s\n", (current_type == WATER_TYPE) ? "Water" : "Solid");
                        break;
                    case SDLK_p:
                        simulation_paused = !simulation_paused;
                        printf("Simulation %s\n", simulation_paused ? "paused" : "resumed");
                        break;
                    case SDLK_r:
                        init_grid();
                        printf("Simulation reset\n");
                        break;
                    case SDLK_ESCAPE:
                        simulation_running = 0;
                        break;
                }
            }
        }
        
        // Update simulation
        if(!simulation_paused) {
            simulate_fluid();
        }
        
        // Render
        draw_grid(surface);
        SDL_UpdateWindowSurface(window);
        SDL_Delay(50); // ~20 FPS
    }
    
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

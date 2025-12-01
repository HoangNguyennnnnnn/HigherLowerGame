/*
 * ============================================================================
 *                    HIGHER LOWER GAME - DATABASE MODULE
 * ============================================================================
 * File: database.c
 * Description: Game database loading và management
 * 
 * Chức năng:
 *   1. Load items từ file data/items.txt
 *   2. Random item selection utilities
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/game.h"

/* ============================================================================
 *                           GLOBAL VARIABLES
 * ============================================================================ */

// Database chứa tất cả items trong game
GameItem game_database[MAX_ITEMS];

// Số lượng items thực tế đã load
int item_count = 0;

/* ============================================================================
 *                           DATABASE INITIALIZATION
 * ============================================================================ */

/**
 * Load items từ file vào game_database
 * 
 * File format (data/items.txt):
 *   name|value|image_url
 *   # Comment lines start with #
 * 
 * Ví dụ:
 *   iPhone 15|1199|https://example.com/iphone.jpg
 */
void init_game_database() {
    FILE *file = fopen(ITEMS_FILE, "r");
    if (!file) {
        printf("⚠️  Warning: Could not open %s, using default items\n", ITEMS_FILE);
        
        // Fallback to default items
        strcpy(game_database[0].name, "iPhone 15 Pro");
        game_database[0].value = 1199;
        strcpy(game_database[0].image_url, "https://images.unsplash.com/photo-1695048133142-1a20484d2569?w=400");
        
        strcpy(game_database[1].name, "MacBook Pro");
        game_database[1].value = 2499;
        strcpy(game_database[1].image_url, "https://images.unsplash.com/photo-1517336714731-489689fd1ca8?w=400");
        
        strcpy(game_database[2].name, "PlayStation 5");
        game_database[2].value = 499;
        strcpy(game_database[2].image_url, "https://images.unsplash.com/photo-1606813907291-d86efa9b94db?w=400");
        
        strcpy(game_database[3].name, "Nike Air Jordan");
        game_database[3].value = 170;
        strcpy(game_database[3].image_url, "https://images.unsplash.com/photo-1542291026-7eec264c27ff?w=400");
        
        strcpy(game_database[4].name, "Tesla Model 3");
        game_database[4].value = 42990;
        strcpy(game_database[4].image_url, "https://images.unsplash.com/photo-1560958089-b8a1929cea89?w=400");
        
        item_count = 5;
        printf("📦 Game database initialized with %d default items\n", item_count);
        return;
    }
    
    char line[512];
    item_count = 0;
    
    while (fgets(line, sizeof(line), file) && item_count < MAX_ITEMS) {
        // Skip empty lines and comments
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }
        
        // Remove newline
        line[strcspn(line, "\n\r")] = 0;
        
        // Parse: name|value|image_url
        char *name = strtok(line, "|");
        char *value_str = strtok(NULL, "|");
        char *image_url = strtok(NULL, "|");
        
        if (name && value_str && image_url) {
            strncpy(game_database[item_count].name, name, ITEM_NAME_LEN - 1);
            game_database[item_count].name[ITEM_NAME_LEN - 1] = '\0';
            
            game_database[item_count].value = atoi(value_str);
            
            strncpy(game_database[item_count].image_url, image_url, IMAGE_URL_LEN - 1);
            game_database[item_count].image_url[IMAGE_URL_LEN - 1] = '\0';
            
            item_count++;
        }
    }
    
    fclose(file);
    
    if (item_count < 2) {
        printf("⚠️  Warning: Not enough items loaded, adding defaults\n");
        strcpy(game_database[0].name, "Item A");
        game_database[0].value = 100;
        strcpy(game_database[0].image_url, "https://via.placeholder.com/400");
        
        strcpy(game_database[1].name, "Item B");
        game_database[1].value = 200;
        strcpy(game_database[1].image_url, "https://via.placeholder.com/400");
        
        item_count = 2;
    }
    
    printf("📦 Game database loaded %d items from %s\n", item_count, ITEMS_FILE);
    for (int i = 0; i < item_count && i < 5; i++) {
        printf("   • %s: $%d\n", game_database[i].name, game_database[i].value);
    }
    if (item_count > 5) {
        printf("   ... and %d more items\n", item_count - 5);
    }
}

/* ============================================================================
 *                           UTILITY FUNCTIONS
 * ============================================================================ */

/**
 * Lấy random index khác với index cho trước
 * 
 * @param except Index cần tránh
 * @return Random index trong khoảng [0, item_count)
 */
int get_random_index_except(int except) {
    if (item_count <= 1) return 0;
    int index;
    do {
        index = rand() % item_count;
    } while (index == except);
    return index;
}

#include "win_main.h"

static Window* window;

static MenuLayer* s_menu_layer;

/* This variable will be set when we get the JSON from SpaceAPI, it keep
 * track of the number of items in the second section.
 */
static uint16_t space_info_current_number = 0;

#define MAX_NUMBER_OF_MENU 3
static char* space_info_title[MAX_NUMBER_OF_MENU];
static char* space_info_subtitle[MAX_NUMBER_OF_MENU];
static void (*space_info_callback[MAX_NUMBER_OF_MENU]) () = {};

/* ------------------------------------------------------------------------
 *                      MENU HANDLING
 * ------------------------------------------------------------------------
 */

/* @desc Return number of sections.
 *
 * @param {menu_layer} :
 * @param {data} :
 *
 * @return {uint16_t} : Number of sections.
 */
/* - Header.
 * - Navigation.
 */
#define NUMBER_OF_SECTIONS 3
static uint16_t menu_get_num_sections_callback(MenuLayer* menu_layer, void* data) {
    return NUMBER_OF_SECTIONS;
}

/* @desc Get number of "items" by "section".
 *
 * @param {menu_layer} :
 * @param {section_index} :
 * @param {data} :
 */
/* Only 1 image is shown. */
#define NUMBER_OF_ITEM_IN_HEADER 1
#define NUMBER_OF_ITEM_IN_OTHER 1
static uint16_t menu_get_num_rows_callback(MenuLayer* menu_layer, uint16_t section_index, void* data) {
    switch (section_index) {
        case 0:
            return NUMBER_OF_ITEM_IN_HEADER;
        case 1:
            return space_info_current_number;
        case 2:
            return NUMBER_OF_ITEM_IN_OTHER;
        default:
            return 0;
    }
}

/* @desc Get header size.
 *
 * @param {menu_layer} :
 * @param {section_index} :
 * @param {data} :
 */
static int16_t menu_get_header_height_callback(MenuLayer* menu_layer, uint16_t section_index, void* data) {
  return PBL_IF_RECT_ELSE(MENU_CELL_BASIC_HEADER_HEIGHT, 0);
}

/* @desc Get cell size.
 */
static int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    return MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT;
}

/* @desc Draw the headers sections.
 *
 * @param {ctx} :
 * @param {cell_layer} :
 * @param {section_index} :
 * @param {data} :
 */
static void menu_draw_header_callback(GContext* ctx, const Layer* cell_layer, uint16_t section_index, void* data) {
#ifndef PBL_ROUND
    switch (section_index) {
        case 0: {
          static char space_state[32];
          if (basic_info) {
            if (basic_info->state == Open) {
              snprintf(space_state, 32, "%s is open", space_name_buffer);
            } else if (basic_info->state == Closed){
              snprintf(space_state, 32, "%s is closed", space_name_buffer);
            } else if (basic_info->state == Undefined) {
              snprintf(space_state, 32, "%s", space_name_buffer);
            }
          } else {
            snprintf(space_state, 32, "%s", space_name_buffer);
          }

          menu_cell_basic_header_draw(ctx, cell_layer, space_state);
          break;
        }
        case 1: {
          menu_cell_basic_header_draw(ctx, cell_layer, "SpaceAPI content");
          break;
        }
        case 2: {
          menu_cell_basic_header_draw(ctx, cell_layer, "Other");
          break;
        }
    }
#endif
}

/* @desc Draw items in the section.
 */
static void menu_draw_row_callback(GContext* ctx, const Layer* cell_layer, MenuIndex* cell_index, void* data) {
    switch (cell_index->section) {
        case 0: ;
          if (basic_info) {
            if (basic_info->state == Open) {
              menu_cell_basic_draw(ctx, cell_layer, space_name_buffer, "is open", NULL);
            } else if (basic_info->state == Closed) {
              menu_cell_basic_draw(ctx, cell_layer, space_name_buffer, "is closed", NULL);
            } else {
              menu_cell_basic_draw(ctx, cell_layer, space_name_buffer, NULL, NULL);
            }
          } else {
            // If basic_info is not initialized there is still no data received from the API.
            menu_cell_basic_draw(ctx, cell_layer, space_name_buffer, "Loading ...", NULL);
          }
          break;
        case 1:
          menu_cell_basic_draw(ctx, cell_layer, space_info_title[cell_index->row], space_info_subtitle[cell_index->row], NULL);
          break;
        case 2:
          menu_cell_basic_draw(ctx, cell_layer, "About", "About the app.", NULL);
          break;
    }
}

/* @desc Assign functions callback to items.
 */
static void menu_select_callback(MenuLayer* menu_layer, MenuIndex* cell_index, void* data) {
    switch (cell_index->section) {
        case 0:
          win_basic_show();
          break;
        case 1:
          if (space_info_callback[cell_index->row] != NULL) {
              (*space_info_callback[cell_index->row]) ();
          }
          break;
        case 2:
          win_about_show();
          break;
        default:
          break;
    }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
    .get_cell_height = menu_get_cell_height_callback,
  });

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(s_menu_layer, window);

  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
}

void win_main_update(void) {
  // Section 1 : SpaceAPI content.

  space_info_current_number = 0;

  /* Drawing the second section with info about person present
   * in the hackerspace.
   */
  if (sensors_array && sensors_array->current) {
    static char sensor_menu_buffer[BUFFER_SIZE];
    /* static char sensor_menu_subtitle_buffer[BUFFER_SIZE]; */
    snprintf(sensor_menu_buffer, BUFFER_SIZE, "Sensors menu.");
    /* snprintf(number_of_people_subtitle_buffer, BUFFER_SIZE, "%ld persons", current->value); */
    DEBUG("Added sensors menu position : %i", space_info_current_number);
    space_info_title[space_info_current_number] = sensor_menu_buffer;
    space_info_subtitle[space_info_current_number] = NULL;
    space_info_callback[space_info_current_number] = lambda(void, () {
        win_objects_menu_show (sensors_array,
            sensors_array->current > 1 ? "Sensors" : "Sensor"
        );
    });
    ++space_info_current_number;
  }

  if (contacts_section && contacts_section->current) {
    /* Creating the "Contact" menu button
     * It will acces to another window.
     */
    static char contact_window_buffer[BUFFER_SIZE];
    static char contact_window_subtitle_buffer[BUFFER_SIZE];
    snprintf(contact_window_buffer, BUFFER_SIZE, contacts_section->current > 1 ?  "Contacts" : "Contact");
    snprintf(contact_window_subtitle_buffer, BUFFER_SIZE, "Contact info about %s", space_name_buffer);
    DEBUG("Contact section added : %i", space_info_current_number);
    space_info_title[space_info_current_number] = contact_window_buffer;
    space_info_subtitle[space_info_current_number] = contact_window_subtitle_buffer;
    space_info_callback[space_info_current_number] = lambda(void, () {
        win_objects_menu_show (contacts_section,
            sensors_array->current > 1 ? "Contacts" : "Contact"
        );
    });

    ++space_info_current_number;
  }

  if (key_masters && key_masters->current) {
    static char keymaster_window_buffer[BUFFER_SIZE];
    snprintf(keymaster_window_buffer, BUFFER_SIZE,
        key_masters->length > 1 ? "Keymasters list" : "Keymaster");
    DEBUG("Keymaster section added : %i", space_info_current_number);
    space_info_title[space_info_current_number] = keymaster_window_buffer;
    space_info_subtitle[space_info_current_number] = NULL;
    space_info_callback[space_info_current_number] = lambda(void, () {
        win_objects_menu_show (key_masters,
            key_masters->current > 1 ? "Keymasters" : "Keymaster"
        );
    });

    ++space_info_current_number;
  }

  menu_layer_reload_data(s_menu_layer);
  layer_mark_dirty(menu_layer_get_layer(s_menu_layer));
}

void win_main_init(void) {
  strcpy(space_name_buffer, "Hackerspace");

  // Section 0 : SpaceAPI basic info
  win_basic_init();
  // Section 1 : SpaceAPI info
  win_object_init();
  win_objects_menu_init();
  // Section 2 : About.
  win_about_init();

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);
}

void win_main_deinit(void) {
  win_basic_deinit();
  win_object_deinit();
  win_objects_menu_deinit();;
  win_about_deinit();

  // Globals value.
  key_masters->free(key_masters, NULL);
  sensors_array->free(sensors_array, NULL);
  contacts_section->free(contacts_section, NULL);

  free_icons();

  window_destroy(window);
}

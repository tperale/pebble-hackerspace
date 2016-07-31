#include "./win_sensor_menu.h"

SecureArray* sensors_array = NULL;

static Window* s_window = NULL;
static SimpleMenuLayer* s_menu_layer = NULL;

static uint32_t number_of_sensors = 0;
static SimpleMenuItem* s_sensors_menu = NULL;
static SimpleMenuSection s_sensors_menu_section[1];

static char** s_title_array = NULL;
static char** s_subtitle_array = NULL;

static void draw_sensor (int index, void* context) {
  win_sensor_show((void*) sensors_array->array[index]);
}

static void window_appear (Window* window) {
  if (s_sensors_menu && s_title_array && s_subtitle_array) {
    return;
  }

  number_of_sensors = sensors_array->current;

  s_sensors_menu = malloc(sizeof(SimpleMenuItem) * number_of_sensors);
  s_title_array = malloc(sizeof(char*) * number_of_sensors);
  s_subtitle_array = malloc(sizeof(char*) * number_of_sensors);

  uint32_t count = 0;
  for (uint32_t i = 0; i < number_of_sensors; ++i) {
    Sensor* tmp = (Sensor*) sensors_array->array[i];

    s_sensors_menu[i] = tmp->menu(sensors_array->array[i]);
    s_sensors_menu[i].callback = draw_sensor;

    ++count;
  }

  s_sensors_menu_section[0] = (SimpleMenuSection) {
    .title = PBL_IF_RECT_ELSE(
        number_of_sensors > 1 ? "Sensors" : "Sensor",
        NULL),
    .num_items = count,
    .items = s_sensors_menu,
  };

  Layer* window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  s_menu_layer = simple_menu_layer_create(bounds, window, s_sensors_menu_section, 1, NULL);

  layer_add_child(window_layer, simple_menu_layer_get_layer(s_menu_layer));
}

static void window_disappear (Window* window) {
  if (!window_stack_contains_window(window)) {
    simple_menu_layer_destroy(s_menu_layer);
    free(s_sensors_menu);
    s_sensors_menu = NULL;
    free(s_title_array);
    s_title_array = NULL;
    free(s_subtitle_array);
    s_subtitle_array = NULL;
  }
}

void win_sensor_menu_show(void) {
  window_stack_push(s_window, true);
}

void win_sensor_menu_init(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .appear = window_appear,
    .disappear = window_disappear,
  });
  win_sensor_init();
}

void win_sensor_menu_deinit(void) {
  win_sensor_deinit();

  if (sensors_array) {
    sensors_array->free(sensors_array, lambda(void, (void* s) {
      Sensor* self = (Sensor*) s;
      self->free(self);
    }));
  }

  window_destroy(s_window);
}

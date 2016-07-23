#include "./radiation.h"

Radiation* Radiation_new (uint32_t type,
    uint32_t value,
    char* unit,
    char* location,
    char* name,
    char* description,
    uint32_t dead_time,
    uint32_t conversion_factor)
{
  Radiation* t = malloc(sizeof(Radiation));

  *t = (Radiation) {
    .type = type,
    .value = value,
    .unit = NULL,
    .location = NULL,
    .name = NULL,
    .description = NULL,
    .dead_time = dead_time,
    .conversion_factor = conversion_factor,
    .Radiation_free = Radiation_free,
    .Radiation_menu = Radiation_menu,
    .Radiation_draw = Radiation_draw,
    .Radiation_destroy = Radiation_destroy,
  };

  if (unit) {
    size_t length = strlen(unit);
    t->unit = malloc(sizeof(char) * length);
    memcpy(t->unit, unit, length);

    snprintf(t->formated_value, 32, "%ld %s", t->value, t->unit);
  } else {
    snprintf(t->formated_value, 32, "%ld", t->value);
  }

  if (location) {
    size_t length = strlen(location);
    t->location = malloc(sizeof(char) * length);
    memcpy(t->location, location, length);
  }

  if (name) {
    size_t length = strlen(name);
    t->name = malloc(sizeof(char) * length);
    memcpy(t->name, name, length);
  }

  if (description) {
    size_t length = strlen(description);
    t->description = malloc(sizeof(char) * length);
    memcpy(t->description, description, length);
  }

  return t;
}

void Radiation_free(void* s) {
  Radiation* self = (Radiation*) s;
  if (self->unit) {
    free(self->unit);
  }

  if (self->location) {
    free(self->location);
  }

  if (self->name) {
    free(self->name);
  }

  if (self->description) {
    free(self->description);
  }

  free(self);
}

SimpleMenuItem Radiation_menu (void* s) {
  Radiation* t = (Radiation*) s;
  SimpleMenuItem result = {};

  /* result.icon = humidity_icon; */

  if (t->name) {
    result.title = t->name;
  } else if (t->location) {
    result.title = t->location;
  } else {
    result.title = "Radiation";
  }

  result.subtitle = t->formated_value;

  return result;
}

void Radiation_draw (Window* window, void* s) { }

void Radiation_destroy (void* s) { }
#include <pebble.h>

#include "./windows/win_main.h"
#include "./windows/win_sensor_menu.h"
#include "./globals.h"
#include "./appinfo.h"

/* ------------------------------------------------------------------------
 *                      Partie communication avec l'API.
 * ------------------------------------------------------------------------
 */

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void inbox_connected_person_callback(DictionaryIterator *iterator, void *context) {
  switch ((uint32_t) dict_find(iterator, KEY_TYPE)->value->uint32) {
      case KEY_BASIC: {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "KEY_BASIC");
        snprintf(space_name_buffer, BUFFER_SIZE, "%s", dict_find(iterator, KEY_SPACE)->value->cstring);
        Tuple* tmp = dict_find(iterator, KEY_OPEN_STATE);
        if (tmp) {
          open_state = tmp->value->uint32;
        }
        break;
      }
      case KEY_CONTACT: {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "KEY_CONTACT");

        if (contacts_section) {
          Contacts_free(contacts_section);
        }

        Tuple* tmp = dict_find(iterator, KEY_LENGTH);
        if (tmp == NULL) {
          APP_LOG(APP_LOG_LEVEL_DEBUG, "Poorly formated contact section response.");
          return;
        }

        contacts_section = Contacts_new(tmp->value->uint32);

        tmp = dict_find(iterator, KEY_CONTACT_PHONE_NUMBER);
        if (tmp) {
          Contacts_add(contacts_section, "Phone number", tmp->value->cstring);
        }

        tmp = dict_find(iterator, KEY_CONTACT_SIP_ADDRESS);
        if (tmp) {
          Contacts_add(contacts_section, "SIP address", tmp->value->cstring);
        }

        tmp = dict_find(iterator, KEY_CONTACT_IRC);
        if (tmp) {
          Contacts_add(contacts_section, "Irc", tmp->value->cstring);
        }

        tmp = dict_find(iterator, KEY_CONTACT_TWITTER);
        if (tmp) {
          Contacts_add(contacts_section, "Twitter", tmp->value->cstring);
        }

        tmp = dict_find(iterator, KEY_CONTACT_FACEBOOK);
        if (tmp) {
          Contacts_add(contacts_section, "Facebook", tmp->value->cstring);
        }

        tmp = dict_find(iterator, KEY_CONTACT_IDENTICA);
        if (tmp) {
          Contacts_add(contacts_section, "Identica", tmp->value->cstring);
        }

        tmp = dict_find(iterator, KEY_CONTACT_FOURSQUARE);
        if (tmp) {
          Contacts_add(contacts_section, "Foursquare", tmp->value->cstring);
        }

        tmp = dict_find(iterator, KEY_CONTACT_EMAIL);
        if (tmp) {
          Contacts_add(contacts_section, "Email", tmp->value->cstring);
        }

        tmp = dict_find(iterator, KEY_CONTACT_MAILLING_LIST);
        if (tmp) {
          Contacts_add(contacts_section, "Mailling list", tmp->value->cstring);
        }

        tmp = dict_find(iterator, KEY_CONTACT_JABBER);
        if (tmp) {
          Contacts_add(contacts_section, "Jabber", tmp->value->cstring);
        }

        tmp = dict_find(iterator, KEY_CONTACT_ISSUE_MAIL);
        if (tmp) {
          Contacts_add(contacts_section, "Issue mail", tmp->value->cstring);
        }

        win_main_update();
        break;
      }
      case KEY_SENSOR_PEOPLE_NOW_PRESENT:;
        switch ((uint32_t) dict_find(iterator, KEY_SUBTYPE)->value->uint32) {
            case KEY_NAMES: {
                APP_LOG(APP_LOG_LEVEL_DEBUG, "KEY_SENSOR_PEOPLE_NOW_PRESENT NAME received.");
                uint32_t index = (uint32_t) dict_find(iterator, KEY_INDEX)->value->uint32;
                uint32_t subindex = (uint32_t) dict_find(iterator, KEY_SUBINDEX)->value->uint32;
                PeopleNowPresent_add_person(
                        sensors_array->array[index],
                        subindex,
                        (char*) dict_find(iterator, KEY_NAMES)->value->cstring
                );
                break;
            }
            default: {
                APP_LOG(APP_LOG_LEVEL_DEBUG, "KEY_SENSOR_PEOPLE_NOW_PRESENT basic");
                if (sensors_array == NULL) {
                    uint32_t length = (uint32_t) dict_find(iterator, KEY_LENGTH)->value->uint32;
                    sensors_array= SensorsArray_new(length);
                }

                uint32_t index = (uint32_t) dict_find(iterator, KEY_INDEX)->value->uint32;
                uint32_t value = (uint32_t) dict_find(iterator, KEY_VALUE)->value->uint32;
                sensors_array->array[index] = PeopleNowPresent_new (value);
                break;
            }
        }
        win_main_update();
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Default message");

  }
}

int main(void) {
  win_main_init();

  // Register callbacks
  app_message_register_inbox_received(inbox_connected_person_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  /* app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum()); */
  app_message_open(1024, 1024);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed main window");

  app_event_loop();

  win_main_deinit();
}

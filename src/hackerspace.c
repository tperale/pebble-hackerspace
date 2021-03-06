#include <pebble.h>

#include "./windows/win_main.h"
#include "./libs/log.h"
#include "./libs/basic/basic.h"
#include "./libs/sensors/people_now_present.h"
#include "./libs/sensors/temperature.h"
#include "./libs/sensors/door_locked.h"
#include "./libs/sensors/barometer.h"
#include "./libs/sensors/humidity.h"
#include "./libs/sensors/power_consumption.h"
#include "./libs/sensors/account_balance.h"
#include "./libs/sensors/beverage_supply.h"
#include "./libs/sensors/total_member_count.h"
#include "./libs/sensors/radiation.h"
#include "./libs/keymaster/keymaster.h"
#include "./globals.h"
#include "./appinfo.h"

static char* GET_CSTRING (DictionaryIterator* it, int key) {
  Tuple* tmp = dict_find(it, key);
  if (tmp) {
    return tmp->value->cstring;
  } else {
    return NULL;
  }
}

static uint32_t GET_UINT32 (DictionaryIterator* it, int key) {
  Tuple* tmp = dict_find(it, key);
  if (tmp) {
    return tmp->value->uint32;
  } else {
    return 0;
  }
}


/* ------------------------------------------------------------------------
 *                      Partie communication avec l'API.
 * ------------------------------------------------------------------------
 */
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  ERROR("Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  INFO("Outbox send success!");
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  LOG("Message dropped!");
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  switch ((uint32_t) dict_find(iterator, KEY_TYPE)->value->uint32) {
      case KEY_BASIC: {
        LOG("KEY_BASIC");

        if (basic_info) {
          basic_info->free(basic_info);
        }

        char* space = GET_CSTRING(iterator, KEY_SPACE);
        if (space) {
          snprintf(space_name_buffer, BUFFER_SIZE, "%s", space);
        }

        basic_info = BasicInfo_new(space,
            GET_CSTRING(iterator, KEY_SPACE_URL),
            GET_UINT32(iterator, KEY_LAST_CHANGE),
            GET_UINT32(iterator, KEY_OPEN_STATE)
        );

        break;
      }
      case KEY_CONTACT: {
        LOG("KEY_CONTACT");

        if (contacts_section) {
          contacts_section->free(contacts_section, NULL);
        }

        Tuple* tmp = dict_find(iterator, KEY_LENGTH);
        if (tmp == NULL) {
          ERROR("Poorly formated contact section response.");
          return;
        }

        contacts_section = SecureArray_new(tmp->value->uint32);

        tmp = dict_find(iterator, KEY_CONTACT_PHONE_NUMBER);
        if (tmp) {
          contacts_section->add(contacts_section,
              ContactItem_new("Phone number", tmp->value->cstring)
          );
        }

        tmp = dict_find(iterator, KEY_CONTACT_SIP_ADDRESS);
        if (tmp) {
          contacts_section->add(contacts_section,
              ContactItem_new("SIP address", tmp->value->cstring)
          );
        }

        tmp = dict_find(iterator, KEY_CONTACT_IRC);
        if (tmp) {
          contacts_section->add(contacts_section,
              ContactItem_new("IRC", tmp->value->cstring)
          );
        }

        tmp = dict_find(iterator, KEY_CONTACT_TWITTER);
        if (tmp) {
          contacts_section->add(contacts_section,
              ContactItem_new("Twitter", tmp->value->cstring)
          );
        }

        tmp = dict_find(iterator, KEY_CONTACT_FACEBOOK);
        if (tmp) {
          contacts_section->add(contacts_section,
              ContactItem_new("Facebook", tmp->value->cstring)
          );
        }

        tmp = dict_find(iterator, KEY_CONTACT_IDENTICA);
        if (tmp) {
          contacts_section->add(contacts_section,
              ContactItem_new("Identica", tmp->value->cstring)
          );
        }

        tmp = dict_find(iterator, KEY_CONTACT_FOURSQUARE);
        if (tmp) {
          contacts_section->add(contacts_section,
              ContactItem_new("Foursquare", tmp->value->cstring)
          );
        }

        tmp = dict_find(iterator, KEY_CONTACT_EMAIL);
        if (tmp) {
          contacts_section->add(contacts_section,
              ContactItem_new("Email", tmp->value->cstring)
          );
        }

        tmp = dict_find(iterator, KEY_CONTACT_MAILLING_LIST);
        if (tmp) {
          contacts_section->add(contacts_section,
              ContactItem_new("Mailling list", tmp->value->cstring)
          );
        }

        tmp = dict_find(iterator, KEY_CONTACT_JABBER);
        if (tmp) {
          contacts_section->add(contacts_section,
              ContactItem_new("Jabber", tmp->value->cstring)
          );
        }

        tmp = dict_find(iterator, KEY_CONTACT_ISSUE_MAIL);
        if (tmp) {
          contacts_section->add(contacts_section,
              ContactItem_new("Issue mail", tmp->value->cstring)
          );
        }

        break;
      }
      case KEY_CONTACT_KEYMASTER: {
        if (!key_masters) {
          uint32_t length = (uint32_t) dict_find(iterator, KEY_LENGTH)->value->uint32;
          key_masters = SecureArray_new(length);
        }

        key_masters->add(key_masters, KeyMaster_new(
          GET_CSTRING(iterator, KEY_NAME),
          GET_CSTRING(iterator, KEY_NICK),
          GET_CSTRING(iterator, KEY_PHONE),
          GET_CSTRING(iterator, KEY_EMAIL),
          GET_CSTRING(iterator, KEY_TWITTER)
        ));
        break;
      }
      case KEY_SENSOR: {
        LOG("KEY_SENSOR, creating new array.");
        if (sensors_array) {
          ERROR("KEY_SENSOR : An array already exist.");
          /* sensors_array->free(sensors_array); */
          return; // TODO Change when update will be allowed.
        }

        uint32_t length = (uint32_t) dict_find(iterator, KEY_LENGTH)->value->uint32;
        sensors_array= SecureArray_new(length);

        break;
      }
      case KEY_SENSOR_TEMPERATURE: {
        LOG("KEY_SENSOR_TEMPERATURE");
        /* uint32_t index = (uint32_t) dict_find(iterator, KEY_INDEX)->value->uint32; */

        sensors_array->add(sensors_array, Temperature_new (GET_UINT32(iterator, KEY_VALUE),
            GET_CSTRING(iterator, KEY_UNIT),
            GET_CSTRING(iterator, KEY_LOCATION),
            GET_CSTRING(iterator, KEY_NAME),
            GET_CSTRING(iterator, KEY_DESCRIPTION)
        ));
        break;
      }
      case KEY_SENSOR_DOOR_LOCKED: {
        LOG("KEY_SENSOR_DOOR_LOCKED");
        /* uint32_t index = (uint32_t) dict_find(iterator, KEY_INDEX)->value->uint32; */

        sensors_array->add(sensors_array, DoorLocked_new (GET_UINT32(iterator, KEY_VALUE),
            GET_CSTRING(iterator, KEY_LOCATION),
            GET_CSTRING(iterator, KEY_NAME),
            GET_CSTRING(iterator, KEY_DESCRIPTION)
        ));
        break;
      }
      case KEY_SENSOR_TOTAL_MEMBER_COUNT: {
        LOG("KEY_SENSOR_TOTAL_MEMBER_COUNT");
        /* uint32_t index = (uint32_t) dict_find(iterator, KEY_INDEX)->value->uint32; */

        sensors_array->add(sensors_array, TotalMemberCount_new (GET_UINT32(iterator, KEY_VALUE),
            GET_CSTRING(iterator, KEY_LOCATION),
            GET_CSTRING(iterator, KEY_NAME),
            GET_CSTRING(iterator, KEY_DESCRIPTION)
        ));
        break;
      }
      case KEY_SENSOR_BAROMETER: {
        LOG("KEY_SENSOR_BAROMETER");
        /* uint32_t index = (uint32_t) dict_find(iterator, KEY_INDEX)->value->uint32; */

        sensors_array->add(sensors_array, Barometer_new (GET_UINT32(iterator, KEY_VALUE),
            GET_CSTRING(iterator, KEY_UNIT),
            GET_CSTRING(iterator, KEY_LOCATION),
            GET_CSTRING(iterator, KEY_NAME),
            GET_CSTRING(iterator, KEY_DESCRIPTION)
        ));
        break;
      }
      case KEY_SENSOR_HUMIDITY: {
        LOG("KEY_SENSOR_HUMIDITY");
        /* uint32_t index = (uint32_t) dict_find(iterator, KEY_INDEX)->value->uint32; */

        sensors_array->add(sensors_array, Humidity_new (GET_UINT32(iterator, KEY_VALUE),
            GET_CSTRING(iterator, KEY_UNIT),
            GET_CSTRING(iterator, KEY_LOCATION),
            GET_CSTRING(iterator, KEY_NAME),
            GET_CSTRING(iterator, KEY_DESCRIPTION)
        ));
        break;
      }
      case KEY_SENSOR_POWER_CONSUMPTION: {
        LOG("KEY_SENSOR_POWER_CONSUMPTION");
        /* uint32_t index = (uint32_t) dict_find(iterator, KEY_INDEX)->value->uint32; */

        sensors_array->add(sensors_array, PowerConsumption_new (GET_UINT32(iterator, KEY_VALUE),
            GET_CSTRING(iterator, KEY_UNIT),
            GET_CSTRING(iterator, KEY_LOCATION),
            GET_CSTRING(iterator, KEY_NAME),
            GET_CSTRING(iterator, KEY_DESCRIPTION)
        ));
        break;
      }
      case KEY_SENSOR_ACCOUNT_BALANCE: {
        LOG("KEY_SENSOR_ACCOUNT_BALANCE");
        /* uint32_t index = (uint32_t) dict_find(iterator, KEY_INDEX)->value->uint32; */

        sensors_array->add(sensors_array, AccountBalance_new (GET_UINT32(iterator, KEY_VALUE),
            GET_CSTRING(iterator, KEY_UNIT),
            GET_CSTRING(iterator, KEY_LOCATION),
            GET_CSTRING(iterator, KEY_NAME),
            GET_CSTRING(iterator, KEY_DESCRIPTION)
        ));
        break;
      }
      case KEY_SENSOR_BEVERAGE_SUPPLY: {
        LOG("KEY_SENSOR_BEVERAGE_SUPPLY");
        /* uint32_t index = (uint32_t) dict_find(iterator, KEY_INDEX)->value->uint32; */

        sensors_array->add(sensors_array, BeverageSupply_new (GET_UINT32(iterator, KEY_VALUE),
            GET_CSTRING(iterator, KEY_UNIT),
            GET_CSTRING(iterator, KEY_LOCATION),
            GET_CSTRING(iterator, KEY_NAME),
            GET_CSTRING(iterator, KEY_DESCRIPTION)
        ));
        break;
      }
      case KEY_SENSOR_RADIATION: {
        uint32_t type = ((uint32_t) dict_find(iterator, KEY_SUBTYPE)->value->uint32);
        sensors_array->add(sensors_array, Radiation_new (type,
            GET_UINT32(iterator, KEY_VALUE),
            GET_CSTRING(iterator, KEY_UNIT),
            GET_CSTRING(iterator, KEY_LOCATION),
            GET_CSTRING(iterator, KEY_NAME),
            GET_CSTRING(iterator, KEY_DESCRIPTION),
            GET_UINT32(iterator, KEY_DEAD_TIME),
            GET_UINT32(iterator, KEY_CONVERSION_FACTOR)
        ));
        break;
      }
      case KEY_SENSOR_PEOPLE_NOW_PRESENT: {
        switch ((uint32_t) dict_find(iterator, KEY_SUBTYPE)->value->uint32) {
            case KEY_NAMES: {
                LOG("KEY_SENSOR_PEOPLE_NOW_PRESENT NAME received.");
                uint32_t index = (uint32_t) dict_find(iterator, KEY_INDEX)->value->uint32;
                uint32_t subindex = (uint32_t) dict_find(iterator, KEY_SUBINDEX)->value->uint32;

                PeopleNowPresent* sensor = sensors_array->get(sensors_array, lambda(int, (void* s, int i) {
                      PeopleNowPresent* self = (PeopleNowPresent*) s;
                      if (self->type == people_now_present && self->index == index) {
                        return 1;
                      }
                      return 0;
                }));

                PeopleNowPresent_add_person(
                    sensor,
                    subindex,
                    (char*) dict_find(iterator, KEY_NAMES)->value->cstring
                );
                break;
            }
            default: {
                LOG("KEY_SENSOR_PEOPLE_NOW_PRESENT basic");
                /* uint32_t index = (uint32_t) dict_find(iterator, KEY_INDEX)->value->uint32; */

                uint32_t value = (uint32_t) dict_find(iterator, KEY_VALUE)->value->uint32;
                sensors_array->add(sensors_array, PeopleNowPresent_new (value,
                      GET_CSTRING(iterator, KEY_LOCATION),
                      GET_CSTRING(iterator, KEY_NAME),
                      GET_CSTRING(iterator, KEY_DESCRIPTION),
                      GET_UINT32(iterator, KEY_INDEX)
                ));
                break;
            }
        }
        break;
      }
      default:
        WARN("Default message");
  }
  win_main_update();
}

int main(void) {
  win_main_init();

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  /* app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum()); */
  app_message_open(1024, 1024);

  DEBUG("Done initializing, pushed main window");

  app_event_loop();

  win_main_deinit();
}

#include <pebble.h>
#pragma once

///////////////////////
// weather variables //
///////////////////////
#define KEY_TEMP_F
#define KEY_TEMP_C
#define KEY_ICON

////////////////////
// font variables //
////////////////////
#define TEXT_FONT RESOURCE_ID_BORDA_BOLD_FONT_11

#define SETTINGS_KEY 1

///////////////////
// Clay settings //
///////////////////
typedef struct ClaySettings {
	GColor BackgroundColor;
  GColor ForegroundColor;
  GColor HandColor;
  GColor HandBackColor;
  bool InvertColors;
  bool InvertHandColors;
} ClaySettings; // Define our settings struct

static void config_default();
static void config_load();
static void setColors();
static void config_save();
static void dial_update_proc(Layer *layer, GContext *ctx);
static void ticks_update_proc(Layer *layer, GContext *ctx);
static void main_window_load(Window *window);
static void update_time();
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
static void main_window_unload(Window *window);
static void load_icons();
static void inbox_received_callback(DictionaryIterator *iterator, void *context);
static void inbox_dropped_callback(AppMessageResult reason, void *context);
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context);
static void outbox_sent_callback(DictionaryIterator *iterator, void *context);
static void init();
static void deinit();
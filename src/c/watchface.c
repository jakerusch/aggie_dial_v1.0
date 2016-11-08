#include <pebble.h>

///////////////////////
// weather variables //
///////////////////////
#define KEY_TEMP
#define KEY_ICON

////////////////////
// font variables //
////////////////////
#define TEXT_FONT RESOURCE_ID_BORDA_BOLD_FONT_11

/////////////////////
// color variables //
/////////////////////
#define BACKGROUND_COLOR GColorBlack
#define FOREGROUND_COLOR GColorWhite
#define HAND_BACKGROUND_COLOR GColorBlack
#define HAND_FOREGROUND_COLOR GColorWhite

///////////////////
// logo variable //
///////////////////
#define LOGO RESOURCE_ID_LOGO_WHITE

static Window *s_main_window;
static Layer *s_hands_layer, *s_dial_layer;
static GBitmap *s_bitmap, *s_weather_bitmap;
static BitmapLayer *s_bitmap_layer, *s_weather_bitmap_layer;
static GPath *s_minute_arrow, *s_hour_arrow;
static GFont s_word_font;
static TextLayer *s_date_text_layer, *s_temp_layer;
static int buf=24;
static char icon_buf[64];

// /////////////////
// // minute hand //
// /////////////////
// static const GPathInfo MINUTE_HAND_POINTS = {
//     6, (GPoint []) {
//         { -4, 10 },
//         { 4, 10 },
//         { 4, -68 },
//         { 2, -70 },
//         {-2, -70},
//         {-4, -68}
//     }
// };

// ///////////////
// // hour hand //
// ///////////////
// static const GPathInfo HOUR_HAND_POINTS = {
//     6, (GPoint []){
//         {-4, 10},
//         {4, 10},
//         {4, -48},
//         {2, -50},
//         {-2,-50},
//         {-4, -48}
//     }
// };

//////////////////////
// hide clock hands //
//////////////////////
static void hide_hands() {
  layer_set_hidden(s_hands_layer, true); 
}

//////////////////////
// show clock hands //
//////////////////////
static void show_hands() {
  layer_set_hidden(s_hands_layer, false);
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  if(direction > 0) {
    hide_hands();
    app_timer_register(5000, show_hands, NULL);
  } else {
    show_hands();
  }
}

/////////////////////////
// draws dial on watch //
/////////////////////////
static void dial_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds); 
  
  graphics_context_set_fill_color(ctx, BACKGROUND_COLOR);
  graphics_fill_circle(ctx, center, (bounds.size.w+buf)/2);
  
  // set number of tickmarks
  int tick_marks_number = 60;

  // tick mark lengths
  int tick_length_end = (bounds.size.w+buf)/2; 
  int tick_length_start;
  
  // set colors
  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_fill_color(ctx, FOREGROUND_COLOR);
  graphics_context_set_stroke_color(ctx, FOREGROUND_COLOR);
  graphics_context_set_stroke_width(ctx, 6);
  
  for(int i=0; i<tick_marks_number; i++) {
    // if number is divisible by 5, make large mark
    if(i%5==0) {
      graphics_context_set_stroke_width(ctx, 4);
      tick_length_start = tick_length_end-8;
    } else {
      graphics_context_set_stroke_width(ctx, 1);
      tick_length_start = tick_length_end-4;
    }

    int angle = TRIG_MAX_ANGLE * i/tick_marks_number;

    GPoint tick_mark_start = {
      .x = (int)(sin_lookup(angle) * (int)tick_length_start / TRIG_MAX_RATIO) + center.x,
      .y = (int)(-cos_lookup(angle) * (int)tick_length_start / TRIG_MAX_RATIO) + center.y,
    };
    
    GPoint tick_mark_end = {
      .x = (int)(sin_lookup(angle) * (int)tick_length_end / TRIG_MAX_RATIO) + center.x,
      .y = (int)(-cos_lookup(angle) * (int)tick_length_end / TRIG_MAX_RATIO) + center.y,
    };      
    
    graphics_draw_line(ctx, tick_mark_end, tick_mark_start);  
  } // end of loop  
}

/////////////////////////////////
// draw hands and update ticks //
/////////////////////////////////
static void ticks_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds); 
    
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
  int hand_point_end = ((bounds.size.h)/2)-10;
  int hand_point_start = hand_point_end - 60;
  
  int filler_point_end = 40;
  int filler_point_start = filler_point_end-15;
  
  float minute_angle = TRIG_MAX_ANGLE * t->tm_min / 60;
  GPoint minute_hand_start = {
    .x = (int)(sin_lookup(minute_angle) * (int)hand_point_start / TRIG_MAX_RATIO) + center.x,
    .y = (int)(-cos_lookup(minute_angle) * (int)hand_point_start / TRIG_MAX_RATIO) + center.y,
  };
  
  GPoint minute_hand_end = {
    .x = (int)(sin_lookup(minute_angle) * (int)hand_point_end / TRIG_MAX_RATIO) + center.x,
    .y = (int)(-cos_lookup(minute_angle) * (int)hand_point_end / TRIG_MAX_RATIO) + center.y,
  };    
  
  float hour_angle = TRIG_MAX_ANGLE * ((((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6);
  GPoint hour_hand_start = {
    .x = (int)(sin_lookup(hour_angle) * (int)hand_point_start / TRIG_MAX_RATIO) + center.x,
    .y = (int)(-cos_lookup(hour_angle) * (int)hand_point_start / TRIG_MAX_RATIO) + center.y,
  };
  
  GPoint hour_hand_end = {
    .x = (int)(sin_lookup(hour_angle) * (int)(hand_point_end-25) / TRIG_MAX_RATIO) + center.x,
    .y = (int)(-cos_lookup(hour_angle) * (int)(hand_point_end-25) / TRIG_MAX_RATIO) + center.y,
  };   
  
  GPoint filler_start = {
    .x = (int)(sin_lookup(hour_angle) * (int)filler_point_start / TRIG_MAX_RATIO) + center.x,
    .y = (int)(-cos_lookup(hour_angle) * (int)filler_point_start / TRIG_MAX_RATIO) + center.y,
  };
  
  GPoint filler_end = {
    .x = (int)(sin_lookup(hour_angle) * (int)filler_point_end / TRIG_MAX_RATIO) + center.x,
    .y = (int)(-cos_lookup(hour_angle) * (int)filler_point_end / TRIG_MAX_RATIO) + center.y,
  }; 
  
  // set colors
  graphics_context_set_antialiased(ctx, true);
   
  // draw wide part of minute hand in background color for shadow
  graphics_context_set_stroke_color(ctx, HAND_BACKGROUND_COLOR);  
  graphics_context_set_stroke_width(ctx, 8);  
  graphics_draw_line(ctx, minute_hand_start, minute_hand_end);  
  
  // draw shadow for minute line
  graphics_context_set_stroke_color(ctx, HAND_BACKGROUND_COLOR);  
  graphics_context_set_stroke_width(ctx, 3);
  graphics_draw_line(ctx, center, minute_hand_start);    
  
  // draw minute line
  graphics_context_set_stroke_color(ctx, HAND_FOREGROUND_COLOR);  
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(ctx, center, minute_hand_start);  
  
  // draw wide part of minute hand
   graphics_context_set_stroke_color(ctx, HAND_FOREGROUND_COLOR);
  graphics_context_set_stroke_width(ctx, 6);  
  graphics_draw_line(ctx, minute_hand_start, minute_hand_end);   
  
  // draw wide part of hour hand in background color for shadow
  graphics_context_set_stroke_color(ctx, HAND_BACKGROUND_COLOR);  
  graphics_context_set_stroke_width(ctx, 8);
  graphics_draw_line(ctx, hour_hand_start, hour_hand_end);  
  
  // draw shadow for small hour line
  graphics_context_set_stroke_color(ctx, HAND_BACKGROUND_COLOR); 
  graphics_context_set_stroke_width(ctx, 3);
  graphics_draw_line(ctx, center, hour_hand_start);   
  
  // draw small hour line
  graphics_context_set_stroke_color(ctx, HAND_FOREGROUND_COLOR); 
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(ctx, center, hour_hand_start);   
  
  // draw wide part of hour hand
  graphics_context_set_stroke_color(ctx, HAND_FOREGROUND_COLOR);  
  graphics_context_set_stroke_width(ctx, 6);
  graphics_draw_line(ctx, hour_hand_start, hour_hand_end);
  
  // draw inner hour line
  graphics_context_set_stroke_color(ctx, HAND_BACKGROUND_COLOR);  
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, filler_start, filler_end);    
  
  // circle overlay
  // draw circle in middle 
  graphics_context_set_fill_color(ctx, HAND_BACKGROUND_COLOR);  
  graphics_fill_circle(ctx, center, 4);  
  // draw circle in middle 
  graphics_context_set_fill_color(ctx, HAND_FOREGROUND_COLOR);  
  graphics_fill_circle(ctx, center, 3);
  
  // dot in the middle
  graphics_context_set_fill_color(ctx, HAND_BACKGROUND_COLOR);
  graphics_fill_circle(ctx, center, 1);    
}


//////////////////////
// load main window //
//////////////////////
static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // set background color
  window_set_background_color(s_main_window, BACKGROUND_COLOR);
  
  // font
  s_word_font = fonts_load_custom_font(resource_get_handle(TEXT_FONT));
  
  // create canvas layer for dial
  s_dial_layer = layer_create(bounds);
  layer_set_update_proc(s_dial_layer, dial_update_proc);
  layer_add_child(window_layer, s_dial_layer); 
  
  // Day Text
  s_date_text_layer = text_layer_create(GRect(0, 123, bounds.size.w, 12));
  text_layer_set_background_color(s_date_text_layer, GColorClear);
  text_layer_set_text_color(s_date_text_layer, FOREGROUND_COLOR);
  text_layer_set_text_alignment(s_date_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_date_text_layer, s_word_font);
  layer_add_child(s_dial_layer, text_layer_get_layer(s_date_text_layer));  
  
  // create temp text
  s_temp_layer = text_layer_create(GRect(48, 138, 24, 12));
  text_layer_set_background_color(s_temp_layer, GColorClear);
  text_layer_set_text_color(s_temp_layer, FOREGROUND_COLOR);
  text_layer_set_text_alignment(s_temp_layer, GTextAlignmentRight);
  text_layer_set_font(s_temp_layer, s_word_font);
  layer_add_child(s_dial_layer, text_layer_get_layer(s_temp_layer));
  
  // create A&M Logo
  s_bitmap = gbitmap_create_with_resource(LOGO);
  s_bitmap_layer = bitmap_layer_create(GRect(0, 33, bounds.size.w, 101));
  bitmap_layer_set_compositing_mode(s_bitmap_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap); 
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));   
  
  // create canvas layer for hands
  s_hands_layer = layer_create(bounds);
  layer_set_update_proc(s_hands_layer, ticks_update_proc);
  layer_add_child(window_layer, s_hands_layer);
}

///////////////////////
// update clock time //
///////////////////////
static void update_time() {
  // get a tm strucutre
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // write date to buffer
  static char date_buffer[32];
  strftime(date_buffer, sizeof(date_buffer), "%A, %B %e", tick_time);
  
  // display this time on the text layer
  text_layer_set_text(s_date_text_layer, date_buffer);
}

//////////////////
// handle ticks //
//////////////////
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(s_hands_layer);
  update_time();
  
  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
  
    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);
  
    // Send the message!
    app_message_outbox_send();
  }  
}
    
///////////////////
// unload window //
///////////////////
static void main_window_unload(Window *window) {
  layer_destroy(s_hands_layer);
  layer_destroy(s_dial_layer);
  gbitmap_destroy(s_bitmap);
  gbitmap_destroy(s_weather_bitmap);
  bitmap_layer_destroy(s_bitmap_layer);
  bitmap_layer_destroy(s_weather_bitmap_layer);
  gpath_destroy(s_minute_arrow);
  gpath_destroy(s_hour_arrow);
  text_layer_destroy(s_date_text_layer);
  text_layer_destroy(s_temp_layer);
}

//////////////////////////////////////
// display appropriate weather icon //
//////////////////////////////////////
static void load_icons() {
  // populate icon variable
    if(strcmp(icon_buf, "clear-day")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CLEAR_SKY_DAY_BLACK_ICON);  
    } else if(strcmp(icon_buf, "clear-night")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CLEAR_SKY_NIGHT_BLACK_ICON);
    }else if(strcmp(icon_buf, "rain")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_RAIN_BLACK_ICON);
    } else if(strcmp(icon_buf, "snow")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SNOW_BLACK_ICON);
    } else if(strcmp(icon_buf, "sleet")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SLEET_BLACK_ICON);
    } else if(strcmp(icon_buf, "wind")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_WIND_BLACK_ICON);
    } else if(strcmp(icon_buf, "fog")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FOG_BLACK_ICON);
    } else if(strcmp(icon_buf, "cloudy")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CLOUDY_BLACK_ICON);
    } else if(strcmp(icon_buf, "partly-cloudy-day")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PARTLY_CLOUDY_DAY_BLACK_ICON);
    } else if(strcmp(icon_buf, "partly-cloudy-night")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PARTLY_CLOUDY_NIGHT_BLACK_ICON);
    }
  // populate weather icon
  if(s_weather_bitmap_layer) {
    bitmap_layer_destroy(s_weather_bitmap_layer);
  }
  s_weather_bitmap_layer = bitmap_layer_create(GRect(72, 136, 24, 16));
  bitmap_layer_set_compositing_mode(s_weather_bitmap_layer, GCompOpSet);  
  bitmap_layer_set_bitmap(s_weather_bitmap_layer, s_weather_bitmap); 
  layer_add_child(s_dial_layer, bitmap_layer_get_layer(s_weather_bitmap_layer)); 
}

///////////////////
// weather calls //
///////////////////
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temp_buf[64];

  // Read tuples for data
  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_KEY_TEMP);
  Tuple *icon_tuple = dict_find(iterator, MESSAGE_KEY_KEY_ICON);  

  // If all data is available, use it
  if(temp_tuple && icon_tuple) {
    
    // temp
    snprintf(temp_buf, sizeof(temp_buf), "%d°", (int)temp_tuple->value->int32);
    text_layer_set_text(s_temp_layer, temp_buf);

    // icon
    snprintf(icon_buf, sizeof(icon_buf), "%s", icon_tuple->value->cstring);
  }  
  
  load_icons();
  APP_LOG(APP_LOG_LEVEL_INFO, "inbox_received_callback");
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

////////////////////
// initialize app //
////////////////////
static void init() {
  s_main_window = window_create();
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  // show window on the watch with animated=true
  window_stack_push(s_main_window, true);
  
  // subscribe to time events
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Make sure the time is displayed from the start
  update_time();
  
//   // init hand paths
//   s_minute_arrow = gpath_create(&MINUTE_HAND_POINTS);
//   s_hour_arrow = gpath_create(&HOUR_HAND_POINTS);
  
  // move hands to proper locations
  Layer *window_layer = window_get_root_layer(s_main_window);
  GRect bounds = layer_get_bounds(window_layer);
  GPoint center = grect_center_point(&bounds);
  gpath_move_to(s_minute_arrow, center);
  gpath_move_to(s_hour_arrow, center);    
  
  // register for taps
  accel_tap_service_subscribe(tap_handler);    
  
  // Register weather callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);  
  
  // Open AppMessage for weather callbacks
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Clock show_clock_window");    
}

///////////////////////
// de-initialize app //
///////////////////////
static void deinit() {
  window_destroy(s_main_window);
}

/////////////
// run app //
/////////////
int main(void) {
  init();
  app_event_loop();
  deinit();
}
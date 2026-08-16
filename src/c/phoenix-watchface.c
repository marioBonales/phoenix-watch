#include <pebble.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static Window *s_window;

static int s_battery_level;
static int s_step_count;
static int steps_objective = 10000;

static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_steps_text_layer;
static TextLayer *s_battery_text_layer;
static TextLayer *s_multiplier_text_layer;

static Layer *s_battery_layer;
static Layer *s_steps_layer;


static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  static char s_time_buffer[8]; 

  strftime(s_time_buffer,sizeof(s_time_buffer), clock_is_24h_style()? "%H:%M": "%I:%M", tick_time);
  text_layer_set_text(s_time_layer, s_time_buffer);

  static char s_date_buffer[16];
  strftime(s_date_buffer, sizeof(s_date_buffer), "%a %m/%d", tick_time);
  text_layer_set_text(s_date_layer, s_date_buffer);
}

static void update_battery_state(BatteryChargeState state) {
  s_battery_level = state.charge_percent;

  layer_mark_dirty(s_battery_layer);
}

static void update_battery(Layer *layer, GContext *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Updated battery");
  GRect bounds =  layer_get_bounds(layer);

  int bar_height = (s_battery_level * (bounds.size.h - 4)/100);

  graphics_context_set_stroke_color(context, GColorBlack);
  graphics_draw_round_rect(context, bounds, 4);

  graphics_context_set_fill_color(context, GColorWhite);
  graphics_fill_rect(context, GRect(2,bounds.size.h - bar_height - 2, bounds.size.w - 4, bar_height), 2, GCornerNone);

  static char battery_buffer[5];

  snprintf(battery_buffer, sizeof(battery_buffer), "%d%%", s_battery_level);

  text_layer_set_text(s_battery_text_layer, battery_buffer);
}

static void update_steps(HealthEventType event, void *context){
  time_t start = time_start_of_today();
  time_t end = time(NULL);
  
  HealthServiceAccessibilityMask mask = health_service_metric_accessible(HealthMetricStepCount, start, end);

  if (!mask) {
    return;
  }

  s_step_count = (int) health_service_sum_today(HealthMetricStepCount);
  static char s_steps_buffer[8];
  snprintf(s_steps_buffer, sizeof(s_steps_buffer), "%d", s_step_count);
  text_layer_set_text(s_steps_text_layer, s_steps_buffer);
  layer_mark_dirty(s_steps_layer);

  if(s_step_count >= steps_objective * 10) {
    text_layer_set_text(s_multiplier_text_layer, "9+");
  }
  else if (s_step_count > steps_objective) {
    static char s_multiplier_buffer[3];
    snprintf(s_multiplier_buffer, sizeof(s_multiplier_buffer), "x%d", s_step_count / steps_objective);
    text_layer_set_text(s_multiplier_text_layer, s_multiplier_buffer);
  } else {
    text_layer_set_text(s_multiplier_text_layer, "");
  }

}

static void update_steps_layer(Layer *layer, GContext *context) {
  GRect bounds =  layer_get_bounds(layer);

  bool overcharged = s_step_count >= steps_objective;


  graphics_context_set_stroke_color(context, GColorBlack);

  graphics_draw_round_rect(context, bounds, 4);

  if (overcharged) {
    graphics_context_set_fill_color(context, GColorWhite);
    graphics_fill_rect(context, GRect(2,2, bounds.size.w - 4, bounds.size.h -4), 2, GCornerNone);
  }

  int bar_height = ((s_step_count % steps_objective / 100) * (bounds.size.h - 4)/100);

  graphics_context_set_fill_color(context, overcharged? GColorGreen : GColorWhite);
  graphics_fill_rect(context, GRect(2,bounds.size.h - bar_height - 3, bounds.size.w - 4, bar_height), 2, GCornerNone);

}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  int time_y = bounds.size.h / 2;

  s_time_layer = text_layer_create(GRect(0,time_y - 42,bounds.size.w, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, PBL_IF_COLOR_ELSE(GColorBlack, GColorBlack));
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  s_date_layer = text_layer_create(GRect(0,time_y, bounds.size.w, 50));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, PBL_IF_COLOR_ELSE(GColorBlack, GColorBlack));
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

  s_steps_text_layer = text_layer_create(GRect(20,bounds.size.h-30, bounds.size.w, 50));
  text_layer_set_background_color(s_steps_text_layer, GColorClear);
  text_layer_set_text_color(s_steps_text_layer, PBL_IF_COLOR_ELSE(GColorBlack, GColorBlack));
  text_layer_set_font(s_steps_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_steps_text_layer, GTextAlignmentLeft);

  s_multiplier_text_layer = text_layer_create(GRect(20,10, bounds.size.w, 50));
  text_layer_set_background_color(s_multiplier_text_layer, GColorClear);
  text_layer_set_text_color(s_multiplier_text_layer, PBL_IF_COLOR_ELSE(GColorBlack, GColorBlack));
  text_layer_set_font(s_multiplier_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_multiplier_text_layer, GTextAlignmentLeft);

  s_battery_text_layer = text_layer_create(GRect(bounds.size.w/2,bounds.size.h-30, bounds.size.w/2 - 18, 20));
  text_layer_set_background_color(s_battery_text_layer, GColorClear);
  text_layer_set_text_color(s_battery_text_layer, PBL_IF_COLOR_ELSE(GColorBlack, GColorBlack));
  text_layer_set_font(s_battery_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_battery_text_layer, GTextAlignmentRight);


  int bar_x = bounds.size.w - 15;
  int bar_y = 10;
  int bar_height = bounds.size.h - 20;
  s_battery_layer = layer_create(GRect(bar_x, bar_y, 8, bar_height));
  layer_set_update_proc(s_battery_layer, update_battery);


  int steps_x = 10;
  s_steps_layer = layer_create(GRect(steps_x, bar_y, 8, bar_height));
  layer_set_update_proc(s_steps_layer, update_steps_layer);

  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_steps_text_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_multiplier_text_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_battery_text_layer));
  layer_add_child(window_layer, s_battery_layer);
  layer_add_child(window_layer, s_steps_layer);
}

static void window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  update_time();
}

static void init(void) {
  s_window = window_create();

  window_set_background_color(s_window, PBL_IF_COLOR_ELSE(GColorChromeYellow, GColorWhite));
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  window_stack_push(s_window, true);
  update_time();
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  battery_state_service_subscribe(update_battery_state);
  update_battery_state(battery_state_service_peek());

  health_service_events_subscribe(update_steps, NULL);

  time_t start = time_start_of_today();
  time_t end = time(NULL);

  HealthServiceAccessibilityMask result = health_service_metric_accessible(HealthMetricStepCount, start, end);
  if (result && HealthServiceAccessibilityMaskAvailable) {
    s_step_count = (int) health_service_sum_today(HealthMetricStepCount);
  }

}

static void deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  init();

  app_event_loop();
  deinit();
}

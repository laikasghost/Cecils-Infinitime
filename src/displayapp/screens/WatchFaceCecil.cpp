//todo: 
//figure out segfault
//make functions to do all this shit
//bottom bar step progress
//colors
//new font??? idk
#include "displayapp/screens/WatchFaceCecil.h"
#include <lvgl/lvgl.h>
#include <cstdio>
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/BleIcon.h"
#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"
#include "components/ble/SimpleWeatherService.h"
#include "displayapp/screens/WeatherSymbols.h"

#include <stdio.h>
using namespace Pinetime::Applications::Screens;

void alignShadowLabelRandom(lv_obj_t* mainLabel, lv_obj_t* shadowLabel) {
  int rand1 = rand() % 15 - 7;
  int rand2 = rand() % 15 - 7;
  lv_label_set_text(shadowLabel, lv_label_get_text(mainLabel));
  lv_obj_align(shadowLabel, mainLabel, LV_ALIGN_IN_TOP_LEFT, rand1, rand2);
}

void alignShadowLabelConsistent(lv_obj_t* mainLabel, lv_obj_t* shadowLabel) {
  lv_label_set_text(shadowLabel, lv_label_get_text(mainLabel));
  lv_obj_align(shadowLabel, mainLabel, LV_ALIGN_IN_TOP_LEFT, 2, 2);
}

lv_obj_t* createShadowContainer(lv_obj_t* parentContainer) {
  lv_obj_t* container = lv_cont_create(lv_scr_act(), nullptr);
  lv_obj_set_parent(container, parentContainer);
  lv_cont_set_layout(container, LV_LAYOUT_OFF);
  lv_cont_set_fit(container, LV_FIT_TIGHT); 
  lv_obj_set_style_local_radius(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_pad_inner(container, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_pad_all(container, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_margin_all(container, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_bg_opa(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  return container;
}

lv_obj_t* createShadowLabel(lv_obj_t* shadowContainer, lv_color_t color) {
  lv_obj_t* shadowLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_parent(shadowLabel, shadowContainer);
  lv_obj_set_style_local_text_color(shadowLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color);
  lv_obj_set_style_local_text_font(shadowLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &pixel_30);
  return shadowLabel;
}

lv_obj_t* createMainLabel(lv_obj_t* shadowLabel, lv_color_t color) {
  lv_obj_t* mainLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_t* shadowContainer = lv_obj_get_parent(shadowLabel);
  lv_obj_set_parent(mainLabel, shadowContainer);
  lv_obj_set_style_local_text_color(mainLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color);
  lv_obj_set_style_local_text_font(mainLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &pixel_30);
  alignShadowLabelConsistent(mainLabel, shadowLabel);
  return mainLabel;
}

//this doesn't work and i don't know why, maybe something about the parent container's alignment
void timeJitter(lv_obj_t* timeContainer) {
  int rand1 = rand() % 3;
  lv_obj_set_style_local_pad_top(timeContainer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, rand1);
}

WatchFaceCecil::WatchFaceCecil(Controllers::DateTime& dateTimeController,
                                                   const Controllers::Battery& batteryController,
                                                   const Controllers::Ble& bleController,
                                                   Controllers::NotificationManager& notificatioManager,
                                                   Controllers::Settings& settingsController,
                                                   Controllers::HeartRateController& heartRateController,
                                                   Controllers::MotionController& motionController,
                                                   Controllers::SimpleWeatherService& weatherService,
                                                   Controllers::FS& filesystem)
  : currentDateTime {{}},
    batteryIcon(false),
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificatioManager {notificatioManager},
    settingsController {settingsController},
    heartRateController {heartRateController},
    motionController {motionController},
    weatherService {weatherService} {

  //for now if i take this shit out, everything breaks :(
  lfs_file f = {};
  if (filesystem.FileOpen(&f, "/fonts/lv_font_dots_40.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_dot40 = lv_font_load("F:/fonts/lv_font_dots_40.bin");
  }
  //end load-bearing garbage

  //maybe find a better way to set background color, idk
  lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_OBJMASK_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_disp_set_bg_color(NULL, color_background);
  
  //parent container for all the other objects
  big_container = lv_cont_create(lv_scr_act(), nullptr);
  lv_obj_set_size(big_container, 240, 240);
  lv_cont_set_fit(big_container, LV_FIT_MAX);
  lv_obj_set_style_local_bg_opa(big_container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_radius(big_container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_pad_inner(big_container, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_pad_all(big_container, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_margin_all(big_container, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
  
  //container for top bar objects/battery bar
  battery_bar = lv_bar_create(lv_scr_act(), nullptr);
  lv_obj_set_parent(battery_bar, big_container);
  lv_obj_align(battery_bar, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 0);
  lv_obj_set_size(battery_bar, 240, 30);
  lv_obj_set_style_local_bg_opa(battery_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_20);
  lv_obj_set_style_local_bg_color(battery_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, color_text);
  lv_obj_set_style_local_radius(battery_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_radius(battery_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_margin_all(battery_bar, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
 lv_obj_set_style_local_pad_all(battery_bar, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
  
  //container for middle section objects
  middle_container = lv_cont_create(lv_scr_act(), nullptr);
  lv_obj_set_parent(middle_container, big_container);
  lv_obj_align(middle_container, battery_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
  lv_cont_set_layout(middle_container, LV_LAYOUT_COLUMN_MID);
  lv_cont_set_fit2(middle_container, LV_FIT_MAX, LV_FIT_TIGHT); //as wide as parent, vertically tight to children
  lv_obj_set_style_local_bg_opa(middle_container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_radius(middle_container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_pad_inner(middle_container, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 3);
  lv_obj_set_style_local_pad_all(middle_container, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 6);
  
  //container for bottom bar objects
  bot_bar_container = lv_cont_create(lv_scr_act(), nullptr);
  lv_obj_set_parent(bot_bar_container, big_container);
  lv_obj_set_size(bot_bar_container, 240, 30);
  lv_obj_align(bot_bar_container, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  lv_cont_set_layout(bot_bar_container, LV_LAYOUT_OFF);
  lv_cont_set_fit2(bot_bar_container, LV_FIT_MAX, LV_FIT_TIGHT); //as wide as parent, vertically tight to children
  lv_obj_set_style_local_bg_opa(bot_bar_container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_20);
  lv_obj_set_style_local_radius(bot_bar_container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_pad_inner(bot_bar_container, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 3);
  lv_obj_set_style_local_margin_all(bot_bar_container, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_pad_all(bot_bar_container, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 6);
  
  //top bar/battery bar items
  //create bluetooth icon
  bleIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_parent(bleIcon, battery_bar);
  lv_obj_align(bleIcon, battery_bar, LV_ALIGN_IN_RIGHT_MID, -6, 0);
  lv_obj_set_style_local_text_color(bleIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_line);
  lv_label_set_text_static(bleIcon, Symbols::bluetooth);
  
  //middle section items
  //BEGIN DATE INFO ROW---
  //date info container
  date_info_container = lv_cont_create(lv_scr_act(), nullptr);
  lv_obj_set_parent(date_info_container, middle_container);
  lv_cont_set_layout(date_info_container, LV_LAYOUT_ROW_BOTTOM);
  lv_cont_set_fit(date_info_container, LV_FIT_TIGHT); 
  lv_obj_set_style_local_bg_opa(date_info_container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_pad_all(date_info_container, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_pad_inner(date_info_container, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 3);
  lv_obj_set_style_local_margin_all(date_info_container, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
  
  //DAY OF WEEK ---
  //day of week shadow container
  day_container = createShadowContainer(date_info_container);
  //create day of week shadow label
  label_day_shadow = createShadowLabel(day_container, color_line);
  lv_label_set_text_static(label_day_shadow, "SUN");
  //create day of week label
  label_day = createMainLabel(label_day_shadow, color_text);
  //---END DAY OF WEEK
  
  //DATE ---
  //create date container
  date_container = createShadowContainer(date_info_container);
  //create date shadow label
  label_date_shadow = createShadowLabel(date_container, color_line);
  lv_label_set_text_static(label_date_shadow, "6/30");
  //create date label
  label_date = createMainLabel(label_date_shadow, color_text);
  //---END DATE

  /*
  //WEEK NUMBER ---
  //create week number container
  week_container = createShadowContainer(date_info_container);
  //create week number shadow label
  label_week_shadow = createShadowLabel(week_container, color_line);
  lv_label_set_text_static(label_week_shadow, "WK26");
  //create week number label
  label_week = createMainLabel(label_week_shadow, color_text);
  //---END WEEK NUMBER
  */
  //---END DATE INFO ROW

  //WEATHER ---
  //create weather shadow container
  weather_container = createShadowContainer(middle_container);
  //create weather shadow label
  label_weather_shadow = createShadowLabel(weather_container, color_line);
  lv_label_set_text_static(label_weather_shadow, "9C");
  //create weather label
  label_weather = createMainLabel(label_weather_shadow, color_text);
  //---END WEATHER

  //TIME ---
  //create time label container
  time_container = createShadowContainer(middle_container);
  //create time shadow label
  label_time_shadow = createShadowLabel(time_container, color_line);
  lv_obj_set_style_local_text_font(label_time_shadow, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &bubble_80);
  //create time label
  label_time = createMainLabel(label_time_shadow, color_text);
  lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &bubble_80);
  lv_label_set_text_static(label_time, "08:34");
  alignShadowLabelRandom(label_time, label_time_shadow);
  //--- END TIME

  //bottom bar items
  //HEARTBEAT---
  //create heartbeat info container
  heartbeat_container = createShadowContainer(bot_bar_container);
  lv_cont_set_layout(heartbeat_container, LV_LAYOUT_ROW_MID);
  lv_obj_align(heartbeat_container, bot_bar_container, LV_ALIGN_IN_LEFT_MID, 6, 0);
  //create heartbeat icon label
  heartbeatIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_parent(heartbeatIcon, heartbeat_container);
  lv_label_set_text_static(heartbeatIcon, Symbols::heartBeat);
  lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  //create heartbeat value label
  heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_parent(heartbeatValue, heartbeat_container);
  lv_obj_set_style_local_text_color(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_obj_set_style_local_text_font(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &pixel_20);
  lv_label_set_text_static(heartbeatValue, "");
  //---END HEARTBEAT

  //STEPS---
  //create step info container
  step_container = createShadowContainer(bot_bar_container);
  lv_cont_set_layout(step_container, LV_LAYOUT_ROW_MID);
  lv_obj_align(step_container, bot_bar_container, LV_ALIGN_IN_RIGHT_MID, -6, 0);
  //create step icon label
  stepIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_parent(stepIcon, step_container);
  lv_obj_set_style_local_text_color(stepIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_label_set_text_static(stepIcon, Symbols::shoe);

  //create step value label
  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_parent(stepValue, step_container);
  lv_obj_set_style_local_text_color(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_obj_set_style_local_text_font(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &pixel_20);
  lv_label_set_text_static(stepValue, "0");
  //---END STEPS
  
  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceCecil::~WatchFaceCecil() {
  lv_task_del(taskRefresh);

  lv_style_reset(&style_line);
  lv_style_reset(&style_border);

  //more load-bearing garbage
  if (font_dot40 != nullptr) {
    lv_font_free(font_dot40);
  }
  //end load-bearing garbage

  lv_obj_clean(lv_scr_act());
}

void WatchFaceCecil::Refresh() {
  powerPresent = batteryController.IsPowerPresent();
  if (powerPresent.IsUpdated()) {
    if (batteryController.IsPowerPresent()) {
      lv_obj_set_style_local_bg_color(battery_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, color_line);
    } else {
      lv_obj_set_style_local_bg_color(battery_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, color_text);
    }
  }

  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated()) {
    auto batteryPercent = batteryPercentRemaining.Get();
    lv_bar_set_value(battery_bar, batteryPercent, LV_ANIM_OFF);
  }

  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated() || bleRadioEnabled.IsUpdated()) {
    lv_label_set_text_static(bleIcon, BleIcon::GetIcon(bleState.Get()));
  }
  lv_obj_realign(bleIcon);

  currentDateTime = std::chrono::time_point_cast<std::chrono::minutes>(dateTimeController.CurrentDateTime());
  if (currentDateTime.IsUpdated()) {
    uint8_t hour = dateTimeController.Hours();
    uint8_t minute = dateTimeController.Minutes();

    if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
      lv_label_set_text_fmt(label_time, "%2d:%02d", hour, minute);
    } else {
      lv_label_set_text_fmt(label_time, "%02d:%02d", hour, minute);
    }
    lv_obj_realign(label_time);
    alignShadowLabelRandom(label_time, label_time_shadow);

    currentDate = std::chrono::time_point_cast<std::chrono::days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      const char* weekNumberFormat = "%V";

      Controllers::DateTime::Months month = dateTimeController.Month();
      uint8_t day = dateTimeController.Day();
      lv_label_set_text_fmt(label_date, "%d/%d", month, day);
      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24) {
        // 24h mode: mmddyyyy, first DOW=Monday;
        //lv_label_set_text_fmt(label_date, "%d/%d", month, day);
        weekNumberFormat = "%V"; // Replaced by the week number of the year (Monday as the first day of the week) as a decimal number
                                 // [01,53]. If the week containing 1 January has four or more days in the new year, then it is considered
                                 // week 1. Otherwise, it is the last week of the previous year, and the next week is week 1. Both January
                                 // 4th and the first Thursday of January are always in week 1. [ tm_year, tm_wday, tm_yday]
      } else {
        // 12h mode: mmddyyyy, first DOW=Sunday;
        //lv_label_set_text_fmt(label_date, "%d/%d", month, day);
        weekNumberFormat = "%U"; // Replaced by the week number of the year as a decimal number [00,53]. The first Sunday of January is the
                                 // first day of week 1; days in the new year before this are in week 0. [ tm_year, tm_wday, tm_yday]
      }

      time_t ttTime =
        std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::system_clock::duration>(currentDateTime.Get()));
      tm* tmTime = std::localtime(&ttTime);

      char buffer[8];
      strftime(buffer, 8, weekNumberFormat, tmTime);
      //uint8_t weekNumber = atoi(buffer);

      lv_label_set_text_fmt(label_day, "%s", dateTimeController.DayOfWeekShortToStringLow(dateTimeController.DayOfWeek()));
      //lv_label_set_text_fmt(label_week, "Week %02d", weekNumber);

      lv_obj_realign(label_day);
      alignShadowLabelConsistent(label_day, label_day_shadow);
      //lv_obj_realign(label_week);
      //alignShadowLabelConsistent(label_week, label_week_shadow);
      lv_obj_realign(label_date);
      alignShadowLabelConsistent(label_date, label_date_shadow);
    }
  }
  
  currentWeather = weatherService.Current();
  if (currentWeather.IsUpdated()) {
    auto optCurrentWeather = currentWeather.Get();
    if (optCurrentWeather) {
      int16_t temp = optCurrentWeather->temperature.Celsius();
      char tempUnit = 'C';
      if (settingsController.GetWeatherFormat() == Controllers::Settings::WeatherFormat::Imperial) {
        temp = optCurrentWeather->temperature.Fahrenheit();
        tempUnit = 'F';
      }
      lv_label_set_text_fmt(label_weather,
                            "%d%c, %s",
                            temp,
                            tempUnit,
                            Symbols::GetSimpleCondition(optCurrentWeather->iconId));
    } else {
      lv_label_set_text(label_weather, "WTHR");
    }
  }
  lv_obj_realign(label_weather);
  alignShadowLabelConsistent(label_weather, label_weather_shadow);

  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
    if (heartbeatRunning.Get()) {
      lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
      lv_label_set_text_fmt(heartbeatValue, "%d", heartbeat.Get());
    } else {
      lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x405E26));
      lv_label_set_text_static(heartbeatValue, "");
    }

    lv_obj_realign(heartbeatIcon);
    lv_obj_realign(heartbeatValue);
  }

  stepCount = motionController.GetTripSteps();
  if (stepCount.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "%lu", stepCount.Get());
    lv_obj_realign(stepValue);
    lv_obj_realign(stepIcon);
  }
}

//other block of load-bearing garbage
bool WatchFaceCecil::IsAvailable(Pinetime::Controllers::FS& filesystem) {
  lfs_file file = {};

  if (filesystem.FileOpen(&file, "/fonts/lv_font_dots_40.bin", LFS_O_RDONLY) < 0) {
    return false;
  }
  filesystem.FileClose(&file);
  return true;
}
//end load-bearing garbage

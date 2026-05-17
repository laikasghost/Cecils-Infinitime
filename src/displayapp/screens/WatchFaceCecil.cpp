//todo: 
//make smaller
//works better now that it is smaller, but not good enough to not crash, can pull up apps but not settings
//how can i take out unnecessary items defined in constructor?
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

lv_color_t color_main = lv_color_hex(0xB58600); //main text color: goldenrod
lv_color_t color_secondary = lv_color_hex(0xF7FFF7); //secondary color: text shadows, etc?: white/v light green
lv_color_t color_background = lv_color_hex(0x4A6931); //moss green
lv_color_t color_contrast_background = lv_color_hex(0x102800); //darker mossy green

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

lv_obj_t* createShadowLabel(lv_obj_t* shadowContainer) {
  lv_obj_t* shadowLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_parent(shadowLabel, shadowContainer);
  lv_obj_set_style_local_text_color(shadowLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_secondary);
  lv_obj_set_style_local_text_font(shadowLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &pixel_30);
  return shadowLabel;
}

lv_obj_t* createMainLabel(lv_obj_t* shadowLabel) {
  lv_obj_t* mainLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_t* shadowContainer = lv_obj_get_parent(shadowLabel);
  lv_obj_set_parent(mainLabel, shadowContainer);
  lv_obj_set_style_local_text_color(mainLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_main);
  lv_obj_set_style_local_text_font(mainLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &pixel_30);
  alignShadowLabelConsistent(mainLabel, shadowLabel);
  return mainLabel;
}

//this doesn't work and i don't know why, maybe something about the parent container's alignment
void timeJitter(lv_obj_t* timeContainer) {
  int rand1 = rand() % 5;
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
  }
  //end load-bearing garbage

  //maybe find a better way to set background color, idk
  lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_OBJMASK_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_disp_set_bg_color(NULL, color_background);//this is color_background, won't work if i just put the var, idk why
  
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
  lv_obj_set_style_local_bg_color(battery_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, color_contrast_background);
  lv_obj_set_style_local_bg_color(battery_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, color_secondary);
  lv_obj_set_style_local_radius(battery_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_radius(battery_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_margin_all(battery_bar, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_pad_all(battery_bar, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 3);
  
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
  steps_bar = lv_bar_create(lv_scr_act(), nullptr);
  lv_obj_set_parent(steps_bar, big_container);
  lv_obj_align(steps_bar, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, -20);
  lv_obj_set_size(steps_bar, 240, 30);
  lv_obj_set_style_local_bg_opa(steps_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_20);
  lv_obj_set_style_local_bg_color(steps_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, color_contrast_background);
  lv_obj_set_style_local_bg_color(steps_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, color_secondary);
  lv_obj_set_style_local_radius(steps_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_radius(steps_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_margin_all(steps_bar, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_pad_all(steps_bar, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 3);
  lv_bar_set_range(steps_bar, 0, 6000);
  
  //top bar/battery bar items
  //create bluetooth icon
  bleIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_parent(bleIcon, battery_bar);
  lv_obj_align(bleIcon, battery_bar, LV_ALIGN_IN_LEFT_MID, 6, 0);
  lv_obj_set_style_local_text_color(bleIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_main);
  lv_label_set_text_static(bleIcon, Symbols::bluetooth);
  
  //middle section items
  //DATE ---
  //create date container
  date_container = createShadowContainer(middle_container);
  //create date shadow label
  label_date_shadow = createShadowLabel(date_container);
  lv_label_set_text_static(label_date_shadow, "6/30");
  //create date label
  label_date = createMainLabel(label_date_shadow);
  //---END DATE

  //WEATHER ---
  //create weather shadow container
  weather_container = createShadowContainer(middle_container);
  //create weather shadow label
  label_weather_shadow = createShadowLabel(weather_container);
  lv_label_set_text_static(label_weather_shadow, "9C");
  //create weather label
  label_weather = createMainLabel(label_weather_shadow);
  //---END WEATHER

  //TIME ---
  //create time label container
  time_container = createShadowContainer(middle_container);
  //create time shadow label
  label_time_shadow = createShadowLabel(time_container);
  lv_obj_set_style_local_text_font(label_time_shadow, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &bubble_80);
  //create time label
  label_time = createMainLabel(label_time_shadow);
  lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &bubble_80);
  lv_label_set_text_static(label_time, "08:34");
  alignShadowLabelRandom(label_time, label_time_shadow);
  //--- END TIME

  //bottom bar items
  fitness_data_container = lv_cont_create(lv_scr_act(), nullptr);
  lv_cont_set_layout(fitness_data_container, LV_LAYOUT_ROW_MID);
  lv_obj_set_parent(fitness_data_container, steps_bar);
  lv_cont_set_fit(fitness_data_container, LV_FIT_TIGHT); 
  lv_obj_set_style_local_radius(fitness_data_container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_pad_inner(fitness_data_container, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 3);
  lv_obj_set_style_local_pad_all(fitness_data_container, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 3);
  lv_obj_set_style_local_pad_left(fitness_data_container, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 6);
  lv_obj_set_style_local_margin_all(fitness_data_container, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_bg_opa(fitness_data_container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

  //STEPS---
  //create step icon label
  stepIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_parent(stepIcon, fitness_data_container);
  lv_obj_set_style_local_text_color(stepIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_main);
  lv_label_set_text_static(stepIcon, Symbols::shoe);
  //create step value label
  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_parent(stepValue, fitness_data_container);
  lv_obj_set_style_local_text_color(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_main);
  lv_obj_set_style_local_text_font(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &pixel_20);
  lv_label_set_text_static(stepValue, "0");
  //---END STEPS

  //HEARTBEAT---
  //create heartbeat icon label
  heartbeatIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_parent(heartbeatIcon, fitness_data_container);
  lv_label_set_text_static(heartbeatIcon, Symbols::heartBeat);
  lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_main);
  //create heartbeat value label
  heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_parent(heartbeatValue, fitness_data_container);
  lv_obj_set_style_local_text_color(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_main);
  lv_obj_set_style_local_text_font(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &pixel_20);
  lv_label_set_text_static(heartbeatValue, "");
  //---END HEARTBEAT
  
  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceCecil::~WatchFaceCecil() {
  lv_task_del(taskRefresh);

  //lv_style_reset(&style_line); make styles eventually prob

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
      lv_obj_set_style_local_bg_color(battery_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, color_contrast_background);
      lv_obj_set_style_local_bg_opa(battery_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_COVER);
    } else {
      lv_obj_set_style_local_bg_color(battery_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, color_contrast_background);
      lv_obj_set_style_local_bg_opa(battery_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_20);
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
      time_t ttTime =
        std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::system_clock::duration>(currentDateTime.Get()));
      tm* tmTime = std::localtime(&ttTime);

      char buffer[8];
      strftime(buffer, 8, weekNumberFormat, tmTime);
      lv_label_set_text_fmt(label_date, "%s %d/%d", dateTimeController.DayOfWeekShortToStringLow(dateTimeController.DayOfWeek()), month, day);

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
      lv_label_set_text_fmt(heartbeatValue, "%d", heartbeat.Get());
      lv_label_set_text_static(heartbeatIcon, Symbols::heartBeat);
    } else {
      lv_label_set_text_static(heartbeatValue, "");
      lv_label_set_text_static(heartbeatIcon, "");
    }

    lv_obj_realign(heartbeatIcon);
    lv_obj_realign(heartbeatValue);
  }

  stepCount = motionController.GetTripSteps();
  if (stepCount.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "%lu", stepCount.Get());
    if (stepCount.Get() <= 6000) {
      lv_bar_set_value(steps_bar, stepCount.Get(), LV_ANIM_OFF);
    } else {
      lv_bar_set_value(steps_bar, 6000, LV_ANIM_OFF);
    }
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

#pragma once

#include <displayapp/screens/BatteryIcon.h>
#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include <displayapp/Controllers.h>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "components/ble/BleController.h"
#include "components/ble/SimpleWeatherService.h"
#include "utility/DirtyValue.h"
#include "displayapp/apps/Apps.h"

namespace Pinetime {
  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    class NotificationManager;
    class HeartRateController;
    class MotionController;
  }

  namespace Applications {
    namespace Screens {

      class WatchFaceCecil : public Screen {
      public:
        WatchFaceCecil(Controllers::DateTime& dateTimeController,
                                 const Controllers::Battery& batteryController,
                                 const Controllers::Ble& bleController,
                                 Controllers::NotificationManager& notificatioManager,
                                 Controllers::Settings& settingsController,
                                 Controllers::HeartRateController& heartRateController,
                                 Controllers::MotionController& motionController,
                                 Controllers::SimpleWeatherService& weatherService,
                                 Controllers::FS& filesystem);
        ~WatchFaceCecil() override;

        void Refresh() override;

        static bool IsAvailable(Pinetime::Controllers::FS& filesystem);

      private:
        Utility::DirtyValue<uint8_t> batteryPercentRemaining {};
        Utility::DirtyValue<bool> powerPresent {};
        Utility::DirtyValue<bool> bleState {};
        Utility::DirtyValue<bool> bleRadioEnabled {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>> currentDateTime {};
        Utility::DirtyValue<uint32_t> stepCount {};
        Utility::DirtyValue<uint8_t> heartbeat {};
        Utility::DirtyValue<bool> heartbeatRunning {};
        Utility::DirtyValue<bool> notificationState {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::days>> currentDate;
        Utility::DirtyValue<std::optional<Controllers::SimpleWeatherService::CurrentWeather>> currentWeather {};

        lv_color_t color_text = lv_color_hex(0xD9F9BD);
        lv_color_t color_line = lv_color_hex(0xF5CF39);
        lv_color_t color_background = lv_color_hex(0x193302);
        
        lv_style_t style_line;
        lv_style_t style_border;

        lv_obj_t* big_container;
        lv_obj_t* middle_container;
        lv_obj_t* bot_bar_container;
        lv_obj_t* batt_info_container;
        lv_obj_t* date_info_container;
        lv_obj_t* time_container;
        lv_obj_t* label_time;
        lv_obj_t* label_time_shadow;
        lv_obj_t* date_container;
        lv_obj_t* label_date;
        lv_obj_t* label_date_shadow;
        lv_obj_t* day_container;
        lv_obj_t* label_day;
        lv_obj_t* label_day_shadow;
        lv_obj_t* week_container;
        lv_obj_t* label_week;
        lv_obj_t* label_week_shadow;
        lv_obj_t* weather_container;
        lv_obj_t* label_weather;
        lv_obj_t* label_weather_shadow;
        lv_obj_t* bleIcon;
        lv_obj_t* batteryPlug;
        lv_obj_t* battery_bar;
        lv_obj_t* label_battery_value;
        lv_obj_t* heartbeat_container;
        lv_obj_t* heartbeatIcon;
        lv_obj_t* heartbeatValue;
        lv_obj_t* step_container;
        lv_obj_t* stepIcon;
        lv_obj_t* stepValue;
        lv_obj_t* notificationIcon;
        lv_obj_t* weather;

        BatteryIcon batteryIcon;

        Controllers::DateTime& dateTimeController;
        const Controllers::Battery& batteryController;
        const Controllers::Ble& bleController;
        Controllers::NotificationManager& notificatioManager;
        Controllers::Settings& settingsController;
        Controllers::HeartRateController& heartRateController;
        Controllers::MotionController& motionController;
        Controllers::SimpleWeatherService& weatherService;

        lv_task_t* taskRefresh;
        //remove these once they're gone from cpp file
        lv_font_t* font_dot40 = nullptr;
        lv_font_t* font_segment40 = nullptr;
        lv_font_t* font_segment115 = nullptr;
        lv_obj_t* backgroundLabel;
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::Cecil> {
      static constexpr WatchFace watchFace = WatchFace::Cecil;
      static constexpr const char* name = "Cecil";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFaceCecil(controllers.dateTimeController,
                                                     controllers.batteryController,
                                                     controllers.bleController,
                                                     controllers.notificationManager,
                                                     controllers.settingsController,
                                                     controllers.heartRateController,
                                                     controllers.motionController,
                                                     *controllers.weatherController,
                                                     controllers.filesystem);
      };

      lv_obj_t* createShadowContainer(lv_obj_t*);
      lv_obj_t* createMainLabel(lv_obj_t*, lv_color_t);
      lv_obj_t* createShadowLabel(lv_obj_t*, lv_color_t);
      
      void alignShadowLabelRandom(lv_obj_t*, lv_obj_t*);
      void alignShadowLabelConsistent(lv_obj_t*, lv_obj_t*);
      void timeJitter(lv_obj_t*);
      
      static bool IsAvailable(Pinetime::Controllers::FS& filesystem) {
        return Screens::WatchFaceCecil::IsAvailable(filesystem);
      }
    };
  }
}

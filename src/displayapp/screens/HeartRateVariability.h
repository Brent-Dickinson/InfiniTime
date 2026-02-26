#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include "displayapp/Colors.h"
#include "displayapp/screens/Screen.h"
#include "systemtask/SystemTask.h"
#include "systemtask/WakeLock.h"
#include "Symbols.h"
#include "components/heartrate/HeartRateController.h"

namespace Pinetime {
  namespace Controllers {
    class HeartRateController;
  }

  namespace Applications {
    namespace Screens {

      class HeartRateVariability : public Screen {
      public:
        HeartRateVariability(Controllers::HeartRateController& heartRateController, System::SystemTask& systemTask);
        ~HeartRateVariability() override;

        void Refresh() override;
        void OnStartStopEvent(lv_event_t event);

      private:
        static constexpr size_t maxSamples = 64;
        static constexpr float minValidBpm = 35.0f;

        struct MetricSnapshot {
          float rmssd = 0.0f;
          float sdnn = 0.0f;
          float pnn50 = 0.0f;
          size_t samples = 0;
        };

        void UpdateStartStopButton(bool isRunning);
        void AddSample(uint8_t bpm, uint16_t sensorCounter);
        void RecomputeMetrics();
        void UpdateMetricLabels();
        void ResetSamples();

        Controllers::HeartRateController& heartRateController;
        Pinetime::System::WakeLock wakeLock;

        lv_obj_t* labelTitle = nullptr;
        lv_obj_t* labelHr = nullptr;
        lv_obj_t* labelStatus = nullptr;
        lv_obj_t* labelRmssd = nullptr;
        lv_obj_t* labelSdnn = nullptr;
        lv_obj_t* labelPnn50 = nullptr;
        lv_obj_t* labelSamples = nullptr;
        lv_obj_t* btnStartStop = nullptr;
        lv_obj_t* labelStartStop = nullptr;
        lv_obj_t* labelHint = nullptr;
        lv_task_t* taskRefresh = nullptr;

        std::array<float, maxSamples> rrIntervals {};
        size_t rrCount = 0;
        uint16_t lastSensorCounter = 0;
        bool metricsDirty = true;
        MetricSnapshot metrics;
        Controllers::HeartRateController::States previousState;
      };
    }

    template <>
    struct AppTraits<Apps::HeartRateVariability> {
      static constexpr Apps app = Apps::HeartRateVariability;
      static constexpr const char* icon = Screens::Symbols::tachometer;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::HeartRateVariability(controllers.heartRateController, *controllers.systemTask);
      }

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
  }
}

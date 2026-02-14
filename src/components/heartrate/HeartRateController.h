#pragma once

#include <cstdint>
#include <components/ble/HeartRateService.h>

namespace Pinetime {
  namespace Applications {
    class HeartRateTask;
  }

  namespace System {
    class SystemTask;
  }

  namespace Controllers {
    class HeartRateController {
    public:
      enum class States : uint8_t { Stopped, NotEnoughData, NoTouch, Running };

      HeartRateController() = default;
      void Enable();
      void Disable();
      void Update(States newState, uint8_t heartRate);

      void SetHeartRateTask(Applications::HeartRateTask* task);

      States State() const {
        return state;
      }

      uint8_t HeartRate() const {
        return heartRate;
      }

      void SetService(Pinetime::Controllers::HeartRateService* service);

    private:
      Applications::HeartRateTask* task = nullptr;
      States state = States::Stopped;
      uint8_t heartRate = 0;
      Pinetime::Controllers::HeartRateService* service = nullptr;
    
    // added by brent 2/14/26 for purposes of getting hrs to UI //
    public:
      void SetSampleRateX10(uint16_t hzX10);
      uint16_t SampleRateX10() const;

    private:
      uint16_t sampleRateX10 {0};

    ///////////////////////////////////////////////////////////////
    };
  }
}
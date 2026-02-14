#include "components/heartrate/HeartRateController.h"
#include <heartratetask/HeartRateTask.h>
#include <systemtask/SystemTask.h>

using namespace Pinetime::Controllers;

void HeartRateController::Update(HeartRateController::States newState, uint8_t heartRate) {
  this->state = newState;
  if (this->heartRate != heartRate) {
    this->heartRate = heartRate;
    service->OnNewHeartRateValue(heartRate);
  }
}

void HeartRateController::Enable() {
  if (task != nullptr) {
    state = States::NotEnoughData;
    task->PushMessage(Pinetime::Applications::HeartRateTask::Messages::Enable);
  }
}

void HeartRateController::Disable() {
  if (task != nullptr) {
    state = States::Stopped;
    task->PushMessage(Pinetime::Applications::HeartRateTask::Messages::Disable);
  }
}

void HeartRateController::SetHeartRateTask(Pinetime::Applications::HeartRateTask* task) {
  this->task = task;
}

void HeartRateController::SetService(Pinetime::Controllers::HeartRateService* service) {
  this->service = service;
}

// added by brent 2/14/26 for purposes of publishing hrs to UI //
void HeartRateController::SetSampleRateX10(uint16_t hzX10) {
  sampleRateX10 = hzX10;
}

uint16_t HeartRateController::SampleRateX10() const {
  return sampleRateX10;
}

//////////////////////////////////////////////////////////////////


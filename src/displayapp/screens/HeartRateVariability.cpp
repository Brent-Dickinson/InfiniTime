#include "displayapp/screens/HeartRateVariability.h"
#include <lvgl/lvgl.h>
#include <algorithm>
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;
using namespace Pinetime::Controllers;

namespace {
  const char* ToString(HeartRateController::States s) {
    switch (s) {
      case HeartRateController::States::NotEnoughData:
        return "Not enough data,\nplease wait...";
      case HeartRateController::States::NoTouch:
        return "No touch detected";
      case HeartRateController::States::Running:
        return "Measuring...";
      case HeartRateController::States::Stopped:
      default:
        return "Stopped";
    }
  }

  constexpr lv_coord_t kLineSpacing = 8;
}

HeartRateVariability::HeartRateVariability(HeartRateController& heartRateController, System::SystemTask& systemTask)
  : heartRateController {heartRateController}, wakeLock(systemTask), previousState {heartRateController.State()} {
  bool isHrRunning = previousState != HeartRateController::States::Stopped;

  labelTitle = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(labelTitle, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_28);
  lv_label_set_text_static(labelTitle, "Heart Rate\nVariability");
  lv_obj_align(labelTitle, nullptr, LV_ALIGN_IN_TOP_MID, 0, 10);

  labelHr = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(labelHr, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_label_set_text_static(labelHr, "---");
  lv_obj_align(labelHr, nullptr, LV_ALIGN_CENTER, 0, -30);

  labelStatus = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(labelStatus, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_label_set_text_static(labelStatus, ToString(previousState));
  lv_obj_align(labelStatus, labelHr, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

  labelRmssd = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(labelRmssd, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_28);
  lv_label_set_text_static(labelRmssd, "RMSSD\n-- ms");
  lv_obj_align(labelRmssd, nullptr, LV_ALIGN_IN_LEFT_MID, 0, -10);

  labelSdnn = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(labelSdnn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_28);
  lv_label_set_text_static(labelSdnn, "SDNN\n-- ms");
  lv_obj_align(labelSdnn, labelRmssd, LV_ALIGN_OUT_BOTTOM_LEFT, 0, kLineSpacing);

  labelPnn50 = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(labelPnn50, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_28);
  lv_label_set_text_static(labelPnn50, "pNN50\n-- %");
  lv_obj_align(labelPnn50, labelSdnn, LV_ALIGN_OUT_BOTTOM_LEFT, 0, kLineSpacing);

  labelSamples = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(labelSamples, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_20);
  lv_label_set_text_static(labelSamples, "Beats: 0/64");
  lv_obj_align(labelSamples, nullptr, LV_ALIGN_IN_BOTTOM_LEFT, 0, -55);

  labelHint = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(labelHint, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_16);
  lv_obj_set_style_local_text_color(labelHint, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_text_static(labelHint, "Need ~20 beats for stable HRV");
  lv_obj_align(labelHint, labelSamples, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);

  btnStartStop = lv_btn_create(lv_scr_act(), nullptr);
  btnStartStop->user_data = this;
  lv_obj_set_height(btnStartStop, 50);
  lv_obj_set_event_cb(btnStartStop, [](lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<HeartRateVariability*>(obj->user_data);
    screen->OnStartStopEvent(event);
  });
  lv_obj_align(btnStartStop, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, -5, -5);

  labelStartStop = lv_label_create(btnStartStop, nullptr);
  UpdateStartStopButton(isHrRunning);

  if (isHrRunning) {
    wakeLock.Lock();
  }

  lastSensorCounter = heartRateController.HandleSensorDataCount();
  taskRefresh = lv_task_create(RefreshTaskCallback, 150, LV_TASK_PRIO_MID, this);
}

HeartRateVariability::~HeartRateVariability() {
  if (taskRefresh != nullptr) {
    lv_task_del(taskRefresh);
  }
  wakeLock.Release();
  lv_obj_clean(lv_scr_act());
}

void HeartRateVariability::Refresh() {
  auto state = heartRateController.State();
  auto bpm = heartRateController.HeartRate();
  bool isRunning = state != HeartRateController::States::Stopped;

  if (previousState != state) {
    if (isRunning && previousState == HeartRateController::States::Stopped) {
      wakeLock.Lock();
      ResetSamples();
    } else if (!isRunning && previousState != HeartRateController::States::Stopped) {
      wakeLock.Release();
      ResetSamples();
    }
    UpdateStartStopButton(isRunning);
    previousState = state;
  }

  if (isRunning && bpm > 0) {
    lv_obj_set_style_local_text_color(labelHr, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::highlight);
    lv_label_set_text_fmt(labelHr, "%03d", bpm);
    auto sensorCounter = heartRateController.HandleSensorDataCount();
    if (sensorCounter != lastSensorCounter) {
      AddSample(bpm, sensorCounter);
    }
  } else {
    lv_obj_set_style_local_text_color(labelHr, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
    lv_label_set_text_static(labelHr, "---");
  }

  lv_label_set_text_static(labelStatus, ToString(state));

  if (metricsDirty) {
    RecomputeMetrics();
    UpdateMetricLabels();
  }
}

void HeartRateVariability::OnStartStopEvent(lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }

  if (heartRateController.State() == HeartRateController::States::Stopped) {
    heartRateController.Enable();
  } else {
    heartRateController.Disable();
  }
}

void HeartRateVariability::UpdateStartStopButton(bool isRunning) {
  if (labelStartStop == nullptr) {
    return;
  }
  if (isRunning) {
    lv_label_set_text_static(labelStartStop, "Stop");
  } else {
    lv_label_set_text_static(labelStartStop, "Start");
  }
}

void HeartRateVariability::AddSample(uint8_t bpm, uint16_t sensorCounter) {
  lastSensorCounter = sensorCounter;
  if (bpm < minValidBpm) {
    return;
  }
  const float rr = 60000.0f / static_cast<float>(bpm);
  if (!std::isfinite(rr)) {
    return;
  }
  if (rrCount < rrIntervals.size()) {
    rrIntervals[rrCount++] = rr;
  } else {
    std::rotate(rrIntervals.begin(), rrIntervals.begin() + 1, rrIntervals.end());
    rrIntervals.back() = rr;
  }
  metricsDirty = true;
}

void HeartRateVariability::RecomputeMetrics() {
  metrics.samples = rrCount;
  if (rrCount < 2) {
    metrics.rmssd = 0.0f;
    metrics.sdnn = 0.0f;
    metrics.pnn50 = 0.0f;
    metricsDirty = false;
    return;
  }

  float mean = 0.0f;
  for (size_t i = 0; i < rrCount; i++) {
    mean += rrIntervals[i];
  }
  mean /= static_cast<float>(rrCount);

  float variance = 0.0f;
  for (size_t i = 0; i < rrCount; i++) {
    float diff = rrIntervals[i] - mean;
    variance += diff * diff;
  }
  variance /= static_cast<float>(rrCount - 1);
  metrics.sdnn = sqrtf(std::max(0.0f, variance));

  float diffSum = 0.0f;
  size_t diffCount = 0;
  size_t over50 = 0;
  for (size_t i = 1; i < rrCount; i++) {
    float diff = rrIntervals[i] - rrIntervals[i - 1];
    diffSum += diff * diff;
    if (fabsf(diff) >= 50.0f) {
      over50++;
    }
    diffCount++;
  }

  metrics.rmssd = diffCount > 0 ? sqrtf(diffSum / static_cast<float>(diffCount)) : 0.0f;
  metrics.pnn50 = diffCount > 0 ? (100.0f * static_cast<float>(over50) / static_cast<float>(diffCount)) : 0.0f;
  metricsDirty = false;
}

void HeartRateVariability::UpdateMetricLabels() {
  if (metrics.samples < 2) {
    lv_label_set_text_static(labelRmssd, "RMSSD\n-- ms");
    lv_label_set_text_static(labelSdnn, "SDNN\n-- ms");
    lv_label_set_text_static(labelPnn50, "pNN50\n-- %");
  } else {
    lv_label_set_text_fmt(labelRmssd, "RMSSD\n%3.0f ms", metrics.rmssd);
    lv_label_set_text_fmt(labelSdnn, "SDNN\n%3.0f ms", metrics.sdnn);
    lv_label_set_text_fmt(labelPnn50, "pNN50\n%2.0f %%", metrics.pnn50);
  }

  lv_label_set_text_fmt(labelSamples, "Beats: %u/%u", static_cast<unsigned>(metrics.samples), static_cast<unsigned>(maxSamples));
}

void HeartRateVariability::ResetSamples() {
  rrIntervals.fill(0.0f);
  rrCount = 0;
  metricsDirty = true;
  metrics = {};
  lastSensorCounter = heartRateController.HandleSensorDataCount();
}
